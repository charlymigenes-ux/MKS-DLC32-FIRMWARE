#include "MKS_draw_print.h"
#include "MKS_draw_language.h"   // mc_language: textos del LCD
#include "../System.h"

PWR_CTRL_t mks_pwr_ctrl;
SPEED_CTRL_t mks_speed_ctrl;
MKS_PRINT_PAGE_t print_src;
MKS_PRINT_DATA_UPDATA_t print_data_updata;
MKS_PRINT_SETTING_T print_setting;

uint32_t ddxd;

/* btn */
static lv_obj_t* btn_popup_cancle;
static lv_obj_t* btn_popup_sure;
static lv_obj_t* btn_finsh_popup_sure;

lv_obj_t* Label_print_file_name;

LV_IMG_DECLARE(M_Pause);  // 暂停
LV_IMG_DECLARE(M_start);  // 开始
LV_IMG_DECLARE(M_Stop);  // 停止
LV_IMG_DECLARE(M_PWRr);  // 功率
LV_IMG_DECLARE(M_SPEED);  // 速度
LV_IMG_DECLARE(back);  // 速度

LV_IMG_DECLARE(add);  // 加
LV_IMG_DECLARE(confirm);  // 确认
LV_IMG_DECLARE(s_return);  // 确认
LV_IMG_DECLARE(reduce);  // 减

LV_IMG_DECLARE(png_cave_pwr);
LV_IMG_DECLARE(png_cave_speed);
LV_IMG_DECLARE(png_cave_xpos);
LV_IMG_DECLARE(png_cave_ypos);
LV_IMG_DECLARE(png_cave_zpos);
LV_IMG_DECLARE(png_pause_pre);          // 暂停
LV_IMG_DECLARE(png_start_pre);          // 开始
LV_IMG_DECLARE(png_stop_pre);           // 停止
LV_IMG_DECLARE(png_cave_pwr_pre);       // 功率
LV_IMG_DECLARE(png_cave_speed_pre);     // 速度
LV_IMG_DECLARE(png_times);              // 雕刻次数

static void job_pause_button_update(bool paused);

// Nota: en las imagenes png_start (barras ||) y png_pause (triangulo) los nombres estan
// cruzados. El boton ofrece la accion SIGUIENTE: en marcha -> || "Pausar"; en pausa -> play "Reanudar".
static void event_handler_suspend(lv_obj_t* obj, lv_event_t event) {

    if (event == LV_EVENT_RELEASED) {

        if(sys.state == State::Hold) {
            job_pause_button_update(false);   // se reanuda: ahora ofrece Pausar
            MKS_GRBL_CMD_SEND("~");
            if(print_setting._need_to_start_write) {
                sys_rt_s_override = print_setting.cur_spindle_pwr;
            }
        }   
        else if(sys.state == State::Cycle)    {
            job_pause_button_update(true);    // se pausa: ahora ofrece Reanudar
            MKS_GRBL_CMD_SEND("!");
            // spindle->stop();
        } 
    }
}

static void event_handler_stop(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        mks_draw_print_popup(mc_language.dis_print_stop_sure);
    }
}


static void event_handler_op(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        mks_clear_print();
        mks_draw_operation();
    }
}

static void event_handler_adj(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        set_print_click(false);
        draw_adj_popup();
    }
}

static void event_handler_none(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

    }
}




/* ===========================================================================
 * Pantalla de trabajo: tres indicadores F / S / R con flechas de ajuste en vivo,
 * barra de progreso fina y datos del trabajo (tiempo, restante, avance y potencia
 * reales). El boton Ajuste abre el panel con Aumentar / Reducir y paso de 1/10/25 %.
 * Pantalla de 480x320: cabecera y=6, barra y=36, tarjetas y=58..182, datos y=190..231,
 * botones Pausar / Parar / Ajuste y=250.
 * ======================================================================== */
enum { G_FEED, G_SPINDLE, G_RAPID, G_COUNT };

#define JOB_BAR_X        12
#define JOB_BAR_Y        36
#define JOB_BAR_W        456
#define JOB_BAR_H        12
#define GAUGE_CARD_X0    12
#define GAUGE_CARD_Y     58
#define GAUGE_CARD_W     148
#define GAUGE_CARD_H     124
#define GAUGE_CARD_GAP   6
#define GAUGE_ARC_SIZE   76
#define GAUGE_ARROW_W    30
#define GAUGE_ARROW_H    52
#define JOB_STATS_Y      190
#define JOB_STATS_COL2_X 250

#define COL_CARD    LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define COL_TRACK   LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define COL_BTN     LV_COLOR_MAKE(0x2A, 0x30, 0x50)
#define COL_EMERALD LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define COL_BLUE    LV_COLOR_MAKE(0x2B, 0xB5, 0xFF)
#define COL_CORAL   LV_COLOR_MAKE(0xFF, 0x5C, 0x5C)
#define COL_MUTED   LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)

typedef struct {
    lv_obj_t*  card;
    lv_obj_t*  arc_track;
    lv_obj_t*  arc_fg;
    lv_obj_t*  btn_dec;
    lv_obj_t*  btn_inc;
    lv_obj_t*  val;
    lv_obj_t*  cap;
    lv_style_t fg_style;
    int16_t    shown;  // ultimo valor dibujado (-1 = ninguno)
} job_gauge_t;

static job_gauge_t job_gauge[G_COUNT];
static lv_style_t  job_card_style, job_track_style, job_val_style, job_cap_style;
static lv_style_t  job_arrow_rel_style, job_arrow_pr_style, job_info_style;
static lv_obj_t *  job_lbl_elapsed, *job_lbl_left, *job_lbl_feed, *job_lbl_power;
static char        job_elapsed_str[40], job_left_str[40], job_feed_str[40], job_power_str[40];
static char        job_val_str[G_COUNT][8];
static char        job_cap_str[G_COUNT][28];
static uint32_t    job_t0_ms = 0;  // inicio del trabajo (0 = sin trabajo)

static void job_gauge_range(int g, int* lo, int* hi) {
    if (g == G_FEED) {
        *lo = FeedOverride::Min;
        *hi = FeedOverride::Max;
    } else if (g == G_SPINDLE) {
        *lo = SpindleSpeedOverride::Min;
        *hi = SpindleSpeedOverride::Max;
    } else {
        *lo = RapidOverride::Low;
        *hi = RapidOverride::Default;
    }
}

static int job_gauge_get(int g) {
    if (g == G_FEED) return sys_rt_f_override;
    if (g == G_SPINDLE) return sys_rt_s_override;
    return sys_rt_r_override;
}

static int range_pct(int v, int lo, int hi) {
    int p = (v - lo) * 100 / (hi - lo);
    return p < 0 ? 0 : (p > 100 ? 100 : p);
}

// El arco por defecto de LVGL 6 va de 45 a 315 grados (0 = abajo, 90 = derecha) con la
// abertura abajo. El tramo relleno nace en 315 (abajo-izquierda) y crece hacia la derecha.
static void job_gauge_draw(int g, int v) {
    job_gauge_t* G = &job_gauge[g];
    int          lo, hi;
    job_gauge_range(g, &lo, &hi);
    int sweep = 270 * range_pct(v, lo, hi) / 100;
    if (sweep < 4) sweep = 4;
    lv_arc_set_angles(G->arc_fg, 315 - sweep, 315);
    snprintf(job_val_str[g], sizeof(job_val_str[g]), "%d%%", v);
    lv_label_set_text(G->val, job_val_str[g]);
    lv_obj_align(G->val, G->arc_fg, LV_ALIGN_CENTER, 0, 0);
    G->shown = v;
}

// dir = +1 / -1. La rapida solo admite 25 / 50 / 100 %.
static void job_gauge_step(int g, int dir, int step) {
    int v = job_gauge_get(g);
    if (g == G_FEED) {
        v += dir * step;
        if (v > FeedOverride::Max) v = FeedOverride::Max;
        if (v < FeedOverride::Min) v = FeedOverride::Min;
        sys_rt_f_override               = v;
        print_setting.cur_spindle_speed = v;
    } else if (g == G_SPINDLE) {
        v += dir * step;
        if (v > SpindleSpeedOverride::Max) v = SpindleSpeedOverride::Max;
        if (v < SpindleSpeedOverride::Min) v = SpindleSpeedOverride::Min;
        sys_rt_s_override             = v;
        print_setting.cur_spindle_pwr = v;
    } else {
        if (dir > 0) v = (v < RapidOverride::Medium) ? RapidOverride::Medium : RapidOverride::Default;
        else v = (v > RapidOverride::Medium) ? RapidOverride::Medium : RapidOverride::Low;
        sys_rt_r_override               = v;
        print_setting.cur_spindle_rapid = v;
    }
    job_gauge_draw(g, v);
}

