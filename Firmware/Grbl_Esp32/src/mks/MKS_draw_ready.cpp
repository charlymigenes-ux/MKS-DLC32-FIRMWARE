#include "MKS_draw_ready.h"
#include "MKS_draw_language.h"   // mc_language: textos del LCD
#include <time.h>

MKS_PAGE_READY ready_src;
lv_style_t bkl_color;    // main


enum {
    ID_R_CONTRL,
    ID_R_SCULPTURE,
    ID_R_TOOL,
    ID_R_NONE,
};

static uint8_t get_event(lv_obj_t* obj) {
    if (obj == ready_src.ready_imgbtn_Control)         return ID_R_CONTRL;
    else if (obj == ready_src.ready_imgbtn_Sculpture)       return ID_R_SCULPTURE;
    else if (obj == ready_src.ready_imgbtn_Tool)            return ID_R_TOOL;
    else if (obj == ready_src.ready_imgbtn_wifi_status )    return ID_R_NONE;
    return ID_R_NONE;  // sin coincidencia: no limpiar la UI ni cambiar de pagina
}


static void event_handler_none(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {

	}
}

static void event_handler(lv_obj_t* obj, lv_event_t event) {

    uint8_t id = get_event(obj);

    if(event == LV_EVENT_PRESSED) {
        // ts35_beep_on();
    }

    mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
    if((event == LV_EVENT_RELEASED) || (event == LV_EVENT_PRESS_LOST))  {

        if((id != ID_R_NONE)) {
            mks_lv_clean_ui();
        }

        switch(id) {
            // case ID_R_ADJUST:        mks_draw_power(); break;
            case ID_R_CONTRL:         mks_draw_move();; break;
            case ID_R_SCULPTURE:
                file_popup_select_flag = false;
                mks_draw_craving();;
            break;
            case ID_R_TOOL:     mks_draw_tool();  break;
#ifdef ENABLE_WIFI
            // case ID_R_WIFI:
            //     mks_grbl.wifi_back_from = 0;
            //     mks_draw_wifi();
            // break;
#endif
            case ID_R_NONE: break;
        }
    }
}

lv_obj_t *logo;
uint32_t logo_count = 0;
void mks_draw_logo(void) {
    mks_ui_page.mks_ui_page = MKS_UI_Logo;
    mks_global.mks_src = lv_obj_create(NULL, NULL);
    mks_global.mks_src = lv_scr_act();
    lv_obj_set_style(mks_global.mks_src ,&mks_global.mks_src_style);
    logo = mks_lvgl_img_set(mks_global.mks_src, logo, &mks_logo, 0 ,0);
}

// =========================================================================================
// Pantalla principal (480x320): barra de Wifi arriba, dos tarjetas de coordenadas (Trabajo y
// Origen) y tres mosaicos grandes (Control, Tallado, Herramientas) que se pulsan enteros.
//   barra y=4 h=44 | tarjetas y=54 h=116 (x=6 y x=244, w=230) | mosaicos y=180 h=128 (w=150)
// =========================================================================================
#define RD_COL_CARD   LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define RD_COL_TRACK  LV_COLOR_MAKE(0x3F, 0x46, 0x66)

LV_IMG_DECLARE(png_tile_ctrl);    // glifos blancos con alfa: toman el degradado del mosaico
LV_IMG_DECLARE(png_tile_carve);
LV_IMG_DECLARE(png_tile_tool);

static lv_obj_t *rd_time_lbl, *rd_date_lbl, *rd_wday_lbl;
static lv_style_t rd_clock_style, rd_sep_style;
static lv_style_t rd_card_style, rd_text_style, rd_muted_style, rd_tile_style[3], rd_tile_pr_style[3];

static void rd_styles_init(void) {
    lv_style_copy(&rd_card_style, &lv_style_plain_color);
    rd_card_style.body.main_color   = RD_COL_CARD;
    rd_card_style.body.grad_color   = RD_COL_CARD;
    rd_card_style.body.radius       = 12;
    rd_card_style.body.border.width = 0;
    // degradados vertical (arriba -> abajo): Control cian-azul, Tallado esmeralda, Herramientas violeta
    static const uint32_t top[3] = { 0x3CD8F7, 0x2FD9A0, 0xB08CFB };
    static const uint32_t bot[3] = { 0x1565D8, 0x0A8F63, 0x6D28D9 };
    for(int i = 0; i < 3; i++) {
        lv_style_copy(&rd_tile_style[i], &rd_card_style);
        rd_tile_style[i].body.main_color = lv_color_hex(top[i]);
        rd_tile_style[i].body.grad_color = lv_color_hex(bot[i]);
        lv_style_copy(&rd_tile_pr_style[i], &rd_tile_style[i]);          // pulsado: mas oscuro
        rd_tile_pr_style[i].body.main_color = lv_color_hex(bot[i]);
        rd_tile_pr_style[i].body.grad_color = lv_color_hex(bot[i]);
    }
    lv_style_copy(&rd_text_style, &lv_style_plain);
    rd_text_style.text.font  = mc_font();
    rd_text_style.text.color = LV_COLOR_WHITE;
    lv_style_copy(&rd_sep_style, &rd_card_style);
    rd_sep_style.body.main_color = RD_COL_TRACK;
    rd_sep_style.body.grad_color = RD_COL_TRACK;
    rd_sep_style.body.radius = 0;
    lv_style_copy(&rd_clock_style, &rd_text_style);
    rd_clock_style.text.font = &lv_font_roboto_22;
    lv_style_copy(&rd_muted_style, &rd_text_style);
    rd_muted_style.text.color = LV_COLOR_MAKE(0x9A, 0xA3, 0xC0);
}

