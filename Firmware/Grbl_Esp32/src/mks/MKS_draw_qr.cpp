#include "MKS_draw_qr.h"
#include "MKS_draw_lvgl.h"

LV_IMG_DECLARE(png_qr_repo);   // 205x205 px con la zona blanca incluida

static lv_obj_t*     qr_overlay = NULL;
static lv_style_t    qr_bg_style, qr_cap_style, qr_hint_style;
static lv_event_cb_t qr_on_tap = NULL;

static lv_signal_cb_t qr_ancestor_signal = NULL;

// Si la ventana se borra desde fuera (por ejemplo, un trabajo lanzado desde la web limpia la pantalla),
// se olvida el puntero para que la proxima ventana pueda abrirse.
static lv_res_t qr_signal(lv_obj_t* obj, lv_signal_t sign, void* param) {
    lv_res_t res = qr_ancestor_signal(obj, sign, param);
    if (res == LV_RES_OK && sign == LV_SIGNAL_CLEANUP) {
        qr_overlay = NULL;
        qr_on_tap = NULL;
    }
    return res;
}

static void qr_tap_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    lv_event_cb_t cb = qr_on_tap;
    mks_qr_popup_close();
    if (cb != NULL) cb(NULL, LV_EVENT_CLICKED);
}

bool mks_qr_popup_is_open(void) {
    return qr_overlay != NULL;
}

void mks_qr_popup_close(void) {
    if (qr_overlay != NULL) {
        lv_obj_del(qr_overlay);
        qr_overlay = NULL;
    }
    qr_on_tap = NULL;
}

void mks_qr_popup(const char* caption, const char* hint, lv_event_cb_t on_tap) {
    if (qr_overlay != NULL) return;
    qr_on_tap = on_tap;

    lv_style_copy(&qr_bg_style, &lv_style_plain_color);
    qr_bg_style.body.main_color   = LV_COLOR_MAKE(0x0C, 0x0E, 0x16);
    qr_bg_style.body.grad_color   = LV_COLOR_MAKE(0x0C, 0x0E, 0x16);
    qr_bg_style.body.opa          = LV_OPA_COVER;
    qr_bg_style.body.radius       = 0;
    qr_bg_style.body.border.width = 0;
    lv_style_copy(&qr_cap_style, &lv_style_plain);
    qr_cap_style.text.font  = &roboto16Latin;
    qr_cap_style.text.color = LV_COLOR_MAKE(0xE6, 0xE9, 0xF5);
    lv_style_copy(&qr_hint_style, &qr_cap_style);
    qr_hint_style.text.color = LV_COLOR_MAKE(0x2D, 0xE0, 0xA7);

    qr_overlay = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(qr_overlay, 480, 320);
    lv_obj_set_pos(qr_overlay, 0, 0);
    lv_obj_set_style(qr_overlay, &qr_bg_style);
    lv_obj_set_click(qr_overlay, true);
    lv_obj_set_event_cb(qr_overlay, qr_tap_event);
    qr_ancestor_signal = lv_obj_get_signal_cb(qr_overlay);
    lv_obj_set_signal_cb(qr_overlay, qr_signal);

    lv_obj_t* img = lv_img_create(qr_overlay, NULL);   // la imagen no recibe toques: los recibe la capa
    lv_img_set_src(img, &png_qr_repo);
    lv_obj_align(img, qr_overlay, LV_ALIGN_IN_TOP_MID, 0, 6);

    lv_obj_t* cap = lv_label_create(qr_overlay, NULL);
    lv_label_set_style(cap, LV_LABEL_STYLE_MAIN, &qr_cap_style);
    lv_label_set_text(cap, caption);
    lv_obj_align(cap, qr_overlay, LV_ALIGN_IN_TOP_MID, 0, 218);

    lv_obj_t* h = lv_label_create(qr_overlay, NULL);
    lv_label_set_style(h, LV_LABEL_STYLE_MAIN, &qr_hint_style);
    lv_label_set_text(h, hint);
    lv_obj_align(h, qr_overlay, LV_ALIGN_IN_TOP_MID, 0, 262);
}