// Un toque cambia 1 %; manteniendo pulsado repite y, pasadas 8 repeticiones, sube de 5 en 5.
static void job_gauge_arrow_event(lv_obj_t* obj, lv_event_t event) {
    static uint8_t reps = 0;
    int            g = -1, dir = 0;
    for (int i = 0; i < G_COUNT; i++) {
        if (obj == job_gauge[i].btn_dec) { g = i; dir = -1; }
        if (obj == job_gauge[i].btn_inc) { g = i; dir = +1; }
    }
    if (g < 0) return;
    if (event == LV_EVENT_SHORT_CLICKED) {
        reps = 0;
        job_gauge_step(g, dir, 1);
    } else if (event == LV_EVENT_LONG_PRESSED_REPEAT) {
        if (g == G_RAPID) return;  // solo tres valores: sin repeticion
        if (reps < 255) reps++;
        job_gauge_step(g, dir, reps > 8 ? 5 : 1);
    } else if (event == LV_EVENT_RELEASED || event == LV_EVENT_PRESS_LOST) {
        reps = 0;
    }
}

static void job_styles_init(void) {
    lv_style_copy(&job_card_style, &lv_style_plain_color);
    job_card_style.body.main_color   = COL_CARD;
    job_card_style.body.grad_color   = COL_CARD;
    job_card_style.body.radius       = 12;
    job_card_style.body.border.width = 0;

    lv_style_copy(&job_track_style, &lv_style_plain);
    job_track_style.line.width   = 8;
    job_track_style.line.color   = COL_TRACK;
    job_track_style.line.rounded = 1;

    lv_style_copy(&job_val_style, &lv_style_plain);
    job_val_style.text.font  = &lv_font_roboto_22;
    job_val_style.text.color = LV_COLOR_WHITE;

    lv_style_copy(&job_cap_style, &lv_style_plain);
    job_cap_style.text.font  = mc_font();
    job_cap_style.text.color = COL_MUTED;

    lv_style_copy(&job_info_style, &lv_style_plain);
    job_info_style.text.font  = mc_font();
    job_info_style.text.color = LV_COLOR_WHITE;

    lv_style_copy(&job_arrow_rel_style, &lv_style_plain_color);
    job_arrow_rel_style.body.main_color   = COL_BTN;
    job_arrow_rel_style.body.grad_color   = COL_BTN;
    job_arrow_rel_style.body.radius       = 10;
    job_arrow_rel_style.body.border.width = 0;
    job_arrow_rel_style.text.font         = &lv_font_roboto_22;
    job_arrow_rel_style.text.color        = LV_COLOR_WHITE;
    lv_style_copy(&job_arrow_pr_style, &job_arrow_rel_style);
    job_arrow_pr_style.body.main_color = COL_EMERALD;
    job_arrow_pr_style.body.grad_color = COL_EMERALD;
    job_arrow_pr_style.text.color      = COL_CARD;
}

static lv_obj_t* job_arrow_create(lv_obj_t* card, lv_coord_t x, const char* sym) {
    lv_obj_t* btn = lv_btn_create(card, NULL);
    lv_obj_set_size(btn, GAUGE_ARROW_W, GAUGE_ARROW_H);
    lv_obj_set_pos(btn, x, 20);
    lv_btn_set_style(btn, LV_BTN_STYLE_REL, &job_arrow_rel_style);
    lv_btn_set_style(btn, LV_BTN_STYLE_PR, &job_arrow_pr_style);
    lv_obj_set_event_cb(btn, job_gauge_arrow_event);
    lv_obj_t* l = lv_label_create(btn, NULL);
    lv_label_set_text(l, sym);
    return btn;
}

static void job_gauges_create(void) {
    const lv_color_t colors[G_COUNT] = { COL_EMERALD, COL_BLUE, COL_CORAL };
    const char*      letters[G_COUNT] = { "F", "S", "R" };
    const char*      names[G_COUNT]   = { mc_language.gauge_feed, mc_language.gauge_spindle, mc_language.gauge_rapid };

    for (int g = 0; g < G_COUNT; g++) {
        job_gauge_t* G = &job_gauge[g];
        lv_coord_t   x = GAUGE_CARD_X0 + g * (GAUGE_CARD_W + GAUGE_CARD_GAP);

        G->card = lv_obj_create(mks_global.mks_src, NULL);
        lv_obj_set_size(G->card, GAUGE_CARD_W, GAUGE_CARD_H);
        lv_obj_set_pos(G->card, x, GAUGE_CARD_Y);
        lv_obj_set_style(G->card, &job_card_style);

        lv_coord_t arc_x = (GAUGE_CARD_W - GAUGE_ARC_SIZE) / 2;
        G->arc_track     = lv_arc_create(G->card, NULL);
        lv_arc_set_style(G->arc_track, LV_ARC_STYLE_MAIN, &job_track_style);
        lv_obj_set_size(G->arc_track, GAUGE_ARC_SIZE, GAUGE_ARC_SIZE);
        lv_obj_set_pos(G->arc_track, arc_x, 8);
        lv_obj_set_click(G->arc_track, false);

        lv_style_copy(&G->fg_style, &job_track_style);
        G->fg_style.line.color = colors[g];
        G->arc_fg              = lv_arc_create(G->card, NULL);
        lv_arc_set_style(G->arc_fg, LV_ARC_STYLE_MAIN, &G->fg_style);
        lv_obj_set_size(G->arc_fg, GAUGE_ARC_SIZE, GAUGE_ARC_SIZE);
        lv_obj_set_pos(G->arc_fg, arc_x, 8);
        lv_obj_set_click(G->arc_fg, false);

        G->btn_dec = job_arrow_create(G->card, 4, "<");
        G->btn_inc = job_arrow_create(G->card, GAUGE_CARD_W - 4 - GAUGE_ARROW_W, ">");

        G->val = lv_label_create(G->card, NULL);
        lv_label_set_style(G->val, LV_LABEL_STYLE_MAIN, &job_val_style);
        lv_label_set_text(G->val, "100%");
        lv_obj_align(G->val, G->arc_fg, LV_ALIGN_CENTER, 0, 0);

        snprintf(job_cap_str[g], sizeof(job_cap_str[g]), "%s  %s", letters[g], names[g]);
        G->cap = lv_label_create(G->card, NULL);
        lv_label_set_style(G->cap, LV_LABEL_STYLE_MAIN, &job_cap_style);
        lv_label_set_text(G->cap, job_cap_str[g]);
        lv_obj_align(G->cap, G->card, LV_ALIGN_IN_BOTTOM_MID, 0, -6);

        G->shown = -1;
        job_gauge_draw(g, job_gauge_get(g));
    }
}

static lv_obj_t* job_info_label(lv_coord_t x, lv_coord_t y) {
    lv_obj_t* l = lv_label_create(mks_global.mks_src, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &job_info_style);
    lv_label_set_text(l, "");
    lv_obj_set_pos(l, x, y);
    return l;
}

static void job_info_create(void) {
    job_lbl_elapsed = job_info_label(14, JOB_STATS_Y);
    job_lbl_left    = job_info_label(14, JOB_STATS_Y + 22);
    job_lbl_feed    = job_info_label(JOB_STATS_COL2_X, JOB_STATS_Y);
    job_lbl_power   = job_info_label(JOB_STATS_COL2_X, JOB_STATS_Y + 22);
}

// Icono y texto del boton Pausar/Reanudar segun el estado real (tambien lo corrige si la
// pausa se hizo desde la WebUI u otro cliente).
static void job_pause_button_update(bool paused) {
    lv_obj_t* btn = print_src.print_imgbtn_suspend;
    lv_obj_t* lab = print_src.print_Label_p_suspend;
    if (btn == NULL || lab == NULL) return;
    lv_imgbtn_set_src(btn, LV_BTN_STATE_PR, paused ? &png_pause_pre : &png_start_pre);
    lv_imgbtn_set_src(btn, LV_BTN_STATE_REL, paused ? &png_pause : &png_start);
    lv_label_set_static_text(lab, paused ? mc_language.resume : mc_language.pause);
    lv_obj_align(lab, btn, LV_ALIGN_IN_RIGHT_MID, -20, 0);  // "Reanudar" es mas largo que "Pausar"
}

