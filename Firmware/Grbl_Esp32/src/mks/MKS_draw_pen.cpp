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
static lv_obj_t *pen_b_step[4], *pen_step_lbl[4];
static lv_obj_t *pen_lbl_pos = NULL, *pen_lbl_state = NULL, *pen_bar = NULL;
static uint8_t   pen_step_idx = 1;                                   // 1 mm por defecto
static const float pen_steps[4] = { 0.1f, 1.0f, 10.0f, 50.0f };
static const char* pen_step_txt[4] = { "0.1", "1", "10", "50" };

// ---------------------------------------------------------------- comandos de la pluma
static bool pen_can_move(void) {
    return sys.state == State::Idle || sys.state == State::Jog;
}

static void pen_goto_z(float z_mm, float feed) {
    if (!pen_can_move()) return;
    char s[48];
    snprintf(s, sizeof(s), "$J=G90G21Z%.2fF%d\n", (double)z_mm, (int)feed);
    MKS_GRBL_CMD_SEND(s);
}

void mks_pen_up(void)   { pen_goto_z(pen_up_mm->get(),   pen_draw_feed->get()); }
void mks_pen_down(void) { pen_goto_z(pen_down_mm->get(), pen_plunge_feed->get()); }

void mks_pen_toggle(void) {
    float* p = system_get_mpos();
    mpos_to_wpos(p);
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
    snprintf(l1, sizeof(l1), "G90G0Z%.2f", (double)pen_up_mm->get());
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
    lv_obj_t* b = pen_btn(parent, 12, y, 200, 66, rel, pr, NULL);
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

// ---------------------------------------------------------------- eventos
static void pen_go_back(void) {
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
    for (int i = 0; i < 4; i++) {
        bool on = (i == idx);
        lv_btn_set_style(pen_b_step[i], LV_BTN_STYLE_REL, on ? &pen_btn_sel_style : &pen_btn_style);
        lv_btn_set_style(pen_b_step[i], LV_BTN_STYLE_PR, on ? &pen_btn_sel_style : &pen_btn_pr_style);
        lv_label_set_style(pen_step_lbl[i], LV_LABEL_STYLE_MAIN, on ? &pen_dark_style : &pen_txt_style);
    }
}

static void pen_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    const uint32_t feed = (uint32_t)pen_draw_feed->get();
    const float    st   = pen_steps[pen_step_idx];

    if      (obj == pen_b_back) pen_go_back();
    else if (obj == pen_b_yp)   { if (pen_can_move()) set_move('Y',  st, feed); }
    else if (obj == pen_b_yn)   { if (pen_can_move()) set_move('Y', -st, feed); }
    else if (obj == pen_b_xn)   { if (pen_can_move()) set_move('X', -st, feed); }
    else if (obj == pen_b_xp)   { if (pen_can_move()) set_move('X',  st, feed); }
    else if (obj == pen_b_cxy)  pen_set_zero("G92X0Y0\n");
    else if (obj == pen_b_cz)   pen_set_zero("G92Z0\n");
    else if (obj == pen_b_up)   mks_pen_up();
    else if (obj == pen_b_down) mks_pen_down();
    else if (obj == pen_b_home) mks_pen_go_origin();    // sube la pluma y vuelve al origen de trabajo
    else {
        for (int i = 0; i < 4; i++) {
            if (obj == pen_b_step[i]) { pen_select_step(i); return; }
        }
    }
}

// ---------------------------------------------------------------- pantalla
void mks_draw_pen(void) {
    pen_styles_init();

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

    // paso del movimiento en mm
    pen_label(mks_global.mks_src, mks_tr("PASO (mm)", "STEP (mm)"), &pen_muted_style, px, 250);
    for (int i = 0; i < 4; i++) {
        pen_b_step[i] = pen_btn(mks_global.mks_src, px + i * 56, 268, 52, 42, &pen_btn_style, &pen_btn_pr_style, pen_event);
        pen_step_lbl[i] = pen_btn_text(pen_b_step[i], pen_step_txt[i], &pen_txt_style);
    }
    pen_select_step(pen_step_idx);

    // panel de la pluma
    lv_obj_t* panel = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(panel, 224, 262);
    lv_obj_set_pos(panel, 250, 48);
    lv_obj_set_style(panel, &pen_card_style);
    pen_label(panel, mks_tr("PLUMA", "PEN"), &pen_muted_style, 14, 8);

    pen_b_up = pen_big_btn(panel, 28, LV_SYMBOL_UP, mks_tr("SUBIR PLUMA", "PEN UP"),
                           &pen_btn_blue_style, &pen_btn_blue_pr_style, &pen_arrow_ok_style);
    pen_b_down = pen_big_btn(panel, 102, LV_SYMBOL_DOWN, mks_tr("BAJAR PLUMA", "PEN DOWN"),
                             &pen_btn_style, &pen_btn_pr_style, &pen_arrow_style);
    lv_obj_set_event_cb(pen_b_up, pen_event);
    lv_obj_set_event_cb(pen_b_down, pen_event);

    pen_label(panel, mks_tr("Estado:", "State:"), &pen_muted_style, 14, 184);
    pen_lbl_state = pen_label(panel, "--", &pen_ok_style, 70, 184);

    pen_b_cz = pen_btn(panel, 12, 206, 98, 48, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_cz, mks_tr("Cero Z", "Zero Z"), &pen_txt_style);
    pen_b_home = pen_btn(panel, 114, 206, 98, 48, &pen_btn_style, &pen_btn_pr_style, pen_event);
    pen_btn_text(pen_b_home, mks_tr("A origen", "To origin"), &pen_txt_style);

    mks_pen_update();
    mks_ui_page.mks_ui_page = MKS_UI_Pen;
    mks_ui_page.wait_count = DEFAULT_UI_COUNT;
}

// Coordenadas de trabajo y estado de la pluma segun la Z actual
void mks_pen_update(void) {
    if (pen_lbl_pos == NULL || pen_lbl_state == NULL || pen_bar == NULL) return;

    float* p = system_get_mpos();
    mpos_to_wpos(p);

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
