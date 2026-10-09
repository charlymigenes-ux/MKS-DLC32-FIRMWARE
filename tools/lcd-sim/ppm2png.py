#!/usr/bin/env python3
"""Convierte los .ppm (P6) de un directorio a .png con solo la libreria estandar."""
import sys, zlib, struct, pathlib
def png(w, h, rgb):
    raw = b"".join(b"\x00" + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    def ch(t, d): return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d))
    return b"\x89PNG\r\n\x1a\n" + ch(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) + ch(b"IDAT", zlib.compress(raw, 9)) + ch(b"IEND", b"")
for p in pathlib.Path(sys.argv[1]).glob("*.ppm"):
    d = p.read_bytes(); parts = d.split(b"\n", 3)
    w, h = map(int, parts[1].split())
    p.with_suffix(".png").write_bytes(png(w, h, parts[3])); p.unlink()
