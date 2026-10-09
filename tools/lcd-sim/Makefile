# Simulador de las pantallas del LCD: LVGL 6.1.1 + el codigo real de MKS_draw_*.cpp compilados en el PC.
# make shots  -> regenera las capturas de los tres idiomas en ../../docs/pantallas/{es,en,zh}
# Se refrescan las fuentes de las pantallas ANTES de resolver dependencias (si no, corre en paralelo con la compilacion)
$(shell ./refresh.sh)
FW      := $(abspath ../../Firmware)
LVGL    := $(FW)/libraries/lvgl
PIC     := $(FW)/Grbl_Esp32/src/lv_pic
CFLAGS  := -Istubs -O1 -w -include stdlib.h -include string.h -include stdio.h -I$(LVGL) -I$(LVGL)/src -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE
CXXFLAGS:= $(CFLAGS) -std=gnu++17 -Isrc -Isrc/mks
LVGL_SRC:= $(shell find $(LVGL)/src/src -name '*.c')
PIC_SRC := $(shell ls $(PIC)/*.c | grep -v ' ')
DRAW_SRC:= $(wildcard src/mks/MKS_draw_*.cpp) src/mks/draw_ui.cpp
OBJ     := $(patsubst $(LVGL)/%.c,build/lvgl/%.o,$(LVGL_SRC)) \
           $(patsubst $(PIC)/%.c,build/pic/%.o,$(PIC_SRC)) \
           $(patsubst src/mks/%.cpp,build/draw/%.o,$(DRAW_SRC)) build/main.o build/stubs.o

sim: $(OBJ)
	g++ -o $@ $(OBJ) -lm
build/lvgl/%.o: $(LVGL)/%.c
	@mkdir -p $(dir $@) && gcc $(CFLAGS) -c $< -o $@
build/pic/%.o: $(PIC)/%.c
	@mkdir -p $(dir $@) && gcc $(CFLAGS) -c $< -o $@
build/draw/%.o: src/mks/%.cpp $(wildcard src/mks/*.h) src/Grbl.h
	@mkdir -p $(dir $@) && g++ $(CXXFLAGS) -c $< -o $@
build/main.o: main.cpp src/Grbl.h
	@mkdir -p build && g++ $(CXXFLAGS) -c $< -o $@
build/stubs.o: stubs.cpp src/Grbl.h
	@mkdir -p build && g++ $(CXXFLAGS) -c $< -o $@

OUT := $(abspath ../../docs/pantallas)
shots: sim
	@for l in es en zh; do rm -rf $(OUT)/$$l; mkdir -p $(OUT)/$$l; ./sim $(OUT)/$$l $$l > /dev/null; python3 ppm2png.py $(OUT)/$$l; done
	@echo "capturas en $(OUT)"
clean:
	rm -rf build sim src/mks
.PHONY: shots clean