static void job_hms(char* out, size_t n, uint32_t s) {
    snprintf(out, n, "%02u:%02u:%02u", (unsigned)(s / 3600), (unsigned)((s / 60) % 60), (unsigned)(s % 60));
}

// Tiempo transcurrido y restante (estimado por el avance de la SD), y avance y potencia
// reales del momento (los de la linea FS: del informe de estado).
static void job_info_update(void) {
    uint32_t el = job_t0_ms ? (millis() - job_t0_ms) / 1000 : 0;
    float    p  = sd_report_perc_complete();
    char     t[12], r[12];

    job_hms(t, sizeof(t), el);
    if (p >= 2.0f && p < 100.0f) job_hms(r, sizeof(r), (uint32_t)(el * (100.0f - p) / p));
    else strcpy(r, "--:--:--");

    float pw = (rpm_max->get() > 0) ? 100.0f * (float)sys.spindle_speed / rpm_max->get() : 0.0f;

    snprintf(job_elapsed_str, sizeof(job_elapsed_str), "%s: %s", mc_language.job_elapsed, t);
    snprintf(job_left_str, sizeof(job_left_str), "%s: %s", mc_language.job_left, r);
    snprintf(job_feed_str, sizeof(job_feed_str), "%s: %d mm/min", mc_language.job_feed_real, (int)st_get_realtime_rate());
    snprintf(job_power_str, sizeof(job_power_str), "%s: %d%%", mc_language.job_power_real, (int)(pw + 0.5f));
    lv_label_set_text(job_lbl_elapsed, job_elapsed_str);
    lv_label_set_text(job_lbl_left, job_left_str);
    lv_label_set_text(job_lbl_feed, job_feed_str);
    lv_label_set_text(job_lbl_power, job_power_str);
}

void mks_draw_print(void) {

    char print_file_name[128];
    
    print_setting.cur_spindle_pwr = sys_rt_s_override;
    print_setting.cur_spindle_speed = sys_rt_f_override;
    print_setting.cur_spindle_rapid = sys_rt_r_override;

    // mks fix
    print_setting.carve_staus = CAVRE_START;

    if (job_t0_ms == 0) job_t0_ms = millis();  // inicio del trabajo (para el tiempo transcurrido)
    job_styles_init();

    memcpy(print_file_name, file_print_send, sizeof(file_print_send));
    if(print_file_name[0] == '/') print_file_name[0] = ' ';

    mks_pwr_ctrl.pwr_len = PWR_1_PERSEN;
    mks_speed_ctrl.speed_len = SPEED_1_PERSEN;

    lv_style_copy(&print_src.print_file_name_style, &lv_style_plain_color);
    print_src.print_file_name_style.text.font = mc_font();

    /* 进度条背景样式 */
    lv_style_copy(&print_src.print_bar_bg_style, &lv_style_plain_color);
    print_src.print_bar_bg_style.body.main_color = LV_COLOR_MAKE(0x3F,0x46,0x66);
    print_src.print_bar_bg_style.body.grad_color = LV_COLOR_MAKE(0x3F,0x46,0x66);
    print_src.print_bar_bg_style.body.radius = 5;

    /* 进度条显示样式 */
    lv_style_copy(&print_src.print_bar_indic_style,&lv_style_plain_color);
    print_src.print_bar_indic_style.body.main_color = COL_EMERALD;
    print_src.print_bar_indic_style.body.grad_color = COL_EMERALD;
    print_src.print_bar_indic_style.body.radius = 5;
    print_src.print_bar_indic_style.body.padding.left = 0;//让指示器跟背景边框之间没有距离
    print_src.print_bar_indic_style.body.padding.top = 0;
    print_src.print_bar_indic_style.body.padding.right = 0;
    print_src.print_bar_indic_style.body.padding.bottom = 0;

    print_src.print_imgbtn_suspend  = lv_imgbtn_creat_n_mks(mks_global.mks_src,  print_src.print_imgbtn_suspend, &png_start_pre, &png_start, 8, 250 ,event_handler_suspend);
    print_src.print_imgbtn_stop     = lv_imgbtn_creat_n_mks(mks_global.mks_src,  print_src.print_imgbtn_stop, &png_stop_pre, &png_stop, 165, 250 ,event_handler_stop);
    print_src.print_imgbtn_adj      = lv_imgbtn_creat_n_mks(mks_global.mks_src,  print_src.print_imgbtn_adj, &png_adj_pre, &png_adj, 322, 250, event_handler_adj);

    print_src.print_bar_print = mks_lv_bar_set(mks_global.mks_src, print_src.print_bar_print, JOB_BAR_W, JOB_BAR_H, JOB_BAR_X, JOB_BAR_Y, 0);

    lv_bar_set_style(print_src.print_bar_print, LV_BAR_STYLE_BG , &print_src.print_bar_bg_style);
    lv_bar_set_style(print_src.print_bar_print, LV_BAR_STYLE_INDIC , &print_src.print_bar_indic_style);

    print_src.print_Label_p_suspend = label_for_imgbtn_name_mid(mks_global.mks_src, print_src.print_Label_p_suspend, print_src.print_imgbtn_suspend ,-35 ,0 ,mc_language.pause);
    job_pause_button_update(sys.state == State::Hold);
    print_src.print_Label_p_stop = label_for_imgbtn_name_mid(mks_global.mks_src, print_src.print_Label_p_stop, print_src.print_imgbtn_stop ,-40 ,0 ,mc_language.stop);
    print_src.print_Label_p_adj = label_for_imgbtn_name_mid(mks_global.mks_src, print_src.print_Label_p_adj, print_src.print_imgbtn_adj ,-20 ,0 ,mc_language.adjust);
    


    job_gauges_create();
    job_info_create();

    Label_print_file_name = label_for_text(mks_global.mks_src, Label_print_file_name, NULL, 12, 6, LV_ALIGN_IN_TOP_LEFT, print_file_name);
    lv_label_set_style(Label_print_file_name, LV_LABEL_STYLE_MAIN, &print_src.print_file_name_style);
    lv_label_set_long_mode(Label_print_file_name, LV_LABEL_LONG_DOT);
    lv_obj_set_width(Label_print_file_name, 340);

    print_src.print_bar_print_percen = label_for_text(mks_global.mks_src, print_src.print_bar_print_percen, NULL, -12, 6, LV_ALIGN_IN_TOP_RIGHT, "0%");

    mks_ui_page.mks_ui_page = MKS_UI_Pring;  //进入雕刻界面
	mks_ui_page.wait_count = DEFAULT_UI_COUNT;
}


static void event_btn_cancle(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        
        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        // lv_obj_set_click(print_src.print_imgbtn_pwr, true);
        // lv_obj_set_click(print_src.print_imgbtn_speed, true);
        lv_obj_del(print_src.print_stop_popup);
    }
}

static void event_btn_sure(lv_obj_t* obj, lv_event_t event) {
    uint16_t buf_cmd[]={0x18};
    if (event == LV_EVENT_RELEASED) {
        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        // lv_obj_set_click(print_src.print_imgbtn_pwr, true);
        // lv_obj_set_click(print_src.print_imgbtn_speed, true);
        closeFile();
        job_t0_ms = 0;
        mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
        mks_ui_page.wait_count = 1;
        mks_clear_print();
        MKS_GRBL_CMD_SEND("M3 S0\n");
        MKS_GRBL_CMD_SEND("G90X0Y0F800\n");
        MKS_GRBL_CMD_SEND(buf_cmd);
        mks_draw_ready();
    }
}

static void event_btn_printdon(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
        
        job_t0_ms = 0;
        lv_obj_del(print_src.print_finsh_popup);
        mks_clear_print();
        mks_draw_ready();
    }
}