static lv_obj_t* rd_card(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* c = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(c, w, h);
    lv_obj_set_pos(c, x, y);
    lv_obj_set_style(c, &rd_card_style);
    return c;
}

static lv_obj_t* rd_label(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y, const lv_style_t* st) {
    lv_obj_t* l = lv_label_create(parent, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, (lv_style_t*)st);
    lv_label_set_text(l, text);
    lv_obj_set_pos(l, x, y);
    return l;
}

static lv_obj_t* rd_tile(int idx, lv_coord_t x, const lv_img_dsc_t* icon, const char* text, lv_obj_t** label_out) {
    lv_obj_t* b = lv_btn_create(mks_global.mks_src, NULL);
    lv_obj_set_size(b, 150, 128);
    lv_obj_set_pos(b, x, 180);
    lv_btn_set_style(b, LV_BTN_STYLE_REL, &rd_tile_style[idx]);
    lv_btn_set_style(b, LV_BTN_STYLE_PR, &rd_tile_pr_style[idx]);
    lv_obj_set_event_cb(b, event_handler);
    lv_cont_set_layout(b, LV_LAYOUT_OFF);   // sin la columna centrada del boton: se colocan a mano
    lv_obj_t* im = lv_img_create(b, NULL);  // la imagen no recibe toques: los recibe el mosaico
    lv_img_set_src(im, icon);
    lv_obj_align(im, b, LV_ALIGN_IN_TOP_MID, 0, 16);
    lv_obj_t* l = lv_label_create(b, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &rd_text_style);
    lv_label_set_text(l, text);
    lv_obj_align(l, b, LV_ALIGN_IN_BOTTOM_MID, 0, -16);
    *label_out = l;
    return b;
}

void mks_draw_ready(void) {

    mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
    rd_styles_init();

    // barra de Wifi
    lv_obj_t* bar = rd_card(6, 4, 468, 44);
    bool wifi_on = mks_get_wifi_status();
    // hora, fecha y dia (por NTP cuando hay Wifi; sin sincronizar se ve --:--) y Wifi a la derecha
    rd_time_lbl = rd_label(bar, "--:--", 12, 10, &rd_clock_style);
    lv_obj_t* sep = lv_obj_create(bar, NULL);
    lv_obj_set_size(sep, 2, 26);
    lv_obj_set_pos(sep, 86, 9);
    lv_obj_set_style(sep, &rd_sep_style);
    rd_date_lbl = rd_label(bar, "--/--/----", 100, 12, &rd_text_style);
    rd_wday_lbl = rd_label(bar, "", 200, 12, &rd_muted_style);
    ready_src.ready_imgbtn_wifi_status = lv_img_create(bar, NULL);
    lv_img_set_src(ready_src.ready_imgbtn_wifi_status, wifi_on ? &png_wifi_connect : &png_wifi_disconnect);
    lv_obj_set_pos(ready_src.ready_imgbtn_wifi_status, 262, 7);
    ready_src.ready_label_wifi_status = rd_label(bar, wifi_on ? mc_language.wifi_status_on : mc_language.wifi_status_off, 298, 12, &rd_text_style);

    // tarjetas de coordenadas: Trabajo (izquierda) y Origen (derecha)
    lv_obj_t* wc = rd_card(6, 54, 230, 116);
    lv_obj_t* mc = rd_card(244, 54, 230, 116);
    ready_src.ready_img_wpos = lv_img_create(wc, NULL);
    lv_img_set_src(ready_src.ready_img_wpos, &png_w_pos);
    lv_obj_set_pos(ready_src.ready_img_wpos, 10, 4);
    ready_src.ready_img_mpos = lv_img_create(mc, NULL);
    lv_img_set_src(ready_src.ready_img_mpos, &png_m_pos);
    lv_obj_set_pos(ready_src.ready_img_mpos, 10, 4);
    ready_src.ready_label_wpos = rd_label(wc, mc_language.Wpos, 46, 10, &rd_text_style);
    ready_src.ready_label_mpos = rd_label(mc, mc_language.Mpos, 46, 10, &rd_text_style);

    // coordenadas en una fila: letra del eje (gris) y valor debajo, tres columnas de 76 px
    static const char* axis[3] = { "X", "Y", "Z" };
    lv_obj_t** wv[3] = { &ready_src.ready_label_xpos, &ready_src.ready_label_ypos, &ready_src.ready_label_zpos };
    lv_obj_t** mv[3] = { &ready_src.ready_label_m_xpos, &ready_src.ready_label_m_ypos, &ready_src.ready_label_m_zpos };
    for(int i = 0; i < 3; i++) {
        for(int k = 0; k < 2; k++) {
            lv_obj_t* card = k == 0 ? wc : mc;
            lv_obj_t* l = rd_label(card, axis[i], 0, 54, &rd_muted_style);
            lv_label_set_long_mode(l, LV_LABEL_LONG_CROP);
            lv_label_set_align(l, LV_LABEL_ALIGN_CENTER);
            lv_obj_set_size(l, 76, 22);
            lv_obj_set_pos(l, 1 + i * 76, 54);
            lv_obj_t* v = rd_label(card, "0.0", 0, 80, k == 0 ? &rd_text_style : &rd_muted_style);
            lv_label_set_long_mode(v, LV_LABEL_LONG_CROP);
            lv_label_set_align(v, LV_LABEL_ALIGN_CENTER);
            lv_obj_set_size(v, 76, 22);
            lv_obj_set_pos(v, 1 + i * 76, 80);
            *(k == 0 ? wv[i] : mv[i]) = v;
        }
    }

    // mosaicos
    ready_src.ready_imgbtn_Control   = rd_tile(0, 6,   &png_tile_ctrl,   mc_language.control,   &ready_src.ready_label_Control);
    ready_src.ready_imgbtn_Sculpture = rd_tile(1, 165, &png_tile_carve, mc_language.sculpture, &ready_src.ready_label_Sculpture);
    ready_src.ready_imgbtn_Tool      = rd_tile(2, 324, &png_tile_tool,      mc_language.tool,      &ready_src.ready_label_Tool);

    mks_ui_page.mks_ui_page = MKS_UI_Ready;
}

