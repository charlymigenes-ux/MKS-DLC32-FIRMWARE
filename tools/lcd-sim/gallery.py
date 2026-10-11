#!/usr/bin/env python3
"""Genera docs/pantallas/index.html (galeria de las capturas del LCD) con los nombres legibles de cada escena."""
import pathlib, sys
OUT = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path("../../docs/pantallas")
NAMES = {
    "00_logo": "Arranque: logo y versión (4 s)", "00b_arranque_qr": "Arranque: mantener pulsado muestra el QR",
    "01_principal": "Pantalla principal", "01b_principal_sin_hora": "Principal sin hora sincronizada (sin reloj)",
    "02_control": "Control", "02b_control_mas": "Control · cabecera, página 2", "03_laser_popup": "Popup del láser",
    "03b_laser_30": "Láser al 30 % (el popup sigue abierto)", "03c_laser_atras": "Atrás con el láser encendido (lo apaga)",
    "04_sd_pagina1": "Archivos SD · página 1", "05_sd_pagina2": "Archivos SD · página 2", "06_sd_pagina3": "Archivos SD · página 3",
    "07a_vista_generando": "Vista previa (generándose)", "07b_vista_previa": "Vista previa del archivo", "07_archivo": "Posicionar el archivo", "08_herramientas": "Configuración (al entrar)", "08b_herramienta": "Configuración · Herramienta",
    "09_wifi": "Configuración · Wifi", "10_idioma": "Configuración · Idioma", "11_beeper": "Configuración · Beeper",
    "12_placa": "Configuración · Placa", "13_acerca": "Configuración · Acerca de", "13b_acerca_qr": "Acerca de: el enlace abre el QR",
    "14_trabajo": "Trabajo en curso",
}
LANGS = [("es", "Español"), ("en", "English"), ("zh", "中文"), ("pt", "Português")]
h = ['<!doctype html><html lang="es"><meta charset="utf-8"><title>Pantallas del LCD</title><style>body{font-family:sans-serif;background:#12141c;color:#e5e7eb;margin:0;padding:24px}h1{margin:0 0 4px}h2{margin:32px 0 12px;color:#2de0a7}.g{display:grid;grid-template-columns:repeat(auto-fill,minmax(480px,1fr));gap:16px}figure{margin:0}img{width:100%;max-width:480px;border-radius:8px;border:1px solid #3f4666}figcaption{font-size:14px;color:#9aa3c0;margin-top:4px}nav a{color:#2de0a7;margin-right:16px}</style><body><h1>Pantallas del LCD</h1><p>Generadas por <code>tools/lcd-sim</code> (480×320, el código real del firmware). Los datos son ficticios.</p><nav>']
h += ['<a href="#%s">%s</a>' % (c, n) for c, n in LANGS] + ['</nav>']
for c, n in LANGS:
    h.append('<h2 id="%s">%s</h2><div class="g">' % (c, n))
    for p in sorted((OUT / c).glob("*.png")):
        h.append('<figure><img src="%s/%s" alt=""><figcaption>%s</figcaption></figure>' % (c, p.name, NAMES.get(p.stem, p.stem)))
    h.append("</div>")
h.append("</body></html>")
(OUT / "index.html").write_text("\n".join(h), encoding="utf-8")
print("galeria:", OUT / "index.html")