// stop print popup
void mks_draw_print_popup(const char* text) {
    
    lv_obj_set_click(print_src.print_imgbtn_suspend, false);
    lv_obj_set_click(print_src.print_imgbtn_stop, false);
    lv_obj_set_click(print_src.print_imgbtn_adj, false);

    print_src.print_stop_popup = lv_obj_create(mks_global.mks_src, NULL);

    lv_obj_set_size(print_src.print_stop_popup, print_popup_size_x, print_popup_size_y);
    lv_obj_set_pos(print_src.print_stop_popup, print_popup_x, print_popup_y);

    lv_style_copy(&print_src.printf_popup_style, &lv_style_scr);
    print_src.printf_popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.text.color = LV_COLOR_BLACK;
    print_src.printf_popup_style.body.radius = 17;
    lv_obj_set_style(print_src.print_stop_popup, &print_src.printf_popup_style);

    lv_style_copy(&print_src.print_popup_btn_style, &lv_style_scr);
    print_src.print_popup_btn_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_popup_btn_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_popup_btn_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_popup_btn_style.text.color = LV_COLOR_WHITE;
    print_src.print_popup_btn_style.body.radius = 10; 

    btn_popup_sure = mks_lv_btn_set(print_src.print_stop_popup, btn_popup_sure, 100,40,20,130,event_btn_sure);
	lv_btn_set_style(btn_popup_sure, LV_BTN_STYLE_REL, &print_src.print_popup_btn_style);
    lv_btn_set_style(btn_popup_sure,LV_BTN_STYLE_PR,&print_src.print_popup_btn_style);

    label_for_btn_name(btn_popup_sure, print_src.print_Label_popup_sure, 30, 0, mc_language.yes);

	btn_popup_cancle = mks_lv_btn_set(print_src.print_stop_popup, btn_popup_cancle, 100,40,230,130,event_btn_cancle);
	lv_btn_set_style(btn_popup_cancle, LV_BTN_STYLE_REL, &print_src.print_popup_btn_style);
    lv_btn_set_style(btn_popup_cancle,LV_BTN_STYLE_PR,&print_src.print_popup_btn_style);

    label_for_btn_name(btn_popup_cancle, print_src.print_Label_popup_sure, 50, 0, mc_language.cancel);
    mks_lvgl_long_sroll_label_with_wight_set(print_src.print_stop_popup, print_src.print_Label_popup, 80, 60, text, 200);
}

void mks_draw_finsh_pupop(void) { 

    lv_obj_set_click(print_src.print_imgbtn_suspend, false);
    lv_obj_set_click(print_src.print_imgbtn_stop, false);
    lv_obj_set_click(print_src.print_imgbtn_adj, false);
    

    print_src.print_finsh_popup = lv_obj_create(mks_global.mks_src, NULL);

    lv_obj_set_size(print_src.print_finsh_popup, 350, 200);
    lv_obj_set_pos(print_src.print_finsh_popup, 80, 50);

    lv_style_copy(&print_src.printf_popup_style, &lv_style_scr);
    print_src.printf_popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.text.color = LV_COLOR_BLACK;
    print_src.printf_popup_style.body.radius = 17;
    lv_obj_set_style(print_src.print_finsh_popup, &print_src.printf_popup_style);

    lv_style_copy(&print_src.print_popup_btn_style, &lv_style_scr);
    print_src.print_popup_btn_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_popup_btn_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_popup_btn_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_popup_btn_style.text.color = LV_COLOR_WHITE;
    print_src.print_popup_btn_style.body.radius = 10; 

    btn_finsh_popup_sure = lv_btn_create(print_src.print_finsh_popup, NULL);
    lv_obj_set_size(btn_finsh_popup_sure,   100, 50);
    lv_obj_set_pos(btn_finsh_popup_sure,    120, 130);
    lv_obj_set_event_cb(btn_finsh_popup_sure, event_btn_printdon);
    lv_btn_set_style(btn_finsh_popup_sure, LV_BTN_STYLE_REL, &print_src.print_popup_btn_style);
    lv_btn_set_style(btn_finsh_popup_sure,LV_BTN_STYLE_PR,&print_src.print_popup_btn_style);

    label_for_btn_name(btn_finsh_popup_sure, print_src.print_Label_popup_sure, 0, 0, mc_language.yes);
    label_for_screen(print_src.print_finsh_popup, print_src.print_Label_popup, 0, -20, mc_language.dis_print_done);
}

char bar_percen_str[20];
void mks_print_bar_updata(void) {
    print_src.print_bar_print = mks_lv_bar_updata(print_src.print_bar_print, (uint16_t)sd_report_perc_complete());
    sprintf(bar_percen_str, "%d%%", (uint16_t)sd_report_perc_complete());
    print_src.print_bar_print_percen = mks_lv_label_updata(print_src.print_bar_print_percen, bar_percen_str);
    lv_obj_align(print_src.print_bar_print_percen, NULL, LV_ALIGN_IN_TOP_RIGHT, -12, 6);
}

/****************************************************************************************pwr_popup****************************************************************************************/

lv_obj_t *pwr_label_power;

char power_add_dec_buf[20];

static void event_pwr_setting_add(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        
        if(mks_pwr_ctrl.pwr_len == PWR_1_PERSEN) {
            print_setting.cur_spindle_pwr += SpindleSpeedOverride::FineIncrement;
            if(print_setting.cur_spindle_pwr > SpindleSpeedOverride::Max) {
                print_setting.cur_spindle_pwr = SpindleSpeedOverride::Max;    
            }
        }else if(mks_pwr_ctrl.pwr_len == PWR_10_PERSEN) {
            print_setting.cur_spindle_pwr += SpindleSpeedOverride::CoarseIncrement;
            if(print_setting.cur_spindle_pwr > SpindleSpeedOverride::Max) {
                print_setting.cur_spindle_pwr = SpindleSpeedOverride::Max;    
            }
        }
        sprintf(power_add_dec_buf, mc_language.power_fmt, print_setting.cur_spindle_pwr);
        lv_label_set_static_text(pwr_label_power, power_add_dec_buf);
    }
}

static void event_pwr_setting_dec(lv_obj_t* obj, lv_event_t event) {

    uint16_t temp;

    if (event == LV_EVENT_RELEASED) {

        if(mks_pwr_ctrl.pwr_len == PWR_1_PERSEN) {
            print_setting.cur_spindle_pwr -= SpindleSpeedOverride::FineIncrement;
            if(print_setting.cur_spindle_pwr < SpindleSpeedOverride::Min) {
                print_setting.cur_spindle_pwr = SpindleSpeedOverride::Min;
            }
        }else if(mks_pwr_ctrl.pwr_len == PWR_10_PERSEN) {
            print_setting.cur_spindle_pwr -= SpindleSpeedOverride::CoarseIncrement;
            if(print_setting.cur_spindle_pwr < SpindleSpeedOverride::Min) {
                print_setting.cur_spindle_pwr = SpindleSpeedOverride::Min;
            }
        }
        sprintf(power_add_dec_buf, mc_language.power_fmt, print_setting.cur_spindle_pwr);
        lv_label_set_static_text(pwr_label_power, power_add_dec_buf);
    }
}

static void event_btn_pwr_1mm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        if(mks_pwr_ctrl.pwr_len == PWR_10_PERSEN) {
            mks_pwr_ctrl.pwr_len = PWR_1_PERSEN;
            lv_label_set_text(print_src.print_label_1_mm, "#ffffff 1% #");
            lv_label_set_text(print_src.print_label_10_mm, "#000000 10% #");

            print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

            print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
            print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
            lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style); 
        }
    }
}

static void event_btn_pwr_10mm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        if(mks_pwr_ctrl.pwr_len == PWR_1_PERSEN) {
            mks_pwr_ctrl.pwr_len = PWR_10_PERSEN;
            lv_label_set_text(print_src.print_label_1_mm, "#000000 1% #");
            lv_label_set_text(print_src.print_label_10_mm, "#ffffff 10% #");

            print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

            print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
            print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
            lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style);
        }
    }
}

static void event_pwr_setting_confirm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        if(sys.state == State::Hold) {
            print_setting._need_to_start_write = true;
        }else{
            sys_rt_s_override = print_setting.cur_spindle_pwr;
        }

        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        lv_obj_del(print_src.print_pwr_speed_src);
    }
}

static void event_pwr_setting_return(lv_obj_t* obj, lv_event_t event) {

    if (event == LV_EVENT_RELEASED) {

        print_setting.cur_spindle_pwr = sys_rt_s_override;

        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        lv_obj_del(print_src.print_pwr_speed_src);
    }
}


