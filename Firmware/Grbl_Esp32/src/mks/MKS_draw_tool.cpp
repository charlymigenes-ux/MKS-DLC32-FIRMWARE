#include "MKS_draw_tool.h"
#include "MKS_draw_language.h"   // mc_language: textos del LCD
#include "../WebUI/WifiConfig.h"

lv_style_t about_src1_style;
lv_style_t btn_tool_style;
static lv_style_t style_line;

lv_obj_t *about_src1; 

// ===========================================================================================
// Pantalla de configuracion: barra superior con la ruta y el estado (wifi, temperatura), menu
// lateral a la izquierda (Atras, Herramienta, Wifi, Idioma, Beeper, Placa, Acerca de) y panel de
// detalle a la derecha. Wifi e Idioma abren sus pantallas; el resto cambia el panel.
// Disposicion en 480x320: barra y=6..30, menu x=6 w=104 y=38 (7 botones de 36 px),
// panel x=118 y=38 w=356 h=272.
// ===========================================================================================
#define COL_CARD     LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define COL_TRACK    LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define COL_SIDE     LV_COLOR_MAKE(0x18, 0x1B, 0x28)
#define COL_EMERALD  LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define COL_MUTED    LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)

enum { SEC_BACK, SEC_TOOL, SEC_WIFI, SEC_LANG, SEC_BEEP, SEC_BOARD, SEC_ABOUT, SEC_COUNT };

#define TOOL_SIDE_X     6
#define TOOL_SIDE_Y     38
#define TOOL_SIDE_W     104
#define TOOL_SIDE_H     36
#define TOOL_SIDE_GAP   3
#define TOOL_PANEL_X    118
#define TOOL_PANEL_Y    38
#define TOOL_PANEL_W    356
#define TOOL_PANEL_H    272
#define TOOL_ROW_X      14
#define TOOL_ROW_Y0     58
#define TOOL_ROW_W      328
#define TOOL_ROW_H      50
#define TOOL_ROW_STEP   54

static lv_style_t tool_side_rel_style, tool_side_sel_style, tool_side_pr_style, tool_panel_style;
static lv_style_t tool_row_rel_style, tool_row_pr_style, tool_row_info_style, tool_pill_style;
static lv_style_t tool_sym_ok_style, tool_sym_muted_style;
static lv_style_t tool_text_style, tool_dark_text_style, tool_muted_style, tool_pill_text_style, tool_title_style, tool_ok_style;

static lv_obj_t* tool_side_btn[SEC_COUNT];
static lv_obj_t* tool_side_lbl[SEC_COUNT];
static lv_obj_t* tool_panel = NULL;
static lv_obj_t* tool_crumb = NULL;
static lv_obj_t* tool_beep_val = NULL;
static int       tool_section = SEC_TOOL;
static int       tool_last_section = SEC_TOOL;   // se reabre al volver de las pantallas de Wifi
static lv_obj_t* tool_lang_row[4];
static int       tool_wifi_state = TOOL_WIFI_SUMMARY;
static int       tool_wifi_page = 0;
static lv_style_t tool_cjk_style;
static const lv_style_t* tool_row_title_style = NULL;   // estilo puntual del titulo de la siguiente fila
static char      tool_buf[SEC_COUNT][64];

// Estado mostrado del beep ($38). Se invierte al tocar y se envia el comando;
// asi dos toques seguidos no leen el valor viejo mientras $38 aun se procesa.
static bool tool_beep_on = false;

// Texto segun el idioma de la pantalla (espanol / resto en ingles)
static const char* T(const char* es, const char* en) { return mks_grbl.language == Espanol ? es : en; }
// Con chino: solo los titulos principales (los demas textos de esta pantalla quedan en ingles)
static const char* TC(const char* es, const char* en, const char* ch) {
    return mks_grbl.language == SimpleChinese ? ch : T(es, en);
}

