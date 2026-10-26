#include "MKS_draw_pen.h"
#include "MKS_draw_ready.h"
#include "MKS_draw_language.h"   // mc_language, mks_tr, mc_font
#include "../SDCard.h"           // SD_client y SD_auth_level (entre pasadas)

// Definida en MKS_draw_move.cpp: "$J=G91<eje><paso>F<velocidad>"
extern void set_move(char axis, float step, uint32_t speed);

#define PEN_COL_CARD  LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define PEN_COL_SIDE  LV_COLOR_MAKE(0x18, 0x1B, 0x28)
#define PEN_COL_TRACK LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define PEN_COL_EM    LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define PEN_COL_BLUE  LV_COLOR_MAKE(0x1E, 0x3A, 0x5F)
#define PEN_COL_MUTED LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)
#define PEN_COL_WARN  LV_COLOR_MAKE(0xFF, 0xB8, 0x4C)

#define PEN_TOL 0.3f   // mm: margen para decir que la pluma esta arriba o abajo

static lv_style_t pen_card_style, pen_btn_style, pen_btn_pr_style, pen_btn_sel_style, pen_btn_blue_style, pen_btn_blue_pr_style;
static lv_style_t pen_txt_style, pen_dark_style, pen_muted_style, pen_ok_style, pen_warn_style, pen_arrow_style, pen_arrow_ok_style;

static lv_obj_t *pen_b_back, *pen_b_yp, *pen_b_yn, *pen_b_xn, *pen_b_xp, *pen_b_cxy;
static lv_obj_t *pen_b_up, *pen_b_down, *pen_b_cz, *pen_b_home;
#define PEN_NSTEP 5                                                  // 4 pasos fijos + 1 personalizado (teclado)
static lv_obj_t *pen_b_step[PEN_NSTEP], *pen_step_lbl[PEN_NSTEP];
static lv_obj_t *pen_lbl_pos = NULL, *pen_lbl_state = NULL, *pen_bar = NULL;
static uint8_t   pen_step_idx = 1;                                   // 1 mm por defecto
static const float pen_steps[4] = { 0.1f, 1.0f, 10.0f, 50.0f };
static const char* pen_step_txt[4] = { "0.1", "1", "10", "50" };
static float     pen_custom_step = 2.5f;                             // mm del paso personalizado: se guarda en $46
static float     pen_jog_feed = 0;                                   // mm/min de los movimientos manuales: se guarda en $45
static bool      pen_prefs_dirty = false;                            // hay un cambio de velocidad o de paso sin guardar en ajustes
static uint32_t  pen_prefs_t = 0;                                    // cuando fue el ultimo cambio (se guarda 1,5 s despues)

static lv_obj_t *pen_b_hm = NULL, *pen_b_vm = NULL, *pen_b_vp = NULL, *pen_b_vv = NULL, *pen_lbl_vel = NULL;
static uint8_t   pen_home_stage = 0;                                 // 0 libre, 1 preparar, 2 subiendo la pluma, 3 lanzar $H
static uint32_t  pen_home_t0 = 0;
static bool      pen_home_unlocked = false;

// teclado numerico (ventana emergente) para el paso personalizado y la velocidad
static lv_obj_t *kp_cont = NULL, *kp_val = NULL, *kp_key[12], *kp_ok = NULL, *kp_cancel = NULL;
static char      kp_buf[12];
static uint8_t   kp_len = 0;
static float     kp_min = 0, kp_max = 0;
static void    (*kp_cb)(float) = NULL;

// ---------------------------------------------------------------- comandos de la pluma
static bool pen_can_move(void) {
    return sys.state == State::Idle || sys.state == State::Jog;
}

static void pen_goto_z(float z_mm, float feed) {
    if (!pen_can_move()) return;
    char s[48];
#ifdef PEN_SERVO
    snprintf(s, sizeof(s), "$J=G90G21G53Z%.2fF%d\n", (double)z_mm, (int)feed);   // coordenadas de maquina: no dependen de G92/G54
#else
    snprintf(s, sizeof(s), "$J=G90G21Z%.2fF%d\n", (double)z_mm, (int)feed);
#endif
    MKS_GRBL_CMD_SEND(s);
}