void mks_print_pwr_set(void) { 

    char buf[20]; 

    lv_obj_set_click(print_src.print_imgbtn_suspend, false);
    lv_obj_set_click(print_src.print_imgbtn_stop, false);

#if defined(USR_RELASE)
    print_src.print_pwr_speed_src = lv_obj_create(mks_src, NULL);
#else
    print_src.print_pwr_speed_src = lv_obj_create(mks_global.mks_src, NULL);
#endif
    lv_obj_set_size(print_src.print_pwr_speed_src, 350, 200);
    lv_obj_set_pos(print_src.print_pwr_speed_src, 75, 50);

    lv_style_copy(&print_src.printf_popup_style, &lv_style_scr);
    print_src.printf_popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.text.color = LV_COLOR_BLACK;
    print_src.printf_popup_style.body.radius = 17;
    lv_obj_set_style(print_src.print_pwr_speed_src, &print_src.printf_popup_style);

    lv_style_copy(&print_src.print_mm_btn1_style, &lv_style_scr);
    print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn1_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_mm_btn1_style.text.color = LV_COLOR_WHITE;
    print_src.print_mm_btn1_style.body.radius = 10; 

    lv_style_copy(&print_src.print_mm_btn2_style, &lv_style_scr);
    print_src.print_mm_btn2_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn2_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_mm_btn2_style.text.color = LV_COLOR_WHITE;
    print_src.print_mm_btn2_style.body.radius = 10; 

    print_src.print_btn_1_mm = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_btn_1_mm, 100, 40, 60, 80, event_btn_pwr_1mm);
    print_src.print_btn_10_mm = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_btn_10_mm, 100, 40, 180, 80, event_btn_pwr_10mm);

    if(mks_pwr_ctrl.pwr_len == PWR_1_PERSEN) { 
        print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
        lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

        print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
        print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
        lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
        lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style); 
    }else {
        print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
        lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

        print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
        print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
        lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
        lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style); 
    }

    sprintf(buf, mc_language.power_fmt, sys_rt_s_override);

    pwr_label_power = label_for_screen(print_src.print_pwr_speed_src, pwr_label_power, 0, -60, buf);

    if(mks_pwr_ctrl.pwr_len == PWR_1_PERSEN) {
        print_src.print_label_1_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_1_mm, print_src.print_label_1_mm, 0, 0, "#ffffff 1%#", 50);
        print_src.print_label_10_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_10_mm, print_src.print_label_10_mm, 0, 0, "#000000 10% #", 50);
    }else{
        print_src.print_label_1_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_1_mm, print_src.print_label_1_mm, 0, 0, "#000000 1%#", 50);
        print_src.print_label_10_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_10_mm, print_src.print_label_10_mm, 0, 0, "#ffffff 10% #", 50);
    }
    
    print_src.print_sp_imgbtn_add = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_add, &add, &add, LV_ALIGN_IN_LEFT_MID, print_pwr_popup_add_btn_x,print_pwr_popup_add_btn_y, event_pwr_setting_add);
    print_src.print_sp_imgbtn_dec = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_dec, &reduce, &reduce, LV_ALIGN_IN_LEFT_MID, print_pwr_popup_add_btn_x+80,print_pwr_popup_add_btn_y, event_pwr_setting_dec);
    print_src.print_sp_btn_sure = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_dec, &confirm, &confirm, LV_ALIGN_IN_LEFT_MID, print_pwr_popup_add_btn_x+160,print_pwr_popup_add_btn_y, event_pwr_setting_confirm);

    print_src.print_sp_btn_return = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, 
                                                        print_src.print_sp_btn_return, 
                                                        &s_return, 
                                                        &s_return, 
                                                        LV_ALIGN_IN_LEFT_MID, 
                                                        print_pwr_popup_add_btn_x+240,
                                                        print_pwr_popup_add_btn_y, event_pwr_setting_return);
}


/****************************************************************************************speed_popup****************************************************************************************/
lv_obj_t *pwr_label_speed;
char speed_add_dec_buf[20];

static void event_speed_setting_add(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        
        if(mks_speed_ctrl.speed_len == SPEED_1_PERSEN) {
            print_setting.cur_spindle_speed += FeedOverride::FineIncrement;
            if(print_setting.cur_spindle_speed > FeedOverride::Max) {
                print_setting.cur_spindle_speed = FeedOverride::Max;
            }

        }else if(mks_speed_ctrl.speed_len == SPEED_10_PERSEN) {
            print_setting.cur_spindle_speed += FeedOverride::CoarseIncrement;
            if(print_setting.cur_spindle_speed > FeedOverride::Max) {
                print_setting.cur_spindle_speed = FeedOverride::Max;
            }
        }
        sprintf(speed_add_dec_buf, mc_language.speed_fmt, print_setting.cur_spindle_speed);
        lv_label_set_static_text(pwr_label_speed, speed_add_dec_buf);
    }
}

static void event_speed_setting_dec(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        
        if(mks_speed_ctrl.speed_len == SPEED_1_PERSEN) {
            print_setting.cur_spindle_speed -= FeedOverride::FineIncrement;
            if(print_setting.cur_spindle_speed < FeedOverride::Min) {
                print_setting.cur_spindle_speed = FeedOverride::Min;
            }
        }else if(mks_speed_ctrl.speed_len == SPEED_10_PERSEN) {
            print_setting.cur_spindle_speed -= FeedOverride::CoarseIncrement;
            if(print_setting.cur_spindle_speed < FeedOverride::Min) {
                print_setting.cur_spindle_speed = FeedOverride::Min;
            }
        }
        sprintf(speed_add_dec_buf, mc_language.speed_fmt, print_setting.cur_spindle_speed);
        lv_label_set_static_text(pwr_label_speed, speed_add_dec_buf);
    }
}

static void event_speed_setting_confirm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        sys_rt_f_override = print_setting.cur_spindle_speed;

        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        lv_obj_del(print_src.print_pwr_speed_src);
    }
}

static void event_speed_setting_return(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {

        print_setting.cur_spindle_speed = sys_rt_f_override;

        lv_obj_set_click(print_src.print_imgbtn_suspend, true);
        lv_obj_set_click(print_src.print_imgbtn_stop, true);
        lv_obj_set_click(print_src.print_imgbtn_adj, true);
        lv_obj_del(print_src.print_pwr_speed_src);
    }
}


static void event_btn_speed_1mm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        if(mks_speed_ctrl.speed_len == SPEED_10_PERSEN) {
            mks_speed_ctrl.speed_len = SPEED_1_PERSEN;

            lv_label_set_text(print_src.print_label_1_mm, "#ffffff 1% #");  //000000
            lv_label_set_text(print_src.print_label_10_mm, "#000000 10% #"); //ffffff
            
            print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

            print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
            print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
            lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style);
        }
    }
}

static void event_btn_speed_10mm(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        if(mks_speed_ctrl.speed_len == SPEED_1_PERSEN) {
            mks_speed_ctrl.speed_len = SPEED_10_PERSEN;
            lv_label_set_text(print_src.print_label_1_mm, "#000000 1% #");
            lv_label_set_text(print_src.print_label_10_mm, "#ffffff 10% #");

            print_src.print_mm_btn1_style.body.main_color = LV_COLOR_WHITE;
            print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_WHITE;
            lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

            print_src.print_mm_btn2_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
            lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style);
        }
    }
}