static const char* tool_section_title(int sec) {
    switch (sec) {
        case SEC_BACK:  return mc_language.back;
        case SEC_TOOL:  return TC("Herramienta", "Tool", "\xe5\xb7\xa5\xe5\x85\xb7");              // 工具
        case SEC_WIFI:  return "Wifi";
        case SEC_LANG:  return mc_language.language;
        case SEC_BEEP:  return "Beeper";
        case SEC_BOARD: return TC("Placa", "Board", "\xe4\xb8\xbb\xe6\x9d\xbf");               // 主板
        default:        return T("Acerca de", "About");
    }
}

static void tool_go_back(void) {
    tool_last_section = SEC_TOOL;   // la proxima vez que se entre desde el menu, empieza en Herramienta
#if defined(ENABLE_WIFI)
    mks_wifi.wifi_scanf_status = wifi_none;   // detiene la maquina de estados del Wifi
#endif
    mks_clear_tool();
    mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
    mks_ui_page.wait_count = DEFAULT_UI_COUNT;
    mks_draw_ready();
}

static void tool_styles_init(void) {
    lv_style_copy(&tool_side_rel_style, &lv_style_plain_color);
    tool_side_rel_style.body.main_color   = COL_SIDE;
    tool_side_rel_style.body.grad_color   = COL_SIDE;
    tool_side_rel_style.body.radius       = 8;
    tool_side_rel_style.body.border.width = 0;
    lv_style_copy(&tool_side_sel_style, &tool_side_rel_style);   // seccion activa
    tool_side_sel_style.body.main_color = COL_EMERALD;
    tool_side_sel_style.body.grad_color = COL_EMERALD;
    lv_style_copy(&tool_side_pr_style, &tool_side_rel_style);    // pulsado
    tool_side_pr_style.body.main_color = COL_TRACK;
    tool_side_pr_style.body.grad_color = COL_TRACK;

    lv_style_copy(&tool_panel_style, &lv_style_plain_color);
    tool_panel_style.body.main_color   = COL_CARD;
    tool_panel_style.body.grad_color   = COL_CARD;
    tool_panel_style.body.radius       = 12;
    tool_panel_style.body.border.width = 1;
    tool_panel_style.body.border.color = COL_TRACK;

    lv_style_copy(&tool_row_rel_style, &lv_style_plain_color);
    tool_row_rel_style.body.main_color   = COL_SIDE;
    tool_row_rel_style.body.grad_color   = COL_SIDE;
    tool_row_rel_style.body.radius       = 10;
    tool_row_rel_style.body.border.width = 0;
    lv_style_copy(&tool_row_pr_style, &tool_row_rel_style);
    tool_row_pr_style.body.main_color = COL_TRACK;
    tool_row_pr_style.body.grad_color = COL_TRACK;
    lv_style_copy(&tool_row_info_style, &tool_row_rel_style);

    lv_style_copy(&tool_pill_style, &lv_style_plain_color);
    tool_pill_style.body.main_color   = LV_COLOR_MAKE(0x1E, 0x3A, 0x5F);
    tool_pill_style.body.grad_color   = LV_COLOR_MAKE(0x1E, 0x3A, 0x5F);
    tool_pill_style.body.radius       = 14;
    tool_pill_style.body.border.width = 0;

    lv_style_copy(&tool_text_style, &lv_style_plain);
    tool_text_style.text.font  = mc_font();   // con chino: la fuente que trae los glifos chinos
    tool_text_style.text.color = LV_COLOR_WHITE;
    lv_style_copy(&tool_dark_text_style, &tool_text_style);
    tool_dark_text_style.text.color = LV_COLOR_MAKE(0x08, 0x0C, 0x18);
    lv_style_copy(&tool_muted_style, &tool_text_style);
    tool_muted_style.text.color = COL_MUTED;
    lv_style_copy(&tool_pill_text_style, &tool_text_style);
    tool_pill_text_style.text.color = LV_COLOR_MAKE(0xCF, 0xE6, 0xFF);
    lv_style_copy(&tool_title_style, &tool_text_style);
    tool_title_style.text.font = (mks_grbl.language == SimpleChinese) ? mc_font() : &lv_font_roboto_22;
    lv_style_copy(&tool_cjk_style, &tool_text_style);   // titulo de la fila en chino: la fuente SimSun trae los glifos
    tool_cjk_style.text.font = &dlc32FontLatin;
    lv_style_copy(&tool_sym_ok_style, &tool_text_style);       // simbolos: Roboto (la fuente china no los trae)
    tool_sym_ok_style.text.font  = &roboto16Latin;
    tool_sym_ok_style.text.color = COL_EMERALD;
    lv_style_copy(&tool_sym_muted_style, &tool_sym_ok_style);
    tool_sym_muted_style.text.color = COL_MUTED;
    lv_style_copy(&tool_ok_style, &tool_text_style);
    tool_ok_style.text.color = COL_EMERALD;
}