void mks_pen_up(void)   { pen_goto_z(pen_up_mm->get(),   pen_draw_feed->get()); }
void mks_pen_down(void) { pen_goto_z(pen_down_mm->get(), pen_plunge_feed->get()); }

void mks_pen_toggle(void) {
    float* p = system_get_mpos();
#ifndef PEN_SERVO
    mpos_to_wpos(p);
#endif
    if (p[2] >= pen_up_mm->get() - PEN_TOL) mks_pen_down();
    else                                    mks_pen_up();
}

void mks_pen_go_origin(void) {      // sube la pluma y vuelve al origen de trabajo en el mismo orden (los dos van al planificador)
    if (!pen_can_move()) return;
    mks_pen_up();
    char s[40];
    snprintf(s, sizeof(s), "$J=G90G21X0Y0F%d\n", (int)pen_draw_feed->get());
    MKS_GRBL_CMD_SEND(s);
}

#ifdef PLOTTER_MODE
// Se llama desde el lazo del trabajo de la SD (Protocol.cpp) justo antes de rebobinar el archivo para otra pasada.
// Las dos lineas pasan por execute_line como cualquier linea del archivo, asi que van al planificador en orden:
// primero sube la pluma, luego vuelve al origen de trabajo en rapido (con la pluma ya arriba).
void mks_pen_between_passes(void) {
    char l1[40], l2[16];
#ifdef PEN_SERVO
    snprintf(l1, sizeof(l1), "G90G53G0Z%.2f", (double)pen_up_mm->get());
#else
    snprintf(l1, sizeof(l1), "G90G0Z%.2f", (double)pen_up_mm->get());
#endif
    snprintf(l2, sizeof(l2), "G90G0X0Y0");
    report_status_message(execute_line(l1, SD_client, SD_auth_level), SD_client);
    report_status_message(execute_line(l2, SD_client, SD_auth_level), SD_client);
}
#endif

// ---------------------------------------------------------------- estilos y piezas
static void pen_styles_init(void) {
    lv_style_copy(&pen_card_style, &lv_style_plain_color);
    pen_card_style.body.main_color = pen_card_style.body.grad_color = PEN_COL_CARD;
    pen_card_style.body.radius = 12;
    pen_card_style.body.border.width = 0;

    lv_style_copy(&pen_btn_style, &pen_card_style);                  // boton normal: fondo oscuro y borde gris
    pen_btn_style.body.main_color = pen_btn_style.body.grad_color = PEN_COL_SIDE;
    pen_btn_style.body.radius = 10;
    pen_btn_style.body.border.width = 1;
    pen_btn_style.body.border.color = PEN_COL_TRACK;
    lv_style_copy(&pen_btn_pr_style, &pen_btn_style);
    pen_btn_pr_style.body.main_color = pen_btn_pr_style.body.grad_color = PEN_COL_TRACK;

    lv_style_copy(&pen_btn_sel_style, &pen_btn_style);               // paso elegido: esmeralda
    pen_btn_sel_style.body.main_color = pen_btn_sel_style.body.grad_color = PEN_COL_EM;
    pen_btn_sel_style.body.border.width = 0;

    lv_style_copy(&pen_btn_blue_style, &pen_btn_style);              // accion principal: azul con borde esmeralda
    pen_btn_blue_style.body.main_color = pen_btn_blue_style.body.grad_color = PEN_COL_BLUE;
    pen_btn_blue_style.body.border.color = PEN_COL_EM;
    pen_btn_blue_style.body.border.width = 2;
    lv_style_copy(&pen_btn_blue_pr_style, &pen_btn_blue_style);
    pen_btn_blue_pr_style.body.main_color = pen_btn_blue_pr_style.body.grad_color = PEN_COL_EM;

    lv_style_copy(&pen_txt_style, &lv_style_plain);
    pen_txt_style.text.font = mc_font();
    pen_txt_style.text.color = LV_COLOR_WHITE;
    lv_style_copy(&pen_dark_style, &pen_txt_style);
    pen_dark_style.text.color = LV_COLOR_MAKE(0x08, 0x0C, 0x18);
    lv_style_copy(&pen_muted_style, &pen_txt_style);
    pen_muted_style.text.color = PEN_COL_MUTED;
    lv_style_copy(&pen_ok_style, &pen_txt_style);
    pen_ok_style.text.color = PEN_COL_EM;
    lv_style_copy(&pen_warn_style, &pen_txt_style);
    pen_warn_style.text.color = PEN_COL_WARN;
    lv_style_copy(&pen_arrow_style, &lv_style_plain);                // simbolos: Roboto (la fuente china no los trae)
    pen_arrow_style.text.font = &lv_font_roboto_22;
    pen_arrow_style.text.color = LV_COLOR_WHITE;
    lv_style_copy(&pen_arrow_ok_style, &pen_arrow_style);
    pen_arrow_ok_style.text.color = PEN_COL_EM;
}