void mks_print_speed_set(void) { 

    char buf[20]; 

    lv_obj_set_click(print_src.print_imgbtn_suspend, false);
    lv_obj_set_click(print_src.print_imgbtn_stop, false);
    lv_obj_set_click(print_src.print_imgbtn_adj, false);

    print_src.print_pwr_speed_src = lv_obj_create(mks_global.mks_src, NULL);

    lv_obj_set_size(print_src.print_pwr_speed_src, 350, 200);
    lv_obj_set_pos(print_src.print_pwr_speed_src, 80, 50);

    lv_style_copy(&print_src.printf_popup_style, &lv_style_scr);
    print_src.printf_popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5);
    print_src.printf_popup_style.text.color = LV_COLOR_BLACK;
    print_src.printf_popup_style.body.radius = 17;
    lv_obj_set_style(print_src.print_pwr_speed_src, &print_src.printf_popup_style);

    lv_style_copy(&print_src.print_mm_btn1_style, &lv_style_scr);
    print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn1_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_mm_btn1_style.text.color = LV_COLOR_WHITE;
    print_src.print_mm_btn1_style.body.radius = 10; 

    lv_style_copy(&print_src.print_mm_btn2_style, &lv_style_scr);
    print_src.print_mm_btn2_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    print_src.print_mm_btn2_style.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    print_src.print_mm_btn2_style.text.color = LV_COLOR_WHITE;
    print_src.print_mm_btn2_style.body.radius = 10; 

    print_src.print_btn_1_mm = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_btn_1_mm, 100, 40, 60, 80, event_btn_speed_1mm);
    print_src.print_btn_10_mm = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_btn_10_mm, 100, 40, 180, 80, event_btn_speed_10mm);

    if(mks_speed_ctrl.speed_len == SPEED_1_PERSEN) {
        print_src.print_mm_btn1_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
        lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);
        
        print_src.print_mm_btn2_style.body.main_color = LV_COLOR_WHITE;
        print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_WHITE;
        lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
        lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn2_style);

    }else if(mks_speed_ctrl.speed_len == SPEED_10_PERSEN){
        print_src.print_mm_btn1_style.body.main_color = LV_COLOR_WHITE;
        print_src.print_mm_btn1_style.body.grad_color = LV_COLOR_WHITE;
        lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
        lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_mm_btn1_style);

         print_src.print_mm_btn2_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
         print_src.print_mm_btn2_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, & print_src.print_mm_btn2_style);
        lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,& print_src.print_mm_btn2_style);
    }
    else {
        print_src.print_popup_btn_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        print_src.print_popup_btn_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
        lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_popup_btn_style);
        lv_btn_set_style(print_src.print_btn_1_mm,LV_BTN_STYLE_PR,&print_src.print_popup_btn_style);
        
        print_src.print_popup_btn_style.body.main_color = LV_COLOR_WHITE;
        print_src.print_popup_btn_style.body.grad_color = LV_COLOR_WHITE;
        lv_btn_set_style(print_src.print_btn_10_mm, LV_BTN_STYLE_REL, &print_src.print_popup_btn_style);
        lv_btn_set_style(print_src.print_btn_10_mm,LV_BTN_STYLE_PR,&print_src.print_popup_btn_style);
    }

    sprintf(buf, mc_language.speed_fmt, sys_rt_f_override);
    // pwr_label_speed = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_pwr_speed_src, pwr_label_speed, 20, 50, buf, 100); 
    pwr_label_speed = label_for_screen(print_src.print_pwr_speed_src, pwr_label_speed, 0, -60, buf);

    if(mks_speed_ctrl.speed_len == SPEED_1_PERSEN) {
        print_src.print_label_1_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_1_mm, print_src.print_label_1_mm, 0, 0, "#ffffff 1%#", 50);
        print_src.print_label_10_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_10_mm, print_src.print_label_10_mm, 0, 0, "#000000 10% #", 50);

    }else if(mks_speed_ctrl.speed_len == SPEED_10_PERSEN) {
        print_src.print_label_1_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_1_mm, print_src.print_label_1_mm, 0, 0, "#000000 1%#", 50);
        print_src.print_label_10_mm = mks_lvgl_long_sroll_label_with_wight_set_center(print_src.print_btn_10_mm, print_src.print_label_10_mm, 0, 0, "#ffffff 10% #", 50);
    }

    print_src.print_sp_imgbtn_add = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_add, &add, &add, LV_ALIGN_IN_LEFT_MID, print_pwr_popup_add_btn_x,print_pwr_popup_add_btn_y, event_speed_setting_add);
    print_src.print_sp_imgbtn_dec = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_dec, &reduce, &reduce, LV_ALIGN_IN_LEFT_MID, print_pwr_popup_add_btn_x+80,print_pwr_popup_add_btn_y, event_speed_setting_dec);
    print_src.print_sp_btn_sure = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, 
                                                        print_src.print_sp_btn_sure, 
                                                        &confirm, 
                                                        &confirm, 
                                                        LV_ALIGN_IN_LEFT_MID, 
                                                        print_pwr_popup_add_btn_x+160,
                                                        print_pwr_popup_add_btn_y, event_speed_setting_confirm);
    
    print_src.print_sp_btn_return = lv_imgbtn_creat_mks(print_src.print_pwr_speed_src, 
                                                        print_src.print_sp_btn_return, 
                                                        &s_return, 
                                                        &s_return, 
                                                        LV_ALIGN_IN_LEFT_MID, 
                                                        print_pwr_popup_add_btn_x+240,
                                                        print_pwr_popup_add_btn_y, event_speed_setting_return);
}


lv_obj_t *label_feed_rate, *label_spindle_speed, *label_rapid_speed;
lv_obj_t *label_back, *label_confirm, *label_add, *label_dec, *label_persen;
lv_obj_t *img_back, *img_confirm, *img_add, *img_dec;
uint8_t sp_select = 0; // default is 0, mean is select feed rate;
uint8_t sp_step = 1;    //default is  1, can select 10, 20, if sp_select = 2, only can select 25

char persen_dis_str[10];

char feed_rate_dis_str[48];
char spindle_speed_dis_str[48];
char rapid_dis_str[48];

// panel de ajuste: barra de cada fila y estilos de Aumentar / Reducir
static lv_obj_t*  adj_bar[3];  // 0 = husillo (S), 1 = avance (F), 2 = rapida (R)
static lv_style_t adj_bar_bg_style, adj_bar_ind_style[3];
static lv_style_t adj_add_rel_style, adj_add_pr_style, adj_dec_rel_style, adj_dec_pr_style;

static void adj_bars_refresh(void) {
    lv_bar_set_value(adj_bar[0], range_pct(print_setting.cur_spindle_pwr, SpindleSpeedOverride::Min, SpindleSpeedOverride::Max), LV_ANIM_OFF);
    lv_bar_set_value(adj_bar[1], range_pct(print_setting.cur_spindle_speed, FeedOverride::Min, FeedOverride::Max), LV_ANIM_OFF);
    lv_bar_set_value(adj_bar[2], range_pct(print_setting.cur_spindle_rapid, RapidOverride::Low, RapidOverride::Default), LV_ANIM_OFF);
}
enum {
    ID_SP_FEED_RATE,
    ID_SP_SPINDLE_SPEED,
    ID_SP_RAPID_SPEED,
    ID_SP_BACK,
    ID_SP_CONFIRM,
    ID_SP_ADD,
    ID_SP_DEC,
    ID_SP_PERSEM,
};

uint8_t get_sp_event_id(lv_obj_t* obj) {

    if(obj == print_src.print_sp_imgbtn_add) return ID_SP_ADD;
    else if(obj == print_src.print_sp_imgbtn_dec) return ID_SP_DEC;
    else if(obj == print_src.print_btn_1_mm) return ID_SP_PERSEM;
    else if(obj == print_src.print_sp_btn_sure) return ID_SP_CONFIRM;
    else if(obj == print_src.print_sp_btn_return) return ID_SP_BACK;
    else if(obj == print_src.print_imgbtn_pwr) return ID_SP_FEED_RATE;
    else if(obj == print_src.print_imgbtn_speed) return ID_SP_SPINDLE_SPEED;
    else if(obj == print_src.print_imgbtn_rapid) return ID_SP_RAPID_SPEED;
    return 255;  // sin coincidencia: sin accion
}


static void sp_img_set(uint8_t num, bool status) {

    switch(num) {
        case 0:
            if(status) lv_img_set_src(img_add, &png_sp_add);
            else lv_img_set_src(img_add, &png_sp_add_pre);
        break;

        case 1:
            if(status) lv_img_set_src(img_dec, &png_sp_dec);
            else lv_img_set_src(img_dec, &png_sp_dec_pre);
        break;

        case 2: 
            if(status) lv_img_set_src(img_confirm, &png_sp_comfirm);
            else lv_img_set_src(img_confirm, &png_sp_comfirm_pre);
        break;

        case 3: 
            if(status) lv_img_set_src(img_back, &png_sp_back);
            else lv_img_set_src(img_back, &png_sp_back_per);
        break;
    }
}

static void sp_list_select(uint8_t num) {

    switch(num) {
        case 0: 
            sp_select = 0;
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
        break;
        
        case 1: 
            sp_select = 1;
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
        break;

        case 2: 
            sp_select = 2;
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
            lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);
        break;
    }
}

static void sp_step_select(void) {

    if(sp_select != 2) {
        if(sp_step == 1) sp_step = 10;
        else if(sp_step == 10) sp_step = 20;
        else if(sp_step == 20) sp_step = 1;
        else if(sp_step == 25) sp_step = 1;
    }else {
        sp_step = 25;
    }
    sprintf(persen_dis_str, "%d%%", sp_step);
    lv_label_set_text(label_persen, persen_dis_str);
}