// "Valor" dentro de una pastilla azul alineada a la derecha de la fila
static void tool_pill(lv_obj_t* row, const char* text, lv_obj_t** lbl_out) {
    lv_obj_t* pill = lv_obj_create(row, NULL);
    lv_obj_set_style(pill, &tool_pill_style);
    lv_obj_set_click(pill, false);
    lv_obj_t* l = lv_label_create(pill, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &tool_pill_text_style);
    lv_label_set_text(l, text);
    lv_obj_set_size(pill, lv_obj_get_width(l) + 24, 28);
    lv_obj_align(l, pill, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(pill, row, LV_ALIGN_IN_RIGHT_MID, -12, 0);
    if (lbl_out) *lbl_out = l;
}

static void tool_beep_refresh(void) {
    if (tool_beep_val == NULL) return;
    lv_obj_t* pill = lv_obj_get_parent(tool_beep_val);
    lv_label_set_text(tool_beep_val, tool_beep_on ? "ON" : "OFF");
    lv_obj_set_size(pill, lv_obj_get_width(tool_beep_val) + 24, 28);
    lv_obj_align(tool_beep_val, pill, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(pill, lv_obj_get_parent(pill), LV_ALIGN_IN_RIGHT_MID, -12, 0);
}

static void event_btn_tool_beep(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        tool_beep_on = !tool_beep_on;
        MKS_GRBL_CMD_SEND((tool_beep_on ? "$38=1\n" : "$38=0\n"));
        tool_beep_refresh();
    }
}

// Fila del panel: titulo, subtitulo gris y valor en pastilla. Con cb es tocable.
static lv_obj_t* tool_row(int idx, const char* title, const char* subtitle, const char* value, lv_event_cb_t cb, lv_obj_t** value_lbl) {
    lv_obj_t* row;
    if (cb != NULL) {
        row = lv_btn_create(tool_panel, NULL);
        lv_btn_set_style(row, LV_BTN_STYLE_REL, &tool_row_rel_style);
        lv_btn_set_style(row, LV_BTN_STYLE_PR, &tool_row_pr_style);
        lv_obj_set_event_cb(row, cb);
        lv_cont_set_layout(row, LV_LAYOUT_OFF);   // el boton apila y centra a sus hijos: aqui se colocan a mano
    } else {
        row = lv_obj_create(tool_panel, NULL);
        lv_obj_set_style(row, &tool_row_info_style);
    }
    lv_obj_set_size(row, TOOL_ROW_W, TOOL_ROW_H);
    lv_obj_set_pos(row, TOOL_ROW_X, TOOL_ROW_Y0 + idx * TOOL_ROW_STEP);

    lv_obj_t* t = lv_label_create(row, NULL);
    lv_label_set_style(t, LV_LABEL_STYLE_MAIN, tool_row_title_style ? tool_row_title_style : &tool_text_style);
    tool_row_title_style = NULL;
    lv_label_set_text(t, title);
    lv_obj_set_pos(t, 14, 5);

    lv_obj_t* st = lv_label_create(row, NULL);
    lv_label_set_style(st, LV_LABEL_STYLE_MAIN, &tool_muted_style);
    lv_label_set_text(st, subtitle);
    lv_obj_set_pos(st, 14, 26);

    tool_pill(row, value, value_lbl);
    return row;
}

static void tool_panel_head(const char* title, const char* subtitle) {
    lv_obj_t* t = lv_label_create(tool_panel, NULL);
    lv_label_set_style(t, LV_LABEL_STYLE_MAIN, &tool_title_style);
    lv_label_set_text(t, title);
    lv_obj_set_pos(t, 14, 8);
    lv_obj_t* s = lv_label_create(tool_panel, NULL);
    lv_label_set_style(s, LV_LABEL_STYLE_MAIN, &tool_muted_style);
    lv_label_set_text(s, subtitle);
    lv_obj_set_pos(s, 14, 36);
}

// Idioma dentro de la pantalla: 0 = chino, 1 = ingles, 2 = aleman, 3 = espanol (parametro $40).
// Igual que la antigua pantalla de idioma: cambia el idioma al instante y manda $40.
static char tool_lang_cmd[4][8] = { "$40=0\n", "$40=1\n", "$40=2\n", "$40=3\n" };

static int tool_language_index(void) {
    switch (mks_grbl.language) {
        case SimpleChinese: return 0;
        case English:       return 1;
        case Deutsch:       return 2;
        default:            return 3;
    }
}

static void tool_lang_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    static const GRBL_Language langs[4] = { SimpleChinese, English, Deutsch, Espanol };
    for (int i = 0; i < 4; i++) {
        if (obj != tool_lang_row[i]) continue;
        if (i == tool_language_index()) return;   // ya es el idioma activo
        mks_grbl.language = langs[i];
        mc_language_init();
        MKS_GRBL_CMD_SEND(tool_lang_cmd[i]);
        tool_last_section = SEC_LANG;
        mks_clear_tool();
        mks_draw_tool();                          // se redibuja con los textos del idioma nuevo
        return;
    }
}