static lv_obj_t* pen_label(lv_obj_t* parent, const char* text, const lv_style_t* st, lv_coord_t x, lv_coord_t y) {
    lv_obj_t* l = lv_label_create(parent, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, (lv_style_t*)st);
    lv_label_set_text(l, text);
    lv_obj_set_pos(l, x, y);
    return l;
}

static lv_obj_t* pen_btn(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                         const lv_style_t* rel, const lv_style_t* pr, lv_event_cb_t cb) {
    lv_obj_t* b = lv_btn_create(parent, NULL);
    lv_obj_set_size(b, w, h);
    lv_obj_set_pos(b, x, y);
    lv_btn_set_style(b, LV_BTN_STYLE_REL, (lv_style_t*)rel);
    lv_btn_set_style(b, LV_BTN_STYLE_PR, (lv_style_t*)pr);
    lv_obj_set_event_cb(b, cb);
    lv_cont_set_layout(b, LV_LAYOUT_OFF);
    return b;
}

static lv_obj_t* pen_btn_text(lv_obj_t* b, const char* text, const lv_style_t* st) {
    lv_obj_t* l = lv_label_create(b, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, (lv_style_t*)st);
    lv_label_set_text(l, text);
    lv_obj_align(l, b, LV_ALIGN_CENTER, 0, 0);
    return l;
}

// Boton grande con simbolo a la izquierda y texto centrado (SUBIR / BAJAR PLUMA)
static lv_obj_t* pen_big_btn(lv_obj_t* parent, lv_coord_t y, const char* symbol, const char* text,
                             const lv_style_t* rel, const lv_style_t* pr, const lv_style_t* sym_style) {
    lv_obj_t* b = pen_btn(parent, 12, y, 200, 52, rel, pr, NULL);
    lv_obj_t* ic = lv_label_create(b, NULL);
    lv_label_set_style(ic, LV_LABEL_STYLE_MAIN, (lv_style_t*)sym_style);
    lv_label_set_text(ic, symbol);
    lv_obj_align(ic, b, LV_ALIGN_IN_LEFT_MID, 16, 0);
    lv_obj_t* tx = lv_label_create(b, NULL);
    lv_label_set_style(tx, LV_LABEL_STYLE_MAIN, &pen_txt_style);
    lv_label_set_text(tx, text);
    lv_obj_align(tx, b, LV_ALIGN_CENTER, 14, 0);
    return b;
}

// ---------------------------------------------------------------- paso y velocidad de los movimientos manuales
static float pen_cur_step(void) { return pen_step_idx < 4 ? pen_steps[pen_step_idx] : pen_custom_step; }

static uint32_t pen_cur_feed(void) {
    if (pen_jog_feed < 50.0f) pen_jog_feed = pen_jog_feed_set->get();
    return (uint32_t)pen_jog_feed;
}

// Velocidad y paso personalizado viven en $45 y $46: se leen al abrir la pantalla y se escriben al cambiarlos.
static void pen_prefs_load(void) {
    pen_jog_feed    = pen_jog_feed_set->get();
    pen_custom_step = pen_custom_step_set->get();
    pen_prefs_dirty = false;
}

