#!/usr/bin/env python3
"""Convierte una matriz de modulos de QR (texto de 0 y 1, una fila por linea) en una imagen de LVGL de 1 bit.

uso: python3 tools/qr/qr2lvgl.py tools/qr/repo_qr_modules.txt [salida.c] [--nombre png_qr_repo] [--escala 5]

- La imagen incluye la zona blanca de 4 modulos alrededor que exige el estandar QR.
- Formato LV_IMG_CF_INDEXED_1BIT de LVGL 6.1: paleta de 2 colores y 1 bit por pixel (sin dependencias).
- repo_qr_modules.txt es el QR de https://github.com/charlymigenes-ux/MKS-DLC32-FIRMWARE
  (QR version 4, correccion M, 33x33 modulos), leido de una imagen y verificado al decodificarlo.
Para otro enlace: genera su QR con cualquier herramienta, extrae su matriz de modulos a un archivo igual y ejecuta esto.
"""
import sys
from pathlib import Path

args = [a for a in sys.argv[1:] if not a.startswith("--")]
opts = dict(zip(sys.argv[1:], sys.argv[2:]))
if not args:
    sys.exit(__doc__)
src = Path(args[0])
out = Path(args[1]) if len(args) > 1 else Path("png_qr_repo.c")
name = opts.get("--nombre", "png_qr_repo")
scale = int(opts.get("--escala", "5"))
QUIET = 4

rows = [l.strip() for l in src.read_text().splitlines() if l.strip()]
n = len(rows)
if any(len(r) != n or set(r) - set("01") for r in rows):
    sys.exit("error: la matriz debe ser cuadrada y solo de 0 y 1")
size = (n + 2 * QUIET) * scale
stride = (size + 7) // 8
data = bytearray()
for y in range(size):
    row = bytearray(stride)
    my = y // scale - QUIET
    for x in range(size):
        mx = x // scale - QUIET
        if 0 <= my < n and 0 <= mx < n and rows[my][mx] == "1":
            row[x >> 3] |= 0x80 >> (x & 7)
    data += row
palette = bytes([255, 255, 255, 255,    # indice 0: blanco  (B, G, R, A)
                 0, 0, 0, 255])         # indice 1: negro
blob = palette + bytes(data)
lines = ["  " + ", ".join("0x%02x" % b for b in blob[i:i + 16]) + "," for i in range(0, len(blob), 16)]
out.write_text(
    "/* QR del repositorio (%dx%d modulos, escala %d, zona blanca incluida). Generado por tools/qr/qr2lvgl.py */\n"
    '#include "lvgl.h"\n\n#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n'
    "const LV_ATTRIBUTE_MEM_ALIGN uint8_t %s_map[] = {\n%s\n};\n\n"
    "const lv_img_dsc_t %s = {\n    .header.always_zero = 0,\n    .header.w = %d,\n    .header.h = %d,\n"
    "    .data_size = %d,\n    .header.cf = LV_IMG_CF_INDEXED_1BIT,\n    .data = %s_map,\n};\n"
    % (n, n, scale, name, "\n".join(lines), name, size, size, len(blob), name))
print("escrito %s (%dx%d px, %d bytes)" % (out, size, size, len(blob)))