static void tool_build_panel(int sec);

// Corta un nombre a max_cp caracteres (UTF-8 sin partir un caracter) y pone "..." si no cabe.
static void tool_fit(char* out, size_t n, const char* name, int max_cp) {
    size_t o = 0;
    int cp = 0;
    for (size_t i = 0; name[i] != '\0' && o + 1 < n; i++) {
        if (((uint8_t)name[i] & 0xC0) != 0x80) {   // inicio de un caracter
            if (cp == max_cp) {
                if (o + 4 < n) { out[o++] = '.'; out[o++] = '.'; out[o++] = '.'; }
                break;
            }
            cp++;
        }
        out[o++] = name[i];
    }
    out[o] = '\0';
}

#if defined(ENABLE_WIFI)
// ---- Pestana Wifi. La maquina de estados (escanear, lista, conectando, desconectando) sigue
// en MKS_FREERTOS_TASK y MKS_draw_wifi.cpp; aqui solo se dibuja cada estado dentro del panel. ----
#define NET_ROWS 5
extern uint8_t wifi_div(int32_t rssi);
static lv_obj_t* tool_net_btn[NET_ROWS];
static int       tool_net_idx[MKS_WIFI_NUM];
static int       tool_net_count = 0;
static lv_obj_t *tool_btn_scan, *tool_btn_up, *tool_btn_next, *tool_btn_disc;
static char      tool_net_pct[NET_ROWS][8];

static void tool_net_collect(void) {   // solo las redes con nombre (el escaneo rellena 16 huecos)
    tool_net_count = 0;
    for (int i = 0; i < MKS_WIFI_NUM; i++) {
        if (mks_wifi.wifi_name_str[i][0] != '\0') tool_net_idx[tool_net_count++] = i;
    }
}

static void tool_net_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    for (int s = 0; s < NET_ROWS; s++) {
        if (obj != tool_net_btn[s]) continue;
        int n = tool_wifi_page * NET_ROWS + s;
        if (n >= tool_net_count) return;
        int idx = tool_net_idx[n];
        wifi_src.wifi_send_num = idx + 1;      // igual que los antiguos manejadores de la lista
        mks_wifi.wifi_choose   = idx;
        draw_pos_wifi_popup(mc_language.wifi_pwd_prompt, mks_wifi.wifi_name_str[idx]);   // teclado de la contrasena
        return;
    }
}