static void sp_add_dec(uint8_t num, uint8_t step, bool dir) {

    int step_get;

    if(dir == true) step_get = step;
    else step_get = -step;

    if(num == 0) {  

        print_setting.cur_spindle_pwr  += step_get;

        if(step > print_setting.cur_spindle_pwr) print_setting.cur_spindle_pwr = SpindleSpeedOverride::Min;

        if(print_setting.cur_spindle_pwr >= SpindleSpeedOverride::Max) {
            print_setting.cur_spindle_pwr = SpindleSpeedOverride::Max;
        } 
        else if(print_setting.cur_spindle_pwr <= SpindleSpeedOverride::Min) {
            print_setting.cur_spindle_pwr = SpindleSpeedOverride::Min;
        }   

        snprintf(spindle_speed_dis_str, sizeof(spindle_speed_dis_str), mc_language.spindle_speed_fmt, print_setting.cur_spindle_pwr);
        lv_label_set_text(label_spindle_speed, spindle_speed_dis_str);
    }
    else if(num == 1) {

        print_setting.cur_spindle_speed  += step_get;

        if(step > print_setting.cur_spindle_speed) print_setting.cur_spindle_speed = FeedOverride::Min;

        if(print_setting.cur_spindle_speed >= FeedOverride::Max) {
            print_setting.cur_spindle_speed = FeedOverride::Max;
        } 
        else if(print_setting.cur_spindle_speed <= FeedOverride::Min) {
            print_setting.cur_spindle_speed = FeedOverride::Min;
        } 

        snprintf(feed_rate_dis_str, sizeof(feed_rate_dis_str), mc_language.feed_rate_fmt, print_setting.cur_spindle_speed);
        lv_label_set_text(label_feed_rate, feed_rate_dis_str);
    }
    else if(num == 2) {

        print_setting.cur_spindle_rapid  += step_get;

        if(step > print_setting.cur_spindle_rapid) print_setting.cur_spindle_rapid = RapidOverride::Low;

        if(print_setting.cur_spindle_rapid >= RapidOverride::Default) {
            print_setting.cur_spindle_rapid = RapidOverride::Default;
        } 
        else if(print_setting.cur_spindle_rapid <= RapidOverride::Low) {
            print_setting.cur_spindle_rapid = RapidOverride::Low;
        } 

        snprintf(rapid_dis_str, sizeof(rapid_dis_str), mc_language.rapid_fmt, print_setting.cur_spindle_rapid);
        lv_label_set_text(label_rapid_speed, rapid_dis_str);
    }
    adj_bars_refresh();
}

static void set_comfirm(uint8_t num) {
    grbl_sendf(CLIENT_SERIAL, "Set num:%d\n", num);
    if(num == 0) sys_rt_s_override = print_setting.cur_spindle_pwr;
    else if(num == 1) sys_rt_f_override = print_setting.cur_spindle_speed;
    else if(num == 2) sys_rt_r_override = print_setting.cur_spindle_rapid;
}



static void event_handler_sp(lv_obj_t* obj, lv_event_t event) {

    uint8_t id = get_sp_event_id(obj);

    if(event == LV_EVENT_PRESSED) { 

        switch (id) {
            case ID_SP_ADD: sp_img_set(0, false); break;
            case ID_SP_DEC: sp_img_set(1, false); break;
            case ID_SP_CONFIRM: sp_img_set(2, false); break;
            case ID_SP_BACK: sp_img_set(3, false); break;
        }
    }

    if((event == LV_EVENT_RELEASED) || (event == LV_EVENT_PRESS_LOST))  {

        switch (id) {
            case ID_SP_ADD:
                sp_img_set(0, true); 
                sp_add_dec(sp_select, sp_step, true);
            break;
            case ID_SP_DEC: 
                sp_img_set(1, true); 
                sp_add_dec(sp_select, sp_step, false);
            break;
            case ID_SP_CONFIRM: 
                sp_img_set(2, true); 
                set_comfirm(sp_select);
                sp_popup_del();
            break;
            case ID_SP_BACK: 
                sp_popup_del();
            break;

            case ID_SP_FEED_RATE: sp_list_select(0); if(sp_step == 25) sp_step_select(); break;
            case ID_SP_SPINDLE_SPEED: sp_list_select(1); if(sp_step == 25) sp_step_select();  break;
            case ID_SP_RAPID_SPEED: sp_list_select(2); sp_step_select(); break;

            case ID_SP_PERSEM: sp_step_select(); break;
        }
    }
}

