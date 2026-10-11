// Sustituto de MKS_preview.cpp para el simulador: genera la miniatura con el mismo nucleo (MKS_preview_core)
// leyendo un archivo del PC (variable SIM_GCODE) y de forma sincrona. sim_pv_hold deja la pantalla "generando".
#include "MKS_preview.h"
#include "MKS_preview_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool    sim_pv_hold = false;
uint8_t sim_pv_hold_pct = 37;

static mks_pv_state_t st = PV_IDLE;
static uint8_t        prog = 0;
static lv_img_dsc_t   dsc;
static uint8_t*       data = NULL;
static float          iw, ih, it;
static uint32_t       im_moves;

static size_t rd(void* c, uint8_t* b, size_t n) { return fread(b, 1, n, (FILE*)c); }
static bool   rw(void* c) { rewind((FILE*)c); return true; }

void mks_pv_request(const char*, uint32_t) {
    free(data); data = NULL;
    if (sim_pv_hold) { st = PV_RUNNING; prog = sim_pv_hold_pct; return; }
    const char* path = getenv("SIM_GCODE");
    FILE* f = path ? fopen(path, "rb") : NULL;
    pv_image_t im;
    if (!f || !pv_generate(rd, rw, f, 1000, 144, NULL, &im)) { st = PV_FAILED; if (f) fclose(f); return; }
    fclose(f);
    data = (uint8_t*)malloc(64 + im.nib_size);
    for (int i = 0; i < 16; i++) {
        uint8_t c = (i == 0) ? 0 : (uint8_t)((i - 1) * 255 / 14);
        data[i * 4] = c; data[i * 4 + 1] = c; data[i * 4 + 2] = c; data[i * 4 + 3] = 255;
    }
    memcpy(data + 64, im.nib, im.nib_size);
    memset(&dsc, 0, sizeof(dsc));
    dsc.header.cf = LV_IMG_CF_INDEXED_4BIT; dsc.header.w = im.w; dsc.header.h = im.h;
    dsc.data_size = 64 + im.nib_size; dsc.data = data;
    iw = im.width_mm; ih = im.height_mm; it = im.travel_m; im_moves = im.moves;
    pv_free(&im);
    st = PV_READY;
}
void mks_pv_cancel(void) {}
void mks_pv_release(void) { free(data); data = NULL; st = PV_IDLE; }
mks_pv_state_t mks_pv_state(void) { return st; }
uint8_t mks_pv_progress(void) { return prog; }
int mks_pv_error(void) { return 0; }
const lv_img_dsc_t* mks_pv_image(void) { return data ? &dsc : NULL; }
void mks_pv_info(float* w, float* h, float* t, uint32_t* m) { if (w) *w = iw; if (h) *h = ih; if (t) *t = it; if (m) *m = im_moves; }