static void pen_prefs_mark(void) { pen_prefs_dirty = true; pen_prefs_t = millis(); }

// Escribe $45 y $46 si cambiaron. Espera 1,5 s sin tocar nada (el - y el + cambian la velocidad muchas veces seguidas)
// y solo con la maquina parada, porque los ajustes no se pueden cambiar en movimiento.
static void pen_prefs_flush(bool force) {
    if (!pen_prefs_dirty) return;
    if (!force && millis() - pen_prefs_t < 1500) return;
    if (sys.state != State::Idle && sys.state != State::Alarm) return;
    char s[40];
    if ((int)pen_jog_feed != (int)pen_jog_feed_set->get()) {
        snprintf(s, sizeof(s), "$45=%d\n", (int)pen_jog_feed);
        MKS_GRBL_CMD_SEND(s);
    }
    if (fabsf(pen_custom_step - pen_custom_step_set->get()) > 0.001f) {
        snprintf(s, sizeof(s), "$46=%.2f\n", (double)pen_custom_step);
        MKS_GRBL_CMD_SEND(s);
    }
    pen_prefs_dirty = false;
}

static void pen_refresh_vel(void) {
    if (pen_lbl_vel == NULL || pen_b_vv == NULL) return;
    char b[24];
    snprintf(b, sizeof(b), "%s%d", mks_tr("Vel ", "Spd "), (int)pen_cur_feed());
    lv_label_set_text(pen_lbl_vel, b);
    lv_obj_align(pen_lbl_vel, pen_b_vv, LV_ALIGN_CENTER, 0, 0);
}

static void pen_refresh_custom_label(void) {
    if (pen_step_lbl[4] == NULL) return;
    char b[16];
    snprintf(b, sizeof(b), "%g", (double)pen_custom_step);
    lv_label_set_text(pen_step_lbl[4], b);
    lv_obj_align(pen_step_lbl[4], pen_b_step[4], LV_ALIGN_CENTER, 0, 0);
}

static void pen_adjust_feed(int dir) {                      // - / +: de 100 en 100 hasta 1000 y de 500 en 500 por encima
    float v = (float)pen_cur_feed();
    float d = (dir > 0) ? (v >= 1000.0f ? 500.0f : 100.0f) : (v > 1000.0f ? 500.0f : 100.0f);
    v += dir * d;
    if (v < 100.0f) v = 100.0f;
    if (v > 10000.0f) v = 10000.0f;
    pen_jog_feed = v;
    pen_refresh_vel();
    pen_prefs_mark();
}

static void pen_set_jog_feed(float v)     { pen_jog_feed = v; pen_refresh_vel(); pen_prefs_mark(); }
static void pen_set_custom_step(float v)  { pen_custom_step = v; pen_refresh_custom_label(); pen_prefs_mark(); }

// ---------------------------------------------------------------- teclado numerico
static void kp_update(void) { if (kp_val) lv_label_set_text(kp_val, kp_len ? kp_buf : "0"); }

static void kp_close(void) {
    if (kp_cont) { lv_obj_del(kp_cont); kp_cont = NULL; kp_val = NULL; }
}

static void kp_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED || kp_cont == NULL) return;
    if (obj == kp_cancel) { kp_close(); return; }
    if (obj == kp_ok) {
        float v = (float)atof(kp_buf);
        if (kp_len == 0 || v <= 0.0f) return;               // sin valor: no cierra
        if (v < kp_min) v = kp_min;
        if (v > kp_max) v = kp_max;
        void (*cb)(float) = kp_cb;
        kp_close();
        if (cb) cb(v);
        return;
    }
    static const char keys[12] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '0', '<' };
    for (int i = 0; i < 12; i++) {
        if (obj != kp_key[i]) continue;
        const char ch = keys[i];
        if (ch == '<') {
            if (kp_len) kp_buf[--kp_len] = 0;
        } else if (ch == '.') {
            if (kp_len < 7 && strchr(kp_buf, '.') == NULL) {
                if (kp_len == 0) kp_buf[kp_len++] = '0';
                kp_buf[kp_len++] = '.';
                kp_buf[kp_len] = 0;
            }
        } else if (kp_len < 7) {
            kp_buf[kp_len++] = ch;
            kp_buf[kp_len] = 0;
        }
        kp_update();
        return;
    }
}