static void tool_wifi_action_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    if (obj == tool_btn_scan) {
        mks_lv_clean_ui();
        mks_draw_wifi_scanf();
    } else if (obj == tool_btn_up) {
        if (tool_wifi_page > 0) { tool_wifi_page--; tool_build_panel(SEC_WIFI); }
    } else if (obj == tool_btn_next) {
        if ((tool_wifi_page + 1) * NET_ROWS < tool_net_count) { tool_wifi_page++; tool_build_panel(SEC_WIFI); }
    } else if (obj == tool_btn_disc) {
        mks_lv_clean_ui();
        mks_draw_wifi_disconnrcting();
        wifi_src.wifi_kb_flag = wifi_kb_send_wifi_disconnect;
    }
}

static lv_obj_t* tool_footer_btn(lv_coord_t x, const char* text) {
    lv_obj_t* b = lv_btn_create(tool_panel, NULL);
    lv_obj_set_size(b, 100, 32);
    lv_obj_set_pos(b, x, 232);
    lv_btn_set_style(b, LV_BTN_STYLE_REL, &tool_row_rel_style);
    lv_btn_set_style(b, LV_BTN_STYLE_PR, &tool_row_pr_style);
    lv_obj_set_event_cb(b, tool_wifi_action_event);
    lv_obj_t* l = lv_label_create(b, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &tool_text_style);
    lv_label_set_text(l, text);
    return b;
}

static void tool_wifi_note(const char* msg) {   // mensaje centrado (buscando, conectando...)
    lv_obj_t* l = lv_label_create(tool_panel, NULL);
    lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &tool_text_style);
    lv_label_set_text(l, msg);
    lv_obj_align(l, tool_panel, LV_ALIGN_CENTER, 0, 10);
}

static void tool_build_wifi(void) {
    tool_btn_scan = tool_btn_up = tool_btn_next = tool_btn_disc = NULL;
    for (int s = 0; s < NET_ROWS; s++) tool_net_btn[s] = NULL;

    switch (tool_wifi_state) {
        case TOOL_WIFI_SCANNING:
            tool_panel_head("Wifi", T("Buscando redes", "Scanning networks"));
            tool_wifi_note(mc_language.wifi_scanning);
            break;
        case TOOL_WIFI_CONNECTING:
            tool_panel_head("Wifi", T("Conectando", "Connecting"));
            tool_wifi_note(mc_language.wifi_connecting);
            break;
        case TOOL_WIFI_DISCONNECTING:
            tool_panel_head("Wifi", T("Desconectando", "Disconnecting"));
            tool_wifi_note(mc_language.wifi_disconnecting);
            break;
        case TOOL_WIFI_LIST: {
            tool_panel_head("Wifi", T("Elige una red", "Choose a network"));
            tool_net_collect();
            if (tool_net_count == 0) tool_wifi_page = 0;
            else if (tool_wifi_page * NET_ROWS >= tool_net_count) tool_wifi_page = (tool_net_count - 1) / NET_ROWS;
            for (int s = 0; s < NET_ROWS; s++) {
                int n = tool_wifi_page * NET_ROWS + s;
                if (n >= tool_net_count) break;
                int idx = tool_net_idx[n];

                lv_obj_t* b = lv_btn_create(tool_panel, NULL);
                lv_obj_set_size(b, TOOL_ROW_W, 30);
                lv_obj_set_pos(b, TOOL_ROW_X, 58 + s * 34);
                lv_btn_set_style(b, LV_BTN_STYLE_REL, &tool_row_rel_style);
                lv_btn_set_style(b, LV_BTN_STYLE_PR, &tool_row_pr_style);
                lv_obj_set_event_cb(b, tool_net_event);
                lv_cont_set_layout(b, LV_LAYOUT_OFF);
                tool_net_btn[s] = b;

                char nm[40];
                tool_fit(nm, sizeof(nm), mks_wifi.wifi_name_str[idx], 24);
                lv_obj_t* l = lv_label_create(b, NULL);
                lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &tool_text_style);
                lv_label_set_text(l, nm);
                lv_obj_align(l, b, LV_ALIGN_IN_LEFT_MID, 12, 0);

                snprintf(tool_net_pct[s], sizeof(tool_net_pct[s]), "%u%%", (unsigned)wifi_div(mks_wifi.wifi_rssi[idx]));
                lv_obj_t* v = lv_label_create(b, NULL);
                lv_label_set_style(v, LV_LABEL_STYLE_MAIN, &tool_ok_style);
                lv_label_set_text(v, tool_net_pct[s]);
                lv_obj_align(v, b, LV_ALIGN_IN_RIGHT_MID, -12, 0);
            }
            if (tool_net_count == 0) tool_wifi_note(T("No se encontraron redes", "No networks found"));
            tool_btn_scan = tool_footer_btn(14, mc_language.scanf);
            tool_btn_up   = tool_footer_btn(128, mc_language.up);
            tool_btn_next = tool_footer_btn(242, mc_language.next);
            break;
        }
        default: {   // TOOL_WIFI_SUMMARY: estado de la conexion actual
            tool_panel_head("Wifi", T("Conexión de la red", "Network connection"));
            bool on = mks_get_wifi_status();
            char ssid[40] = "--";
            if (on) tool_fit(ssid, sizeof(ssid), WiFi.SSID().c_str(), 18);
            snprintf(tool_buf[0], sizeof(tool_buf[0]), "%s", ssid);
            snprintf(tool_buf[1], sizeof(tool_buf[1]), "%s", on ? WiFi.localIP().toString().c_str() : T("Sin conexión", "Disconnected"));
            tool_row(0, T("Red", "Network"), tool_buf[1], tool_buf[0], NULL, NULL);
            snprintf(tool_buf[2], sizeof(tool_buf[2]), "%d%%", on ? (int)WebUI::wifi_config.getSignal(WiFi.RSSI()) : 0);
            tool_row(1, T("Señal", "Signal"), T("Intensidad de la señal", "Signal strength"), tool_buf[2], NULL, NULL);
            if (on) {
                tool_btn_disc = tool_row(2, T("Desconectar", "Disconnect"), T("Cerrar la conexión actual", "Close the current connection"), ">", tool_wifi_action_event, NULL);
            } else {
                tool_btn_scan = tool_row(2, T("Buscar redes", "Scan networks"), T("Redes disponibles", "Available networks"), ">", tool_wifi_action_event, NULL);
            }
            break;
        }
    }
}
#endif

