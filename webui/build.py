#!/usr/bin/env python3
"""Compila la WebUI para el SPIFFS del DLC32.

Junta index.html, css/style.css, js/i18n.js y js/app.js en un solo HTML con el
CSS y el JS embebidos, lo comprime con gzip y lo deja en
Firmware/Grbl_Esp32/data/index.html.gz, que es lo que sube `pio run -t uploadfs`.

    python3 webui/build.py            # compila y actualiza el .gz del firmware
    python3 webui/build.py --check    # solo verifica que el .gz commiteado
                                      # corresponda a las fuentes actuales

El modo --check existe porque el .gz es un artefacto generado que vive fuera de
esta carpeta: sin esta verificacion es facil commitear fuentes editadas junto a
un .gz viejo y flashear una interfaz que no corresponde al codigo.
"""

import argparse
import gzip
import sys
from pathlib import Path

WEBUI_DIR = Path(__file__).resolve().parent
REPO_ROOT = WEBUI_DIR.parent

SRC_HTML = WEBUI_DIR / "index.html"
SRC_CSS = WEBUI_DIR / "css" / "style.css"
SRC_JS = WEBUI_DIR / "js" / "app.js"
SRC_I18N = WEBUI_DIR / "js" / "i18n.js"
SRC_I18N_ZH = WEBUI_DIR / "js" / "i18n_zh.js"

DIST_HTML = WEBUI_DIR / "dist" / "index.html"
FIRMWARE_GZ = REPO_ROOT / "Firmware" / "Grbl_Esp32" / "data" / "index.html.gz"

CSS_TAG = '<link rel="stylesheet" href="css/style.css">'
I18N_ZH_TAG = '<script src="js/i18n_zh.js"></script>'
I18N_TAG = '<script src="js/i18n.js"></script>'
JS_TAG = '<script src="js/app.js"></script>'

# El ESP32 sirve el archivo tal cual desde SPIFFS, asi que cada byte ahorrado
# aqui es espacio de flash. Nivel maximo, siempre.
GZIP_LEVEL = 9


def inline() -> str:
    """Devuelve el HTML con el CSS y el JS embebidos."""
    html = SRC_HTML.read_text(encoding="utf-8")

    for tag, name in ((CSS_TAG, "CSS"), (I18N_ZH_TAG, "I18N_ZH"), (I18N_TAG, "I18N"), (JS_TAG, "JS")):
        if html.count(tag) != 1:
            sys.exit(
                f"error: se esperaba exactamente una referencia a {name} en "
                f"index.html ({tag!r}), se encontraron {html.count(tag)}.\n"
                "Si cambiaste como se enlazan los assets, actualiza build.py."
            )

    css = SRC_CSS.read_text(encoding="utf-8")
    i18n = SRC_I18N.read_text(encoding="utf-8")
    i18n_zh = SRC_I18N_ZH.read_text(encoding="utf-8")
    js = SRC_JS.read_text(encoding="utf-8")

    html = html.replace(CSS_TAG, f"<style>{css}</style>")
    html = html.replace(I18N_ZH_TAG, f"<script>{i18n_zh}</script>")
    html = html.replace(I18N_TAG, f"<script>{i18n}</script>")
    html = html.replace(JS_TAG, f"<script>{js}</script>")
    return html


def compress(html: str) -> bytes:
    """Comprime con mtime=0 para que la salida sea reproducible byte a byte."""
    return gzip.compress(html.encode("utf-8"), compresslevel=GZIP_LEVEL, mtime=0)


def check() -> int:
    """Compara el .gz del firmware con las fuentes actuales.

    Se compara el HTML *descomprimido*, no los bytes del gzip: el stream
    deflate que genera zlib depende de la version de la libreria (el .gz
    commiteado se genero en una plataforma y aqui se verifica en otra), asi que
    comparar bytes crudos daba falsos negativos aunque el contenido fuera
    identico.
    """
    if not FIRMWARE_GZ.exists():
        print(f"FALLA: no existe {FIRMWARE_GZ.relative_to(REPO_ROOT)}")
        return 1
    try:
        actual = gzip.decompress(FIRMWARE_GZ.read_bytes())
    except OSError as e:
        print(f"FALLA: {FIRMWARE_GZ.relative_to(REPO_ROOT)} no es un gzip valido ({e})")
        return 1
    expected = inline().encode("utf-8")
    if actual != expected:
        print(
            f"FALLA: {FIRMWARE_GZ.relative_to(REPO_ROOT)} no corresponde a las "
            "fuentes de webui/.\nCorre: python3 webui/build.py"
        )
        return 1
    print(f"OK: {FIRMWARE_GZ.relative_to(REPO_ROOT)} esta al dia.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="no escribe nada; falla si el .gz del firmware esta desactualizado",
    )
    args = parser.parse_args()

    if args.check:
        return check()

    html = inline()
    blob = compress(html)

    DIST_HTML.parent.mkdir(parents=True, exist_ok=True)
    DIST_HTML.write_text(html, encoding="utf-8")
    FIRMWARE_GZ.write_bytes(blob)

    raw_kb = len(html.encode("utf-8")) / 1024
    gz_kb = len(blob) / 1024
    print(f"{DIST_HTML.relative_to(REPO_ROOT)}  {raw_kb:.1f} KB")
    print(f"{FIRMWARE_GZ.relative_to(REPO_ROOT)}  {gz_kb:.1f} KB  (gzip -{GZIP_LEVEL})")
    print("\nPara flashear:  pio run -e mks_dlc32_8mb -t uploadfs")
    return 0


if __name__ == "__main__":
    sys.exit(main())