static void kp_open(const char* title, float current, float vmin, float vmax, void (*cb)(float)) {
    if (kp_cont) return;
    kp_min = vmin; kp_max = vmax; kp_cb = cb; kp_len = 0; kp_buf[0] = 0;

    kp_cont = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(kp_cont, 412, 300);
    lv_obj_set_pos(kp_cont, 34, 10);
    lv_obj_set_style(kp_cont, &pen_card_style);
    pen_label(kp_cont, title, &pen_muted_style, 14, 10);

    lv_obj_t* box = lv_obj_create(kp_cont, NULL);             // caja con el valor tecleado
    lv_obj_set_size(box, 384, 44);
    lv_obj_set_pos(box, 14, 36);
    lv_obj_set_style(box, &pen_btn_style);
    kp_val = pen_label(box, "0", &pen_ok_style, 12, 12);

    static const char* txt[12] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "<" };
    for (int i = 0; i < 12; i++) {
        kp_key[i] = pen_btn(kp_cont, 14 + (i % 3) * 70, 92 + (i / 3) * 50, 64, 44, &pen_btn_style, &pen_btn_pr_style, kp_event);
        pen_btn_text(kp_key[i], txt[i], &pen_txt_style);
    }
    kp_ok = pen_btn(kp_cont, 244, 92, 154, 60, &pen_btn_blue_style, &pen_btn_blue_pr_style, kp_event);
    pen_btn_text(kp_ok, "OK", &pen_txt_style);
    kp_cancel = pen_btn(kp_cont, 244, 160, 154, 44, &pen_btn_style, &pen_btn_pr_style, kp_event);
    pen_btn_text(kp_cancel, mks_tr("Cancelar", "Cancel"), &pen_txt_style);

    char hint[64];
    snprintf(hint, sizeof(hint), "%s %g", mks_tr("Actual:", "Current:"), (double)current);
    pen_label(kp_cont, hint, &pen_muted_style, 244, 216);
    snprintf(hint, sizeof(hint), "%g - %g", (double)vmin, (double)vmax);
    pen_label(kp_cont, hint, &pen_muted_style, 244, 240);
}

// ---------------------------------------------------------------- Home (con la pluma arriba)
static bool pen_is_up(void) {
    float* p = system_get_mpos();
#ifndef PEN_SERVO
    mpos_to_wpos(p);
#endif
    return p[2] >= pen_up_mm->get() - PEN_TOL;
}

static void pen_popup_close(lv_obj_t* obj, lv_event_t event);

static void pen_start_home(void) {
    if (sys.state != State::Idle && sys.state != State::Alarm) {
        mks_draw_common_popup_info_com(mc_language.dis_warning, mc_language.dis_wait_mc_stop, " ", pen_popup_close);
        return;
    }
    pen_home_stage = 1;
    pen_home_t0 = millis();
    pen_home_unlocked = false;
}

// La llama mks_pen_update (cada 200 ms): desbloquea si hay alarma, sube la pluma, espera a que la maquina pare y lanza $H.
static void pen_home_tick(void) {
    if (pen_home_stage == 0) return;
    if (millis() - pen_home_t0 > 20000) { pen_home_stage = 0; return; }       // algo no avanzo: se abandona
    if (sys.state == State::Alarm) {
        if (!pen_home_unlocked) { MKS_GRBL_CMD_SEND("$X\n"); pen_home_unlocked = true; }
        return;
    }
    if (sys.state != State::Idle) return;
    if (pen_home_stage == 1) {
        if (pen_is_up()) pen_home_stage = 3;
        else { mks_pen_up(); pen_home_stage = 2; }
    } else if (pen_home_stage == 2) {
        if (pen_is_up()) pen_home_stage = 3;
    }
    if (pen_home_stage == 3) { MKS_GRBL_CMD_SEND("$H\n"); pen_home_stage = 0; }
}

