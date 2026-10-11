#include "MKS_draw_boot.h"
#include "MKS_draw_lvgl.h"
#include "MKS_draw_qr.h"
#include "MKS_draw_language.h"

#define BOOT_URL        "github.com/charlymigenes-ux/MKS-DLC32-FIRMWARE"
#define BOOT_TICKS      800     // 4 s a 5 ms por vuelta
#define BOOT_LIGHT_TICK 100     // 0,5 s

static uint16_t boot_ticks = 0;
static bool     boot_done = false, boot_light_sent = false, boot_hold = false;
static lv_style_t boot_ver_style;

static const char* boot_T(const char* es, const char* en) {
    return mks_tr(es, en);   // el chino de la fuente no trae estos caracteres: se usa ingles
}

static void boot_qr_closed(lv_obj_t*, lv_event_t) {
    boot_done = true;   // tocar el QR continua el arranque
}

// Pulsacion larga sobre el logo: se espera a que se SUELTE el dedo para abrir el QR (si se abriera con el
// dedo puesto, esa misma pulsacion lo cerraria al soltar). Mientras se mantiene, el contador de 4 s se detiene.
static void boot_hold_event(lv_obj_t*, lv_event_t event) {
    if (boot_done) return;
    if (event == LV_EVENT_LONG_PRESSED) {
        boot_hold = true;
    } else if ((event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) && boot_hold) {
        boot_hold = false;
        if (!mks_qr_popup_is_open()) {
            mks_qr_popup(BOOT_URL, boot_T("Toca el QR para continuar", "Tap the QR to continue"), boot_qr_closed);
        }
    }
}

void mks_boot_begin(void) {
    boot_ticks = 0;
    boot_done = boot_light_sent = boot_hold = false;
    mks_draw_logo();

    // version (la parte de FW_NAME antes del parentesis) abajo a la derecha
    char ver[24] = "";
    const char* open = strchr(FW_NAME, '(');
    snprintf(ver, sizeof(ver), "%.*s", (int)(open != NULL ? open - FW_NAME : strlen(FW_NAME)), FW_NAME);
    char text[40];
    snprintf(text, sizeof(text), "M+X  %s", ver);

    lv_style_copy(&boot_ver_style, &lv_style_plain);
    boot_ver_style.text.font  = &roboto16Latin;
    boot_ver_style.text.color = LV_COLOR_MAKE(0xD6, 0xDA, 0xE8);
    lv_obj_t* l = lv_label_create(mks_global.mks_src, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &boot_ver_style);
    lv_label_set_text(l, text);
    lv_obj_align(l, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -8);

    lv_obj_set_click(mks_global.mks_src, true);
    lv_obj_set_event_cb(mks_global.mks_src, boot_hold_event);
}

bool mks_boot_backlight_due(void) {
    if (boot_light_sent) return false;
    if (boot_ticks >= BOOT_LIGHT_TICK || boot_hold || mks_qr_popup_is_open()) {
        boot_light_sent = true;
        return true;
    }
    return false;
}

bool mks_boot_tick(void) {
    if (boot_hold || mks_qr_popup_is_open()) return false;   // con el dedo puesto o el QR a la vista el contador se detiene
    if (!boot_done && ++boot_ticks >= BOOT_TICKS) boot_done = true;
    if (boot_done) {
        lv_obj_set_event_cb(mks_global.mks_src, NULL);
        return true;
    }
    return false;
}
