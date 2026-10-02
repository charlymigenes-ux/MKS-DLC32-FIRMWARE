#!/usr/bin/env python3
"""Convierte un PNG de 480x320 en la imagen de inicio del firmware (lv_pic/mks_logo.c).

uso:  python3 tools/png2lvgl.py logo.png [salida.c] [--nombre mks_logo] [--tam 480x320]

- Solo usa la libreria estandar de Python (sin Pillow ni descargas).
- Acepta PNG de 8 o 16 bits, sin entrelazar: gris, RGB, paleta, y con canal alfa
  (el alfa se compone sobre negro).
- Escribe las cuatro variantes de profundidad de color que espera LVGL, igual que
  el archivo original; el firmware usa la que corresponde a su lv_conf.h.
Despues hay que compilar y flashear el firmware (pio run -e mks_dlc32_8mb -t upload).
"""
import struct
import sys
import zlib
from pathlib import Path

DEFAULT_OUT = Path(__file__).resolve().parent.parent / "Firmware" / "Grbl_Esp32" / "src" / "lv_pic" / "mks_logo.c"


def read_png(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        sys.exit("error: no es un PNG")
    pos, idat, plte, w = 8, [], None, None
    while pos < len(data):
        n, typ = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + n]
        pos += 12 + n
        if typ == b"IHDR":
            w, h, depth, ctype, _, _, inter = struct.unpack(">IIBBBBB", body)
        elif typ == b"PLTE":
            plte = body
        elif typ == b"IDAT":
            idat.append(body)
        elif typ == b"IEND":
            break
    if w is None:
        sys.exit("error: PNG sin cabecera")
    if inter:
        sys.exit("error: PNG entrelazado (Adam7); guardalo sin entrelazar")
    if depth not in (8, 16):
        sys.exit("error: solo PNG de 8 o 16 bits por canal")
    chans = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(ctype)
    if chans is None:
        sys.exit("error: tipo de color PNG no soportado")
    if ctype == 3 and plte is None:
        sys.exit("error: PNG de paleta sin PLTE")
    bpp = chans * depth // 8
    stride = w * bpp
    raw = zlib.decompress(b"".join(idat))
    rows, prev = [], bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]
        cur = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a = cur[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if f == 1:
                cur[i] = (cur[i] + a) & 255
            elif f == 2:
                cur[i] = (cur[i] + b) & 255
            elif f == 3:
                cur[i] = (cur[i] + ((a + b) >> 1)) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                cur[i] = (cur[i] + pr) & 255
        rows.append(cur)
        prev = cur
    step = depth // 8   # con 16 bits se toma el byte alto
    pix = []
    for cur in rows:
        for x in range(w):
            o = x * bpp
            if ctype == 3:
                i = cur[o] * 3
                r, g, b = plte[i], plte[i + 1], plte[i + 2]
                a = 255
            elif ctype == 0:
                r = g = b = cur[o]; a = 255
            elif ctype == 4:
                r = g = b = cur[o]; a = cur[o + step]
            elif ctype == 2:
                r, g, b = cur[o], cur[o + step], cur[o + 2 * step]; a = 255
            else:
                r, g, b, a = cur[o], cur[o + step], cur[o + 2 * step], cur[o + 3 * step]
            if a != 255:
                r, g, b = r * a // 255, g * a // 255, b * a // 255
            pix.append((r, g, b))
    return w, h, pix


def fmt(v):
    return "\n".join("  " + ", ".join("0x%02x" % x for x in v[i:i + 64]) + ", " for i in range(0, len(v), 64)) + "\n"


def encode(pix):
    b8, b16, b16s, b32 = [], [], [], []
    for r, g, b in pix:
        b8.append((r & 0xE0) | ((g >> 5) << 2) | (b >> 6))
        v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        b16 += [v & 0xFF, v >> 8]
        b16s += [v >> 8, v & 0xFF]
        b32 += [b, g, r, 0xFF]
    return [b8, b16, b16s, b32]


CONDS = ["#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8",
         "#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0",
         "#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0",
         "#if LV_COLOR_DEPTH == 32"]


def main():
    args = sys.argv[1:]
    name, size = "mks_logo", (480, 320)
    for opt in ("--nombre", "--tam"):
        if opt in args:
            i = args.index(opt)
            val = args[i + 1]
            del args[i:i + 2]
            if opt == "--nombre":
                name = val
            else:
                size = tuple(int(x) for x in val.lower().split("x"))
    if not args:
        sys.exit(__doc__)
    out = Path(args[1]) if len(args) > 1 else DEFAULT_OUT
    w, h, pix = read_png(args[0])
    if (w, h) != size:
        sys.exit("error: la imagen mide %dx%d y debe medir %dx%d" % (w, h, size[0], size[1]))
    text = ("/*\n*  Imagen de inicio generada por tools/png2lvgl.py (%dx%d)\n*/\n\n\n"
            '#include "lvgl.h"\n\n#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n\n'
            "const LV_ATTRIBUTE_MEM_ALIGN uint8_t %s_map[] = {\n") % (w, h, name)
    for cond, v in zip(CONDS, encode(pix)):
        text += cond + "\n" + fmt(v) + "#endif\n"
    text += ("};\n\nconst lv_img_dsc_t %s = {\n    .header.always_zero = 0,\n    .header.w = %d,\n"
             "    .header.h = %d,\n    .data_size = %d * LV_COLOR_SIZE / 8,\n"
             "    .header.cf = LV_IMG_CF_TRUE_COLOR,\n    .data = %s_map,\n};\n\n//end of file\n") % (name, w, h, w * h, name)
    out.write_text(text, encoding="utf-8")
    print("escrito %s (%d KB)" % (out, len(text) // 1024))


main()
