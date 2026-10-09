#!/bin/bash
# Copia las fuentes de las pantallas del firmware al arbol del simulador
# (los #include "../Grbl.h", "../Serial.h"... caen en los stubs de src/).
cd "$(dirname "$0")" || exit 1
F="$(pwd)/../../Firmware"
rm -rf src/mks && mkdir -p src/mks
cp "$F"/Grbl_Esp32/src/mks/*.h "$F"/Grbl_Esp32/src/mks/*.cpp src/mks/