char xpos_str[50] = "X:0.0";
char ypos_str[50] = "Y:0.0";
char zpos_str[50] = "Z:0.0";

char m_xpos_str[50] = "X:0.0";
char m_ypos_str[50] = "Y:0.0";
char m_zpos_str[50] = "Z:0.0";

char wifi_status_str[50];
char wifi_ip_str[100];

void mks_widi_show_ip(IPAddress ip, uint8_t p) {
    if(p) {
        strcat(wifi_ip_str, ip.toString().c_str());
    }else {
    }
}


// Hora local por NTP: la placa no tiene reloj propio. Zona fija CST6 (Mexico, sin horario de verano).
static void rd_clock_update(void) {
    static bool ntp_started = false;
    static char tbuf[8], dbuf[16];
    if(!ntp_started && mks_get_wifi_status()) {
        configTzTime("CST6", "pool.ntp.org", "time.google.com");
        ntp_started = true;
    }
    time_t now = time(NULL);
    if(rd_time_lbl == NULL || now < 1600000000) return;   // aun sin sincronizar
    struct tm t;
    localtime_r(&now, &t);
    static const char* wd_es[7] = { "Dom", "Lun", "Mar", "Mi\xc3\xa9", "Jue", "Vie", "S\xc3\xa1" "b" };
    static const char* wd_en[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", t.tm_hour, t.tm_min);
    snprintf(dbuf, sizeof(dbuf), "%02d/%02d/%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
    lv_label_set_text(rd_time_lbl, tbuf);
    lv_label_set_text(rd_date_lbl, dbuf);
    lv_label_set_text(rd_wday_lbl, mks_grbl.language == Espanol ? wd_es[t.tm_wday] : wd_en[t.tm_wday]);
}

void ready_data_updata(void) {

    static uint8_t wifi_ref_count = 0;
    static float mks_print_position[MAX_N_AXIS];
    float* print_position = system_get_mpos();

    sprintf(xpos_str, "%.1f", print_position[0]);
    sprintf(ypos_str, "%.1f", print_position[1]);
    sprintf(zpos_str, "%.1f", print_position[2]);

    lv_label_set_static_text(ready_src.ready_label_m_xpos, xpos_str);
    lv_label_set_static_text(ready_src.ready_label_m_ypos, ypos_str);
    lv_label_set_static_text(ready_src.ready_label_m_zpos, zpos_str);

    mpos_to_wpos(print_position);
    sprintf(m_xpos_str, "%.1f", print_position[0]);
    sprintf(m_ypos_str, "%.1f", print_position[1]);
    sprintf(m_zpos_str, "%.1f", print_position[2]);

    lv_label_set_static_text(ready_src.ready_label_xpos, m_xpos_str);
    lv_label_set_static_text(ready_src.ready_label_ypos, m_ypos_str);
    lv_label_set_static_text(ready_src.ready_label_zpos, m_zpos_str);

    rd_clock_update();

    #if defined(ENABLE_WIFI)
    if (mks_get_wifi_status() == false){
        ready_src.ready_label_wifi_status = mks_lv_label_updata(ready_src.ready_label_wifi_status, mc_language.wifi_status_off);
    }
    else {
        ready_src.ready_label_wifi_status = mks_lv_label_updata(ready_src.ready_label_wifi_status, mc_language.wifi_status_on);
    }
    #endif
}



