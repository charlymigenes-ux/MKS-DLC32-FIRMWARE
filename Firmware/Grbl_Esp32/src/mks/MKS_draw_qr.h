#ifndef __MKS_DRAW_QR_H
#define __MKS_DRAW_QR_H

#include "lvgl.h"

// Ventana con el QR del repositorio del firmware (imagen fija lv_pic/png_qr_repo.c, generada con tools/qr/qr2lvgl.py).
// Tocar cualquier punto la cierra. on_tap (puede ser NULL) se llama despues de cerrarla.
void mks_qr_popup(const char* caption, const char* hint, lv_event_cb_t on_tap);
void mks_qr_popup_close(void);
bool mks_qr_popup_is_open(void);

#endif