// ---------------------------------------------------------------- eventos
static void pen_go_back(void) {
    pen_prefs_flush(true);                         // si se cambio algo y la maquina esta parada, se guarda antes de salir
    kp_cont = NULL; kp_val = NULL;                 // lv_obj_clean() ya los borra
    pen_home_stage = 0;
    pen_lbl_vel = pen_b_vv = pen_b_hm = pen_b_vm = pen_b_vp = NULL;
    pen_step_lbl[4] = NULL;
    pen_lbl_pos = pen_lbl_state = pen_bar = NULL;
    lv_obj_clean(mks_global.mks_src);
    mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
    mks_ui_page.wait_count = DEFAULT_UI_COUNT;
    mks_draw_ready();
}

static void pen_popup_close(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) common_popup_com_del();
}

static void pen_set_zero(const char* cmd) {
    if (sys.state == State::Idle && mks_get_motor_status()) {
        MKS_GRBL_CMD_SEND(cmd);
        mks_draw_common_popup_info_com(mc_language.dis_info, mc_language.dis_pos_succeed, " ", pen_popup_close);
    } else {
        mks_draw_common_popup_info_com(mc_language.dis_warning, mc_language.dis_wait_mc_stop, " ", pen_popup_close);
    }
}

static void pen_select_step(uint8_t idx) {
    pen_step_idx = idx;
    for (int i = 0; i < PEN_NSTEP; i++) {
        bool on = (i == idx);
        lv_btn_set_style(pen_b_step[i], LV_BTN_STYLE_REL, on ? &pen_btn_sel_style : &pen_btn_style);
        lv_btn_set_style(pen_b_step[i], LV_BTN_STYLE_PR, on ? &pen_btn_sel_style : &pen_btn_pr_style);
        lv_label_set_style(pen_step_lbl[i], LV_LABEL_STYLE_MAIN, on ? &pen_dark_style : &pen_txt_style);
    }
}

static void pen_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    const uint32_t feed = pen_cur_feed();            // velocidad de los movimientos manuales (editable en pantalla)
    const float    st   = pen_cur_step();            // paso: uno de los 4 fijos o el personalizado

    if      (obj == pen_b_back) pen_go_back();
    else if (obj == pen_b_hm)   pen_start_home();
    else if (obj == pen_b_vm)   pen_adjust_feed(-1);
    else if (obj == pen_b_vp)   pen_adjust_feed(+1);
    else if (obj == pen_b_vv)   kp_open(mks_tr("Velocidad (mm/min)", "Speed (mm/min)"), (float)feed, 50.0f, 10000.0f, pen_set_jog_feed);
    else if (obj == pen_b_yp)   { if (pen_can_move()) set_move('Y',  st, feed); }
    else if (obj == pen_b_yn)   { if (pen_can_move()) set_move('Y', -st, feed); }
    else if (obj == pen_b_xn)   { if (pen_can_move()) set_move('X', -st, feed); }
    else if (obj == pen_b_xp)   { if (pen_can_move()) set_move('X',  st, feed); }
    else if (obj == pen_b_cxy)  pen_set_zero("G92X0Y0\n");
#ifndef PEN_SERVO       // con servo, Z=0 es una posicion del servo y un desplazamiento G92 solo la descolocaria
    else if (obj == pen_b_cz)   pen_set_zero("G92Z0\n");
#endif
    else if (obj == pen_b_up)   mks_pen_up();
    else if (obj == pen_b_down) mks_pen_down();
    else if (obj == pen_b_home) mks_pen_go_origin();    // sube la pluma y vuelve al origen de trabajo
    else {
        for (int i = 0; i < PEN_NSTEP; i++) {
            if (obj != pen_b_step[i]) continue;
            pen_select_step(i);
            if (i == 4) kp_open(mks_tr("Paso personalizado (mm)", "Custom step (mm)"), pen_custom_step, 0.01f, 400.0f, pen_set_custom_step);
            return;
        }
    }
}