static void tool_build_panel(int sec) {
    tool_beep_val = NULL;
    lv_obj_clean(tool_panel);

    if (sec == SEC_TOOL) {
        tool_panel_head(tool_section_title(sec), T("Información del sistema y del hardware", "System and hardware information"));

        const char* board = BOARD_NAME;
        if (strncmp(board, "Board:", 6) == 0) board += 6;
        tool_row(0, T("Placa", "Board"), T("Modelo de la placa", "Board model"), board, NULL, NULL);

        // FW_NAME = "V1.0(DLC32.8M.20261001)": la version en la pastilla y la compilacion debajo
        char ver[24] = "", build[40] = "";
        const char* open = strchr(FW_NAME, '(');
        if (open != NULL) {
            snprintf(ver, sizeof(ver), "%.*s", (int)(open - FW_NAME), FW_NAME);
            snprintf(build, sizeof(build), "%s", open + 1);
            char* close = strchr(build, ')');
            if (close) *close = '\0';
        } else {
            snprintf(ver, sizeof(ver), "%s", FW_NAME);
        }
        tool_row(1, "Firmware", build[0] ? build : T("Versión instalada", "Installed version"), ver, NULL, NULL);

        snprintf(tool_buf[0], sizeof(tool_buf[0]), "%uMHz | %.0f\xC2\xB0""C", (unsigned)ESP.getCpuFreqMHz(), temperatureRead());
        tool_row(2, "CPU", T("Reloj y temperatura", "Clock and temp."), tool_buf[0], NULL, NULL);
    }
    else if (sec == SEC_LANG) {
        tool_panel_head(tool_section_title(sec), T("Idioma de la pantalla ($40)", "Screen language ($40)"));
        static const char* native[4] = { "\xe4\xb8\xad\xe6\x96\x87", "English", "Deutsch", "Espa\xc3\xb1ol" };
        static const char* codes[4]  = { "$40=0", "$40=1", "$40=2", "$40=3" };
        int cur = tool_language_index();
        for (int i = 0; i < 4; i++) {
            if (i == 0) tool_row_title_style = &tool_cjk_style;
            tool_lang_row[i] = tool_row(i, native[i], codes[i], i == cur ? T("Activo", "Active") : T("Elegir", "Select"), tool_lang_event, NULL);
        }
    }
    else if (sec == SEC_WIFI) {
#if defined(ENABLE_WIFI)
        tool_build_wifi();
#else
        tool_panel_head("Wifi", T("No disponible en esta compilación", "Not available in this build"));
#endif
    }
    else if (sec == SEC_BEEP) {
        tool_panel_head("Beeper", T("Sonido al tocar la pantalla", "Sound when touching the screen"));
        tool_beep_on = beep_status->get();
        tool_row(0, "Beeper", T("Toca para activar o desactivar", "Tap to turn on or off"), tool_beep_on ? "ON" : "OFF", event_btn_tool_beep, &tool_beep_val);
    }
    else if (sec == SEC_BOARD) {
        tool_panel_head(tool_section_title(sec), T("Datos del microcontrolador", "Microcontroller details"));
        snprintf(tool_buf[0], sizeof(tool_buf[0]), "%s rev %u", ESP.getChipModel(), (unsigned)ESP.getChipRevision());
        tool_row(0, "Chip", T("Procesador", "Processor"), tool_buf[0], NULL, NULL);
        snprintf(tool_buf[1], sizeof(tool_buf[1]), "%u MB", (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
        tool_row(1, "Flash", T("Memoria del programa", "Program memory"), tool_buf[1], NULL, NULL);
        snprintf(tool_buf[2], sizeof(tool_buf[2]), "%04X", (unsigned)(uint16_t)(ESP.getEfuseMac() >> 32));
        tool_row(2, "ID", T("Identificador del chip", "Chip identifier"), tool_buf[2], NULL, NULL);
    }
    else {  // SEC_ABOUT
        tool_panel_head(tool_section_title(sec), "MKS DLC32 Firmware");
        tool_row(0, T("Licencia", "License"), T("Software libre", "Free software"), "GPL v3", NULL, NULL);
        tool_row(1, T("Basado en", "Based on"), "Grbl_ESP32 | LVGL", "MKS", NULL, NULL);
        lv_obj_t* url = lv_label_create(tool_panel, NULL);
        lv_label_set_style(url, LV_LABEL_STYLE_MAIN, &tool_muted_style);
        lv_label_set_long_mode(url, LV_LABEL_LONG_BREAK);
        lv_obj_set_width(url, TOOL_ROW_W);
        lv_label_set_text(url, "github.com/charlymigenes-ux/MKS-DLC32-FIRMWARE");
        lv_obj_set_pos(url, 14, TOOL_ROW_Y0 + 2 * TOOL_ROW_STEP + 4);
    }
}

static void tool_select(int sec) {
    tool_section = sec;
    tool_last_section = sec;
    for (int i = SEC_TOOL; i < SEC_COUNT; i++) {
        bool on = (i == sec);
        lv_btn_set_style(tool_side_btn[i], LV_BTN_STYLE_REL, on ? &tool_side_sel_style : &tool_side_rel_style);
        lv_btn_set_style(tool_side_btn[i], LV_BTN_STYLE_PR, on ? &tool_side_sel_style : &tool_side_pr_style);
        lv_label_set_style(tool_side_lbl[i], LV_LABEL_STYLE_MAIN, on ? &tool_dark_text_style : &tool_text_style);
    }
    snprintf(tool_buf[3], sizeof(tool_buf[3]), "%s > %s", TC("Configuración", "Settings", "\xe8\xae\xbe\xe7\xbd\xae"), tool_section_title(sec));
    lv_label_set_text(tool_crumb, tool_buf[3]);
    tool_build_panel(sec);
#if defined(ENABLE_WIFI)
    mks_ui_page.mks_ui_page = (sec == SEC_WIFI) ? MKS_UI_Wifi : MKS_UI_Tool;
#else
    mks_ui_page.mks_ui_page = MKS_UI_Tool;
#endif
}

static void tool_side_event(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_RELEASED) return;
    for (int i = 0; i < SEC_COUNT; i++) {
        if (obj != tool_side_btn[i]) continue;
        if (i == SEC_BACK) tool_go_back();
#if defined(ENABLE_WIFI)
        else if (i == SEC_WIFI) mks_draw_wifi();   // decide: resumen si hay conexion, o empieza a buscar redes
#endif
        else tool_select(i);
        return;
    }
}

void mks_draw_tool(void) {

    tool_styles_init();
    tool_beep_val = NULL;

    // barra superior: ruta a la izquierda; estado (wifi y temperatura) a la derecha
    tool_crumb = lv_label_create(mks_global.mks_src, NULL);
    lv_label_set_style(tool_crumb, LV_LABEL_STYLE_MAIN, &tool_text_style);
    lv_label_set_text(tool_crumb, "");
    lv_obj_set_pos(tool_crumb, 12, 8);

    snprintf(tool_buf[4], sizeof(tool_buf[4]), "%.0f\xC2\xB0""C", temperatureRead());
    lv_obj_t* temp = lv_label_create(mks_global.mks_src, NULL);
    lv_label_set_style(temp, LV_LABEL_STYLE_MAIN, &tool_text_style);
    lv_label_set_text(temp, tool_buf[4]);
    lv_obj_align(temp, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 8);

    lv_obj_t* wifi = lv_label_create(mks_global.mks_src, NULL);
    lv_label_set_style(wifi, LV_LABEL_STYLE_MAIN, mks_get_wifi_status() ? &tool_sym_ok_style : &tool_sym_muted_style);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    lv_obj_align(wifi, temp, LV_ALIGN_OUT_LEFT_MID, -14, 0);

    // menu lateral
    for (int i = 0; i < SEC_COUNT; i++) {
        lv_obj_t* b = lv_btn_create(mks_global.mks_src, NULL);
        lv_obj_set_size(b, TOOL_SIDE_W, TOOL_SIDE_H);
        lv_obj_set_pos(b, TOOL_SIDE_X, TOOL_SIDE_Y + i * (TOOL_SIDE_H + TOOL_SIDE_GAP));
        lv_btn_set_style(b, LV_BTN_STYLE_REL, &tool_side_rel_style);
        lv_btn_set_style(b, LV_BTN_STYLE_PR, &tool_side_pr_style);
        lv_obj_set_event_cb(b, tool_side_event);
        lv_cont_set_layout(b, LV_LAYOUT_OFF);
        tool_side_btn[i] = b;

        lv_obj_t* l = lv_label_create(b, NULL);
        lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &tool_text_style);
        lv_label_set_text(l, tool_section_title(i));
        lv_obj_align(l, b, LV_ALIGN_IN_LEFT_MID, 12, 0);
        tool_side_lbl[i] = l;
    }

    tool_panel = lv_obj_create(mks_global.mks_src, NULL);
    lv_obj_set_size(tool_panel, TOOL_PANEL_W, TOOL_PANEL_H);
    lv_obj_set_pos(tool_panel, TOOL_PANEL_X, TOOL_PANEL_Y);
    lv_obj_set_style(tool_panel, &tool_panel_style);

    tool_select(tool_last_section);   // tambien fija la pagina activa de la tarea de la interfaz
}

void mks_clear_tool(void) {
    lv_obj_clean(mks_global.mks_src);
}

// La llama la maquina de estados del Wifi (MKS_draw_wifi.cpp) para dibujar cada estado.
void mks_tool_wifi(int state) {
    tool_wifi_state = state;
    if (state == TOOL_WIFI_LIST) tool_wifi_page = 0;
    tool_last_section = SEC_WIFI;
    mks_clear_tool();
    mks_draw_tool();
}