void draw_adj_popup(void) {

    sp_select = 0;
    sp_step = 1;

    // Los indicadores de la pantalla de trabajo pueden haber cambiado los valores.
    print_setting.cur_spindle_pwr   = sys_rt_s_override;
    print_setting.cur_spindle_speed = sys_rt_f_override;
    print_setting.cur_spindle_rapid = sys_rt_r_override;

    sprintf(persen_dis_str, "%d%%", sp_step);

    // Panel oscuro de 440x228 con tres filas (husillo, avance, rapida) con su barra,
    // a la derecha Aumentar (verde) / Reducir (azul) / paso, y abajo Atras / Confirmar.
    print_src.print_pwr_speed_src = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(print_src.print_pwr_speed_src, 440, 228);
    lv_obj_set_pos(print_src.print_pwr_speed_src, 20, 40);

    lv_style_copy(&print_src.printf_popup_style, &lv_style_scr);
    print_src.printf_popup_style.body.main_color   = COL_CARD;
    print_src.printf_popup_style.body.grad_color   = COL_CARD;
    print_src.printf_popup_style.body.border.width = 2;
    print_src.printf_popup_style.body.border.color = COL_TRACK;
    print_src.printf_popup_style.text.color        = LV_COLOR_WHITE;
    print_src.printf_popup_style.body.radius       = 14;
    print_src.printf_popup_style.text.font         = mc_font();
    lv_obj_set_style(print_src.print_pwr_speed_src, &print_src.printf_popup_style);

    // fila sin seleccionar
    lv_style_copy(&print_src.print_mm_btn1_style, &lv_style_scr);
    print_src.print_mm_btn1_style.body.main_color   = COL_BTN;
    print_src.print_mm_btn1_style.body.grad_color   = COL_BTN;
    print_src.print_mm_btn1_style.body.opa          = LV_OPA_COVER;
    print_src.print_mm_btn1_style.body.border.width = 1;
    print_src.print_mm_btn1_style.body.border.color = COL_TRACK;
    print_src.print_mm_btn1_style.text.color        = LV_COLOR_WHITE;
    print_src.print_mm_btn1_style.body.radius       = 10;

    // fila seleccionada / boton pulsado
    lv_style_copy(&print_src.print_mm_btn2_style, &lv_style_scr);
    print_src.print_mm_btn2_style.body.main_color   = COL_TRACK;
    print_src.print_mm_btn2_style.body.grad_color   = COL_TRACK;
    print_src.print_mm_btn2_style.body.opa          = LV_OPA_COVER;
    print_src.print_mm_btn2_style.body.border.width = 2;
    print_src.print_mm_btn2_style.body.border.color = COL_EMERALD;
    print_src.print_mm_btn2_style.text.color        = LV_COLOR_WHITE;
    print_src.print_mm_btn2_style.body.radius       = 10;

    // Aumentar (verde) y Reducir (azul)
    lv_style_copy(&adj_add_rel_style, &print_src.print_mm_btn1_style);
    adj_add_rel_style.body.main_color   = LV_COLOR_MAKE(0x1F, 0xA8, 0x7C);
    adj_add_rel_style.body.grad_color   = LV_COLOR_MAKE(0x1F, 0xA8, 0x7C);
    adj_add_rel_style.body.border.width = 0;
    lv_style_copy(&adj_add_pr_style, &adj_add_rel_style);
    adj_add_pr_style.body.main_color = COL_EMERALD;
    adj_add_pr_style.body.grad_color = COL_EMERALD;
    lv_style_copy(&adj_dec_rel_style, &print_src.print_mm_btn1_style);
    adj_dec_rel_style.body.main_color   = LV_COLOR_MAKE(0x1E, 0x88, 0xC8);
    adj_dec_rel_style.body.grad_color   = LV_COLOR_MAKE(0x1E, 0x88, 0xC8);
    adj_dec_rel_style.body.border.width = 0;
    lv_style_copy(&adj_dec_pr_style, &adj_dec_rel_style);
    adj_dec_pr_style.body.main_color = COL_BLUE;
    adj_dec_pr_style.body.grad_color = COL_BLUE;

    // barras de las filas
    lv_style_copy(&adj_bar_bg_style, &lv_style_plain_color);
    adj_bar_bg_style.body.main_color = COL_TRACK;
    adj_bar_bg_style.body.grad_color = COL_TRACK;
    adj_bar_bg_style.body.radius     = 3;
    const lv_color_t bar_colors[3] = { COL_BLUE, COL_EMERALD, COL_CORAL };  // husillo, avance, rapida
    for (int i = 0; i < 3; i++) {
        lv_style_copy(&adj_bar_ind_style[i], &lv_style_plain_color);
        adj_bar_ind_style[i].body.main_color     = bar_colors[i];
        adj_bar_ind_style[i].body.grad_color     = bar_colors[i];
        adj_bar_ind_style[i].body.radius         = 3;
        adj_bar_ind_style[i].body.padding.left   = 0;
        adj_bar_ind_style[i].body.padding.top    = 0;
        adj_bar_ind_style[i].body.padding.right  = 0;
        adj_bar_ind_style[i].body.padding.bottom = 0;
    }

    print_src.print_sp_imgbtn_add = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_add, 134, 58, 296, 10, event_handler_sp);
    print_src.print_sp_imgbtn_dec = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_sp_imgbtn_dec, 134, 58, 296, 76, event_handler_sp);
    print_src.print_btn_1_mm = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_btn_1_mm, 134, 46, 296, 142, event_handler_sp);
    print_src.print_sp_btn_sure = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_sp_btn_sure, 130, 48, 150, 170, event_handler_sp);
    print_src.print_sp_btn_return = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_sp_btn_return, 130, 48, 10, 170, event_handler_sp);

    print_src.print_imgbtn_pwr = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_imgbtn_pwr, 270, 46, 10, 10, event_handler_sp);
    print_src.print_imgbtn_speed = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_imgbtn_speed, 270, 46, 10, 62, event_handler_sp);
    print_src.print_imgbtn_rapid = mks_lv_btn_set(print_src.print_pwr_speed_src, print_src.print_imgbtn_rapid, 270, 46, 10, 114, event_handler_sp);

    snprintf(feed_rate_dis_str, sizeof(feed_rate_dis_str), mc_language.feed_rate_fmt, print_setting.cur_spindle_speed);
    snprintf(spindle_speed_dis_str, sizeof(spindle_speed_dis_str), mc_language.spindle_speed_fmt, print_setting.cur_spindle_pwr);
    snprintf(rapid_dis_str, sizeof(rapid_dis_str), mc_language.rapid_fmt, print_setting.cur_spindle_rapid);

    // texto de cada fila arriba y su barra debajo (y = fila + 34)
    label_feed_rate = label_for_text(print_src.print_pwr_speed_src, label_feed_rate, print_src.print_imgbtn_speed, 12, -5, LV_ALIGN_IN_LEFT_MID, feed_rate_dis_str);
    label_spindle_speed = label_for_text(print_src.print_pwr_speed_src, label_spindle_speed, print_src.print_imgbtn_pwr, 12, -5, LV_ALIGN_IN_LEFT_MID, spindle_speed_dis_str);
    label_rapid_speed = label_for_text(print_src.print_pwr_speed_src, label_rapid_speed, print_src.print_imgbtn_rapid, 12, -5, LV_ALIGN_IN_LEFT_MID, rapid_dis_str);

    const lv_coord_t bar_y[3] = { 10 + 34, 62 + 34, 114 + 34 };
    for (int i = 0; i < 3; i++) {
        adj_bar[i] = lv_bar_create(print_src.print_pwr_speed_src, NULL);
        lv_obj_set_size(adj_bar[i], 246, 6);
        lv_obj_set_pos(adj_bar[i], 22, bar_y[i]);
        lv_bar_set_style(adj_bar[i], LV_BAR_STYLE_BG, &adj_bar_bg_style);
        lv_bar_set_style(adj_bar[i], LV_BAR_STYLE_INDIC, &adj_bar_ind_style[i]);
        lv_obj_set_click(adj_bar[i], false);
    }
    adj_bars_refresh();

    // etiquetas: a la derecha del icono (28 px) dentro de cada boton
    label_for_text(print_src.print_pwr_speed_src, label_back, print_src.print_sp_btn_return, 14, 0, LV_ALIGN_CENTER, mc_language.back);
    label_for_text(print_src.print_pwr_speed_src, label_confirm, print_src.print_sp_btn_sure, 14, 0, LV_ALIGN_CENTER, mc_language.confirm);
    label_for_text(print_src.print_pwr_speed_src, label_add, print_src.print_sp_imgbtn_add, 14, 0, LV_ALIGN_CENTER, mc_language.add);
    label_for_text(print_src.print_pwr_speed_src, label_dec, print_src.print_sp_imgbtn_dec, 14, 0, LV_ALIGN_CENTER, mc_language.reduce);
    label_persen = label_for_text(print_src.print_pwr_speed_src, label_persen, print_src.print_btn_1_mm, 0, 0, LV_ALIGN_CENTER, persen_dis_str);

    img_add     = mks_lvgl_img_set_algin(print_src.print_pwr_speed_src, img_add, &png_sp_add, LV_ALIGN_IN_TOP_LEFT, 296 + 10, 10 + 15);
    img_dec     = mks_lvgl_img_set_algin(print_src.print_pwr_speed_src, img_dec, &png_sp_dec, LV_ALIGN_IN_TOP_LEFT, 296 + 10, 76 + 14);
    img_confirm = mks_lvgl_img_set_algin(print_src.print_pwr_speed_src, img_confirm, &png_sp_comfirm, LV_ALIGN_IN_TOP_LEFT, 150 + 10, 170 + 10);
    img_back    = mks_lvgl_img_set_algin(print_src.print_pwr_speed_src, img_back, &png_sp_back, LV_ALIGN_IN_TOP_LEFT, 10 + 10, 170 + 10);

    lv_btn_set_style(print_src.print_sp_imgbtn_add, LV_BTN_STYLE_REL, &adj_add_rel_style);
    lv_btn_set_style(print_src.print_sp_imgbtn_add, LV_BTN_STYLE_PR, &adj_add_pr_style);

    lv_btn_set_style(print_src.print_sp_imgbtn_dec, LV_BTN_STYLE_REL, &adj_dec_rel_style);
    lv_btn_set_style(print_src.print_sp_imgbtn_dec, LV_BTN_STYLE_PR, &adj_dec_pr_style);

    lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
    lv_btn_set_style(print_src.print_btn_1_mm, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);

    lv_btn_set_style(print_src.print_sp_btn_sure, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
    lv_btn_set_style(print_src.print_sp_btn_sure, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);

    lv_btn_set_style(print_src.print_sp_btn_return, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
    lv_btn_set_style(print_src.print_sp_btn_return, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);

    // fila 0 (husillo) seleccionada al abrir
    lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_REL, &print_src.print_mm_btn2_style);
    lv_btn_set_style(print_src.print_imgbtn_pwr, LV_BTN_STYLE_PR, &print_src.print_mm_btn2_style);

    lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
    lv_btn_set_style(print_src.print_imgbtn_speed, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);

    lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_REL, &print_src.print_mm_btn1_style);
    lv_btn_set_style(print_src.print_imgbtn_rapid, LV_BTN_STYLE_PR, &print_src.print_mm_btn1_style);
}

void sp_popup_del(void) {
    set_print_click(true);
    lv_obj_del(print_src.print_pwr_speed_src);
}


void set_print_click(bool status) {
    lv_obj_set_click(print_src.print_imgbtn_suspend, status);
    lv_obj_set_click(print_src.print_imgbtn_stop , status);
    lv_obj_set_click(print_src.print_imgbtn_adj , status);
}


char pl_info[128];
void mks_print_data_updata(void) {

    // Indicadores: reflejan tambien cambios hechos desde otro cliente (WebUI, panel de ajuste).
    for (int g = 0; g < G_COUNT; g++) {
        if (job_gauge[g].shown != job_gauge_get(g)) job_gauge_draw(g, job_gauge_get(g));
    }
    job_info_update();
    {
        static int8_t last_paused = -1;
        int8_t        paused      = (sys.state == State::Hold) ? 1 : 0;
        if (paused != last_paused) {
            last_paused = paused;
            job_pause_button_update(paused);
        }
    }

    if (SD_ready_next == false) {
        if (mks_grbl.is_mks_ts35_flag == true) {
            mks_print_bar_updata();
        }
    }
}

uint8_t get_print_speed(void) {
    
    return sys_rt_f_override;
}

uint8_t get_print_power(void) {

    return sys_rt_s_override;
}


void mks_clear_print(void) {
    lv_obj_clean(mks_global.mks_src);
}

void mks_del_obj(lv_obj_t *obj) { 
    lv_obj_del(obj);
}