// ---------------------------------------------------------------- pantalla
void mks_draw_pen(void) {
    pen_styles_init();
    pen_prefs_load();                              // velocidad ($45) y paso personalizado ($46) guardados

    // barra superior: volver, titulo y coordenadas de trabajo
    pen_bar = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(pen_bar, 468, 36);
    lv_obj_set_pos(pen_bar, 6, 4);
    lv_obj_set_style(pen_bar, &pen_card_style);
    pen_b_back = pen_btn(pen_bar, 4, 2, 56, 32, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_back, LV_SYMBOL_LEFT, &pen_arrow_style);
    pen_label(pen_bar, mks_tr("Pluma y mover", "Pen and move"), &pen_txt_style, 72, 9);
    pen_lbl_pos = pen_label(pen_bar, "X 0.0   Y 0.0   Z 0.0", &pen_ok_style, 0, 9);
    lv_obj_align(pen_lbl_pos, pen_bar, LV_ALIGN_IN_RIGHT_MID, -12, 0);

    // cruceta de movimiento X e Y: celdas de 68 px con 6 px de hueco (zona tactil comoda)
    const lv_coord_t px = 14, py = 48, c = 68, g = 6;
    pen_b_yp = pen_btn(mks_global.mks_src, px + c + g,       py,               c, c, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_b_xn = pen_btn(mks_global.mks_src, px,               py + c + g,       c, c, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_b_xp = pen_btn(mks_global.mks_src, px + 2 * (c + g), py + c + g,       c, c, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_b_yn = pen_btn(mks_global.mks_src, px + c + g,       py + 2 * (c + g), c, c, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_yp, LV_SYMBOL_UP,    &pen_arrow_style);
    pen_btn_text(pen_b_xn, LV_SYMBOL_LEFT,  &pen_arrow_style);
    pen_btn_text(pen_b_xp, LV_SYMBOL_RIGHT, &pen_arrow_style);
    pen_btn_text(pen_b_yn, LV_SYMBOL_DOWN,  &pen_arrow_style);
    pen_b_cxy = pen_btn(mks_global.mks_src, px + c + g, py + c + g, c, c, &pen_btn_blue_style, &pen_btn_blue_pr_style, pen_event);
    pen_btn_text(pen_b_cxy, mks_tr("Cero XY", "Zero XY"), &pen_txt_style);

    // Home: esquina libre de la cruceta (arriba a la izquierda). Sube la pluma y lanza $H.
    pen_b_hm = pen_btn(mks_global.mks_src, px, py, c, c, &pen_btn_style, &pen_btn_pr_style, pen_event);
    {
        lv_obj_t* ic = lv_label_create(pen_b_hm, NULL);
        lv_label_set_style(ic, LV_LABEL_STYLE_MAIN, &pen_arrow_ok_style);
        lv_label_set_text(ic, LV_SYMBOL_HOME);
        lv_obj_align(ic, pen_b_hm, LV_ALIGN_CENTER, 0, -11);
        lv_obj_t* tx = lv_label_create(pen_b_hm, NULL);
        lv_label_set_style(tx, LV_LABEL_STYLE_MAIN, &pen_txt_style);
        lv_label_set_text(tx, "Home");
        lv_obj_align(tx, pen_b_hm, LV_ALIGN_CENTER, 0, 17);
    }

    // paso del movimiento en mm: 4 fijos y uno personalizado (se teclea)
    pen_label(mks_global.mks_src, mks_tr("PASO (mm)", "STEP (mm)"), &pen_muted_style, px, 250);
    for (int i = 0; i < PEN_NSTEP; i++) {
        pen_b_step[i] = pen_btn(mks_global.mks_src, px + i * 44, 268, 40, 42, &pen_btn_style, &pen_btn_pr_style, pen_event);
        pen_step_lbl[i] = pen_btn_text(pen_b_step[i], i < 4 ? pen_step_txt[i] : "", &pen_txt_style);
    }
    pen_refresh_custom_label();
    pen_select_step(pen_step_idx);

    // panel de la pluma
    lv_obj_t* panel = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(panel, 224, 262);
    lv_obj_set_pos(panel, 250, 48);
    lv_obj_set_style(panel, &pen_card_style);
    pen_label(panel, mks_tr("PLUMA", "PEN"), &pen_muted_style, 14, 8);

    pen_b_up = pen_big_btn(panel, 26, LV_SYMBOL_UP, mks_tr("SUBIR PLUMA", "PEN UP"),
                           &pen_btn_blue_style, &pen_btn_blue_pr_style, &pen_arrow_ok_style);
    pen_b_down = pen_big_btn(panel, 82, LV_SYMBOL_DOWN, mks_tr("BAJAR PLUMA", "PEN DOWN"),
                             &pen_btn_style, &pen_btn_pr_style, &pen_arrow_style);
    lv_obj_set_event_cb(pen_b_up, pen_event);
    lv_obj_set_event_cb(pen_b_down, pen_event);

    pen_label(panel, mks_tr("Estado:", "State:"), &pen_muted_style, 14, 140);
    pen_lbl_state = pen_label(panel, "--", &pen_ok_style, 70, 140);

    // velocidad de los movimientos manuales: - [Vel xxxx] +  (tocar el valor abre el teclado)
    pen_b_vm = pen_btn(panel, 12, 162, 44, 38, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_vm, "-", &pen_txt_style);
    pen_b_vv = pen_btn(panel, 60, 162, 104, 38, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_lbl_vel = pen_btn_text(pen_b_vv, "", &pen_ok_style);
    pen_b_vp = pen_btn(panel, 168, 162, 44, 38, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_vp, "+", &pen_txt_style);
    pen_refresh_vel();

#ifdef PEN_SERVO       // con servo no hay "Cero Z" (Z es una posicion del servo): A origen ocupa todo el ancho
    pen_b_home = pen_btn(panel, 12, 206, 200, 48, &pen_btn_style, &pen_btn_pr_style, pen_event);
#else
    pen_b_cz = pen_btn(panel, 12, 206, 98, 48, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_cz, mks_tr("Cero Z", "Zero Z"), &pen_txt_style);
    pen_b_home = pen_btn(panel, 114, 206, 98, 48, &pen_btn_style, &pen_btn_pr_style, pen_event);
#endif
    pen_btn_text(pen_b_home, mks_tr("A origen", "To origin"), &pen_txt_style);

    mks_pen_update();
    mks_ui_page.mks_ui_page = MKS_UI_Pen;
    mks_ui_page.wait_count = DEFAULT_UI_COUNT;
}

// Coordenadas de trabajo y estado de la pluma segun la Z actual
void mks_pen_update(void) {
    pen_home_tick();      // secuencia del boton Home (pluma arriba, esperar y $H)
    pen_prefs_flush(false);                        // guarda velocidad y paso 1,5 s despues del ultimo cambio
    if (pen_lbl_pos == NULL || pen_lbl_state == NULL || pen_bar == NULL) return;

    float* p = system_get_mpos();
#ifndef PEN_SERVO
    mpos_to_wpos(p);
#endif

    char buf[48];
    snprintf(buf, sizeof(buf), "X %.1f   Y %.1f   Z %.1f", (double)p[0], (double)p[1], (double)p[2]);
    lv_label_set_text(pen_lbl_pos, buf);
    lv_obj_align(pen_lbl_pos, pen_bar, LV_ALIGN_IN_RIGHT_MID, -12, 0);

    const float up = pen_up_mm->get(), dn = pen_down_mm->get(), z = p[2];
    const lv_style_t* st = &pen_warn_style;
    const char* name = mks_tr("EN MOVIMIENTO", "MOVING");
    if (z >= up - PEN_TOL)      { name = mks_tr("ARRIBA", "UP");  st = &pen_ok_style; }
    else if (z <= dn + PEN_TOL) { name = mks_tr("ABAJO", "DOWN"); st = &pen_ok_style; }
    snprintf(buf, sizeof(buf), "%s  (Z %.1f)", name, (double)z);
    lv_label_set_style(pen_lbl_state, LV_LABEL_STYLE_MAIN, (lv_style_t*)st);
    lv_label_set_text(pen_lbl_state, buf);
}
