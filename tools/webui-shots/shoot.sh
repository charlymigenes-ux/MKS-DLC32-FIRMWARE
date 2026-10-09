#!/bin/bash
# Captura una vista de la WebUI contra la placa falsa (mock.py en el puerto 8120) con Firefox sin pantalla.
# uso: ./shoot.sh nombre ancho,alto vista oscuro(0|1) idioma     vistas: control sdfile settings language about
#      p. ej. ./shoot.sh webui-control-oscuro 1280,900 control 1 es      (deja el PNG en ../../docs/webui/)
export MOZ_HEADLESS=1
D="$(cd "$(dirname "$0")" && pwd)/../../docs/webui"
mkdir -p /tmp/ffprof "$D"
timeout 80 firefox --headless --no-remote --profile /tmp/ffprof --screenshot "$D/$1.png" --window-size="$2" "http://127.0.0.1:8120/?shot=$3&dark=$4&lang=$5" >/dev/null 2>&1
echo "$1 $(stat -c %s "$D/$1.png" 2>/dev/null)"
