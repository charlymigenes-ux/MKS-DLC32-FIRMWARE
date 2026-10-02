#include "MKS_draw_carving.h"
#include "MKS_draw_language.h"   // mc_language: textos del LCD
#include "MKS_LVGL.h"
#include "FS.h"
#include "../SDCard.h"

 /* Screan Build */
// static lv_obj_t* scr;

MKS_FILE_LIST_t mks_file_list;

lv_obj_t* caving_src1;
static lv_obj_t* caving_Popup;
lv_obj_t* caving_read_file_src1;

/* style */
static lv_style_t popup_style;
static lv_style_t btn_style;
static lv_style_t caving_src1_style;
/* BTN */
static lv_obj_t* btn_popup_cancle;
static lv_obj_t* btn_popup_sure;
static lv_obj_t* btn_popup_frame;

bool file_popup_select_flag = false;


/* imgbtn */
lv_obj_t* up;
lv_obj_t* next;
lv_obj_t* Cback;

lv_obj_t* label_up;
lv_obj_t* label_next;
lv_obj_t* label_Cback;

lv_obj_t* file_0;
lv_obj_t* file_1;
lv_obj_t* file_2;
lv_obj_t* file_3;
lv_obj_t* file_4;
lv_obj_t* file_5;
lv_obj_t* file_6;
lv_obj_t* file_7;

lv_obj_t* file_list[8];
lv_obj_t* Label_file_list[8];

/* Label */
lv_obj_t* Label_file_0;
lv_obj_t* Label_file_1;
lv_obj_t* Label_file_2;
lv_obj_t* Label_file_3;
lv_obj_t* Label_file_4;
lv_obj_t* Label_file_5;
lv_obj_t* Label_file_6;
lv_obj_t* Label_file_7;

lv_obj_t* Label_NoFile;
lv_obj_t* Label_popup_cancel;
lv_obj_t* Label_popup_sure;
lv_obj_t* Label_popup;
lv_obj_t* Label_popup_file_name;

#define USE_TW_DRAW

LV_IMG_DECLARE(Previous);		//先申明此图片
LV_IMG_DECLARE(Next);			//先申明此图片
LV_IMG_DECLARE(back);			//先申明此图片
LV_IMG_DECLARE(file);			//先申明此图片
LV_IMG_DECLARE(FileDir);		//先申明此图片

LV_IMG_DECLARE(png_previous_pre);		
LV_IMG_DECLARE(png_next_pre);
LV_IMG_DECLARE(png_back_pre);
char file_print_send[128];

#define FILE_NUM		8
#define FIEL_NAME		128
char filename[FILE_NUM][FIEL_NAME];

static void event_handler_up(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {

		char p[30];

		if(file_popup_select_flag == true) return;

		if(mks_readSD_Status() == SDState::NotPresent)  // check sdcard is work
		{

		}
		else{

			if(mks_file_list.file_page == 1) {

			}else {
			mks_file_list.file_count = 0;
				mks_file_list.file_page--;
				// mks_draw_file_loadig();
				// lv_refr_now(lv_refr_get_disp_refreshing());
				// mks_del_file_obj_1(mks_file_list.file_begin_num);
				mks_del_file_obj();
				mks_file_list.file_begin_num = 0;
				mks_listDir(SD, "/",MKS_FILE_DEEP);
				// draw_file_btmimg();
				// draw_file_btmimg_1(mks_file_list.file_begin_num);
				// lv_obj_del(caving_read_file_src1);
				SD.end();
			}
		}
	}
}

static void event_handler_next(lv_obj_t* obj, lv_event_t event) {

		if (event == LV_EVENT_RELEASED) {

			if(file_popup_select_flag == true) return;

			if(mks_readSD_Status() == SDState::NotPresent)  // check sdcard is work
			{ 
				
			}
			else {
			if (event == LV_EVENT_RELEASED) {
				if(mks_file_list.file_begin_num >= MKS_FILE_NUM) {
					mks_file_list.file_count = 0;
					mks_file_list.file_page++;
					// mks_draw_file_loadig();
					// lv_refr_now(lv_refr_get_disp_refreshing());
					mks_del_file_obj();
					mks_file_list.file_begin_num = 0;
					mks_listDir(SD, "/",MKS_FILE_DEEP);
					// draw_file_btmimg();
					// lv_obj_del(caving_read_file_src1);
					SD.end();
				}
			}
		}
	}
}

static void event_handler_cback(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		mks_clear_craving();
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
        mks_ui_page.wait_count = DEFAULT_UI_COUNT;
		mks_draw_ready();
	}
}

static void event_handler_file0(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file0\n");
		mks_file_list.file_choose = 0;
		
#if defined(USE_TW_DRAW)
	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
	mks_lv_clean_ui();
	get_print_file_name(mks_file_list.filename_str[0]);
	frame_ctrl.is_use_same_file = false;
	mks_draw_inFile(mks_file_list.filename_str[0]);
#else
	mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[0]);
#endif
		

		
	}
}

static void event_handler_file1(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file1\n");
		mks_file_list.file_choose = 1;
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[1]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[1]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[1]);
#endif	
		
	}
}

static void event_handler_file2(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file2\n");
		mks_file_list.file_choose = 2;
		
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[2]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[2]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[2]);
#endif
	}
}

static void event_handler_file3(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file3\n");
		mks_file_list.file_choose = 3;
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[3]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[3]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[3]);
#endif
		
	}
}

static void event_handler_file4(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file4\n");
		mks_file_list.file_choose = 4;
		
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[4]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[4]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[4]);
#endif
	}
}

static void event_handler_file5(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file5\n");
		mks_file_list.file_choose = 5;
		
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[5]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[5]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[5]);
#endif
	}
}

static void event_handler_file6(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file6\n");
		mks_file_list.file_choose = 6;
		
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[6]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[6]);
#else 
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[6]);
#endif
	}
}

static void event_handler_file7(lv_obj_t* obj, lv_event_t event) {
	if (event == LV_EVENT_RELEASED) {
		// grbl_send(CLIENT_SERIAL, "file7\n");
		mks_file_list.file_choose = 7;
		
#if defined(USE_TW_DRAW)
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		get_print_file_name(mks_file_list.filename_str[7]);
		frame_ctrl.is_use_same_file = false;
		mks_draw_inFile(mks_file_list.filename_str[7]);
#else
		mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[7]);
#endif
	}
}


// =========================================================================================
// Lista de archivos de la SD: titulo y pagina arriba, menu lateral (Atras, Subir, Siguiente,
// Abrir) y un panel con 5 filas. Tocar una fila la SELECCIONA (marca y color) y el pie del panel
// dice cual es; "Abrir" la abre. Geometria en 480x320: barra y=6, menu x=6 y=38 w=104,
// panel x=118 y=38 w=356 h=272 con filas de 38 px cada 42 y pie a y=226.
// =========================================================================================
#define FILE_SLOTS      8    // tamano de las tablas de punteros (file_0..file_7 se conservan)
#define FILE_COL_CARD   LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define FILE_COL_SIDE   LV_COLOR_MAKE(0x18, 0x1B, 0x28)
#define FILE_COL_TRACK  LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define FILE_COL_EMER   LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define FILE_COL_MUTED  LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)

static lv_style_t file_side_style, file_side_pr_style, file_open_style, file_panel_style;
static lv_style_t file_row_rel_style, file_row_pr_style, file_row_sel_style;
static lv_style_t file_text_style, file_dark_style, file_muted_style, file_sym_style;
static lv_obj_t  *file_panel, *file_title, *file_pagelbl, *file_btn_open, *file_sel_lbl;
static lv_obj_t  *file_btn_back, *file_btn_up, *file_btn_next;
static lv_obj_t  *file_name_lbl[MKS_FILE_NUM], *file_size_lbl[MKS_FILE_NUM], *file_chk_lbl[MKS_FILE_NUM];
static int        file_selected = -1;
static char       file_sel_txt[96], file_page_txt[24];

static const char* file_T(const char* es, const char* en, const char* ch) {
	return mks_grbl.language == SimpleChinese ? ch : (mks_grbl.language == Espanol ? es : en);
}

// Corta el nombre a max_cp caracteres (UTF-8 sin partir un caracter) y pone "..." si no cabe.
static void file_name_fit(char* out, size_t n, const char* name, int max_cp) {
	size_t o = 0;
	int cp = 0;
	for(size_t i = 0; name[i] != '\0' && o + 1 < n; i++) {
		if(((uint8_t)name[i] & 0xC0) != 0x80) {   // inicio de un caracter
			if(cp == max_cp) {
				if(o + 4 < n) { out[o++] = '.'; out[o++] = '.'; out[o++] = '.'; }
				break;
			}
			cp++;
		}
		out[o++] = name[i];
	}
	out[o] = '\0';
}

static void file_size_str(char* out, size_t n, uint32_t bytes) {
	if(bytes < 1024)           snprintf(out, n, "%u B", (unsigned)bytes);
	else if(bytes < 1048576)   snprintf(out, n, "%.1f KB", bytes / 1024.0f);
	else                       snprintf(out, n, "%.2f MB", bytes / 1048576.0f);
}

static void file_styles_init(void) {
	lv_style_copy(&file_side_style, &lv_style_plain_color);
	file_side_style.body.main_color   = FILE_COL_SIDE;
	file_side_style.body.grad_color   = FILE_COL_SIDE;
	file_side_style.body.radius       = 8;
	file_side_style.body.border.width = 0;
	lv_style_copy(&file_side_pr_style, &file_side_style);
	file_side_pr_style.body.main_color = FILE_COL_TRACK;
	file_side_pr_style.body.grad_color = FILE_COL_TRACK;
	lv_style_copy(&file_open_style, &file_side_style);          // "Abrir": apagado hasta que hay seleccion
	file_open_style.body.main_color = LV_COLOR_MAKE(0x24, 0x2A, 0x42);
	file_open_style.body.grad_color = LV_COLOR_MAKE(0x24, 0x2A, 0x42);

	lv_style_copy(&file_panel_style, &lv_style_plain_color);
	file_panel_style.body.main_color   = FILE_COL_CARD;
	file_panel_style.body.grad_color   = FILE_COL_CARD;
	file_panel_style.body.radius       = 12;
	file_panel_style.body.border.width = 1;
	file_panel_style.body.border.color = FILE_COL_TRACK;

	lv_style_copy(&file_row_rel_style, &lv_style_plain_color);
	file_row_rel_style.body.main_color   = FILE_COL_SIDE;
	file_row_rel_style.body.grad_color   = FILE_COL_SIDE;
	file_row_rel_style.body.radius       = 8;
	file_row_rel_style.body.border.width = 0;
	lv_style_copy(&file_row_pr_style, &file_row_rel_style);
	file_row_pr_style.body.main_color = FILE_COL_TRACK;
	file_row_pr_style.body.grad_color = FILE_COL_TRACK;
	lv_style_copy(&file_row_sel_style, &file_row_rel_style);    // fila seleccionada
	file_row_sel_style.body.main_color = FILE_COL_EMER;
	file_row_sel_style.body.grad_color = FILE_COL_EMER;

	lv_style_copy(&file_text_style, &lv_style_plain);
	file_text_style.text.font  = mc_font();
	file_text_style.text.color = LV_COLOR_WHITE;
	lv_style_copy(&file_dark_style, &file_text_style);
	file_dark_style.text.color = LV_COLOR_MAKE(0x08, 0x0C, 0x18);
	lv_style_copy(&file_muted_style, &file_text_style);
	file_muted_style.text.color = FILE_COL_MUTED;
	lv_style_copy(&file_sym_style, &file_dark_style);           // marca de seleccion (simbolo de Roboto)
	file_sym_style.text.font = &roboto16Latin;
}

static void file_nav_style(lv_obj_t* b, bool on) {
	if(b == NULL) return;
	lv_btn_set_style(b, LV_BTN_STYLE_REL, on ? &file_side_style : &file_open_style);
	lv_btn_set_style(b, LV_BTN_STYLE_PR, on ? &file_side_pr_style : &file_open_style);
}

// "1/3" arriba a la derecha; Anterior/Siguiente se apagan en la primera/ultima pagina.
static void file_page_update(void) {
	if(file_pagelbl == NULL) return;
	uint16_t total = mks_file_list.file_total < 1 ? 1 : (mks_file_list.file_total + MKS_FILE_NUM - 1) / MKS_FILE_NUM;
	snprintf(file_page_txt, sizeof(file_page_txt), "%u/%u", (unsigned)mks_file_list.file_page, (unsigned)total);
	lv_label_set_text(file_pagelbl, file_page_txt);
	lv_obj_align(file_pagelbl, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 12);
	file_nav_style(file_btn_up, mks_file_list.file_page > 1);
	file_nav_style(file_btn_next, mks_file_list.file_page < total);
}

// Abre el archivo de la fila n: lo mismo que hacian antes los manejadores file0..file7.
static void file_open_row(int n) {
	if(n < 0 || n >= mks_file_list.file_begin_num) return;
	mks_file_list.file_choose = n;
#if defined(USE_TW_DRAW)
	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
	mks_lv_clean_ui();
	get_print_file_name(mks_file_list.filename_str[n]);
	frame_ctrl.is_use_same_file = false;
	mks_draw_inFile(mks_file_list.filename_str[n]);
#else
	mks_draw_caving_popup(mks_file_list.file_begin_num, mks_file_list.filename_str[n]);
#endif
}

static void file_row_event(lv_obj_t* obj, lv_event_t event) {
	if(event != LV_EVENT_RELEASED) return;
	lv_obj_t* rows[MKS_FILE_NUM] = { file_0, file_1, file_2, file_3, file_4 };
	for(int i = 0; i < MKS_FILE_NUM; i++) {
		if(obj == rows[i]) { file_open_row(i); return; }   // tocar un archivo lo abre directamente
	}
}

static lv_obj_t* file_nav_btn(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, const char* text, lv_event_cb_t cb, const lv_style_t* style) {
	lv_obj_t* b = lv_btn_create(mks_global.mks_src, NULL);
	lv_obj_set_size(b, w, h);
	lv_obj_set_pos(b, x, y);
	lv_btn_set_style(b, LV_BTN_STYLE_REL, (lv_style_t*)style);
	lv_btn_set_style(b, LV_BTN_STYLE_PR, &file_side_pr_style);
	lv_obj_set_event_cb(b, cb);
	lv_cont_set_layout(b, LV_LAYOUT_OFF);
	lv_obj_t* l = lv_label_create(b, NULL);
	lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &file_text_style);
	lv_label_set_text(l, text);
	lv_obj_align(l, b, LV_ALIGN_CENTER, 0, 0);
	return b;
}

static void file_screen_build(void) {
	file_styles_init();

	// barra superior: Atras (al menu), titulo y pagina
	file_btn_back = file_nav_btn(6, 6, 96, 32, mc_language.back, event_handler_cback, &file_side_style);

	file_title = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(file_title, LV_LABEL_STYLE_MAIN, &file_text_style);
	lv_label_set_text(file_title, file_T("Archivos SD", "SD Files", "SD\xe5\x8d\xa1"));   // SD卡
	lv_obj_set_pos(file_title, 116, 12);

	file_pagelbl = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(file_pagelbl, LV_LABEL_STYLE_MAIN, &file_muted_style);
	lv_label_set_text(file_pagelbl, "");

	// pie: Anterior / Siguiente, mitad y mitad
	file_btn_up   = file_nav_btn(6, 274, 232, 40, mc_language.up, event_handler_up, &file_open_style);
	file_btn_next = file_nav_btn(242, 274, 232, 40, mc_language.next, event_handler_next, &file_open_style);

	file_panel = lv_obj_create(mks_global.mks_src, NULL);
	lv_obj_set_size(file_panel, 468, 222);
	lv_obj_set_pos(file_panel, 6, 44);
	lv_obj_set_style(file_panel, &file_panel_style);
	file_page_update();
}

void draw_filexx(uint8_t num, char *name) {

	if(name[0] == '/') name[0] = ' ';
	if(num >= MKS_FILE_NUM || file_panel == NULL) return;

	lv_obj_t** rows[FILE_SLOTS] = { &file_0, &file_1, &file_2, &file_3, &file_4, &file_5, &file_6, &file_7 };
	lv_obj_t** labs[FILE_SLOTS] = { &Label_file_0, &Label_file_1, &Label_file_2, &Label_file_3,
									&Label_file_4, &Label_file_5, &Label_file_6, &Label_file_7 };

	if(num == 0) file_page_update();

	lv_obj_t* row = lv_btn_create(file_panel, NULL);
	lv_obj_set_size(row, 444, 38);
	lv_obj_set_pos(row, 10, 6 + num * 42);
	lv_btn_set_style(row, LV_BTN_STYLE_REL, &file_row_rel_style);
	lv_btn_set_style(row, LV_BTN_STYLE_PR, &file_row_pr_style);
	lv_obj_set_event_cb(row, file_row_event);
	lv_cont_set_layout(row, LV_LAYOUT_OFF);   // sin la columna centrada del boton: se colocan a mano
	*rows[num] = row;

	char shown[64];
	file_name_fit(shown, sizeof(shown), name, 32);
	file_name_lbl[num] = lv_label_create(row, NULL);
	lv_label_set_style(file_name_lbl[num], LV_LABEL_STYLE_MAIN, &file_text_style);
	lv_label_set_text(file_name_lbl[num], shown);
	lv_obj_align(file_name_lbl[num], row, LV_ALIGN_IN_LEFT_MID, 14, 0);
	*labs[num] = file_name_lbl[num];

	char sz[16];
	file_size_str(sz, sizeof(sz), mks_file_list.file_size[num]);
	file_size_lbl[num] = lv_label_create(row, NULL);
	lv_label_set_style(file_size_lbl[num], LV_LABEL_STYLE_MAIN, &file_muted_style);
	lv_label_set_text(file_size_lbl[num], sz);
	lv_obj_align(file_size_lbl[num], row, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	lv_refr_now(lv_refr_get_disp_refreshing());
}

void mks_draw_craving(void) {

	SDState state = get_sd_state(true);

	// punteros a filas de una lista anterior (sus objetos ya se borraron al salir de la pagina)
	file_0 = file_1 = file_2 = file_3 = file_4 = file_5 = file_6 = file_7 = NULL;
	Label_file_0 = Label_file_1 = Label_file_2 = Label_file_3 = NULL;
	Label_file_4 = Label_file_5 = Label_file_6 = Label_file_7 = NULL;

	file_screen_build();

	// if(mks_readSD_Status() == SDState::NotPresent)  // check sdcard is work
	if(state == SDState::NotPresent)
	{
		mks_grbl.mks_sd_status = 0;	// no sd insert
		label_for_screen(mks_global.mks_src, Label_NoFile, 0, 0, mc_language.dis_no_sd_card);
	}else {

		
		mks_file_list.file_begin_num = 0;
		mks_file_list.file_count = 0;
		mks_file_list.file_page = 1;
		mks_file_list.file_total = mks_countDir(SD, "/", MKS_FILE_DEEP);
		mks_grbl.mks_sd_status = 1; // sd had inserted
		file_page_update();

		mks_listDir(SD, "/",MKS_FILE_DEEP);
		SD.end();
	}	
	mks_ui_page.mks_ui_page = MKS_UI_Caving;
    mks_ui_page.wait_count = DEFAULT_UI_COUNT;
}

void draw_file_btmimg(void) {

	char filename_dis_str[MKS_FILE_NUM][MKS_FILE_NAME_LENGTH];

	for(uint8_t i=0; i<mks_file_list.file_begin_num; i++) {
		strcpy(filename_dis_str[i], mks_file_list.filename_str[i]);
		if(filename_dis_str[i][0] == '/') filename_dis_str[i][0] = ' ';
	}
	
	if(mks_file_list.file_begin_num == 1) {
		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, Label_file_0, 
																		caving_first_file_label_x, 
																		caving_first_file_label_y, 
																		// mks_file_list.filename_str[0], 
																		filename_dis_str[0],
																		caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 2) {
		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, 
									file_0, 
									&file, 
									&file, 
									LV_ALIGN_CENTER, 
									caving_first_file_x, 
									caving_first_file_y, 
									event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, Label_file_0, 
																		caving_first_file_label_x, 
																		caving_first_file_label_y, 
																		// mks_file_list.filename_str[0],
																		filename_dis_str[0],
																		caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
																		Label_file_1, 
																		caving_first_file_label_x + 120, 
																		caving_first_file_label_y, 
																		// mks_file_list.filename_str[1], 
																		filename_dis_str[1],
																		caving_file_name_show);	
	}
	else if(mks_file_list.file_begin_num == 3) {

		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, 
			Label_file_0, 
			caving_first_file_label_x, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[0],
			filename_dis_str[0], 
			caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
			Label_file_1, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[1], 
			filename_dis_str[1],
			caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
			Label_file_2, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[2], 
			filename_dis_str[2],
			caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 4) {

		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, 
			Label_file_0, 
			caving_first_file_label_x, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[0], 
			filename_dis_str[0],
			caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
			Label_file_1, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[1],
			filename_dis_str[1], 
			caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
			Label_file_2, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[2], 
			filename_dis_str[2],
			caving_file_name_show);

		file_3 = lv_imgbtn_creat_mks(mks_global.mks_src, file_3, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y, event_handler_file3);
		Label_file_3 = label_for_file(mks_global.mks_src, 
			Label_file_3, 
			caving_first_file_label_x + 360, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[3], 
			filename_dis_str[3],
			caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 5) {
		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, 
			Label_file_0, 
			caving_first_file_label_x, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[0],
			filename_dis_str[0], 
			caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
			Label_file_1, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[1], 
			filename_dis_str[1],
			caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
			Label_file_2, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[2], 
			filename_dis_str[2],
			caving_file_name_show);

		file_3 = lv_imgbtn_creat_mks(mks_global.mks_src, file_3, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y, event_handler_file3);
		Label_file_3 = label_for_file(mks_global.mks_src, 
			Label_file_3, 
			caving_first_file_label_x + 360, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[3], 
			filename_dis_str[3],
			caving_file_name_show);

		file_4 = lv_imgbtn_creat_mks(mks_global.mks_src, file_4, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y + 105, event_handler_file4);
		Label_file_4 = label_for_file(mks_global.mks_src, 
			Label_file_4, 
			caving_first_file_label_x, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[4], 
			filename_dis_str[4],
			caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 6) {

		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, 
			Label_file_0, 
			caving_first_file_label_x, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[0], 
			filename_dis_str[0],
			caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
			Label_file_1, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[1], 
			filename_dis_str[1],
			caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
			Label_file_2, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[2], 
			filename_dis_str[2],
			caving_file_name_show);

		file_3 = lv_imgbtn_creat_mks(mks_global.mks_src, file_3, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y, event_handler_file3);
		Label_file_3 = label_for_file(mks_global.mks_src, 
			Label_file_3, 
			caving_first_file_label_x + 360, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[3], 
			filename_dis_str[3],
			caving_file_name_show);

		file_4 = lv_imgbtn_creat_mks(mks_global.mks_src, file_4, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y + 105, event_handler_file4);
		Label_file_4 = label_for_file(mks_global.mks_src, 
			Label_file_4, 
			caving_first_file_label_x, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[4], 
			filename_dis_str[4],
			caving_file_name_show);

		file_5 = lv_imgbtn_creat_mks(mks_global.mks_src, file_5, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y + 105, event_handler_file5);
		Label_file_5 = label_for_file(mks_global.mks_src, 
			Label_file_5, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[5], 
			filename_dis_str[5],
			caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 7) {
		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, 
			Label_file_0, 
			caving_first_file_label_x, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[0],
			filename_dis_str[0],
			caving_file_name_show);

		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
			Label_file_1, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[1], 
			filename_dis_str[1],
			caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
			Label_file_2, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[2], 
			filename_dis_str[2],
			caving_file_name_show);

		file_3 = lv_imgbtn_creat_mks(mks_global.mks_src, file_3, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y, event_handler_file3);
		Label_file_3 = label_for_file(mks_global.mks_src, 
			Label_file_3, 
			caving_first_file_label_x + 360, 
			caving_first_file_label_y, 
			// mks_file_list.filename_str[3], 
			filename_dis_str[3],
			caving_file_name_show);

		file_4 = lv_imgbtn_creat_mks(mks_global.mks_src, file_4, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y + 105, event_handler_file4);
		Label_file_4 = label_for_file(mks_global.mks_src, 
			Label_file_4, 
			caving_first_file_label_x, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[4], 
			filename_dis_str[4],
			caving_file_name_show);

		file_5 = lv_imgbtn_creat_mks(mks_global.mks_src, file_5, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y + 105, event_handler_file5);
		Label_file_5 = label_for_file(mks_global.mks_src, 
			Label_file_5, 
			caving_first_file_label_x + 120, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[5], 
			filename_dis_str[5],
			caving_file_name_show);

		file_6 = lv_imgbtn_creat_mks(mks_global.mks_src, file_6, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y + 105, event_handler_file6);
		Label_file_6 = label_for_file(mks_global.mks_src, 
			Label_file_6, 
			caving_first_file_label_x + 240, 
			caving_first_file_label_y + 105, 
			// mks_file_list.filename_str[6], 
			filename_dis_str[6],
			caving_file_name_show);
	}
	else if(mks_file_list.file_begin_num == 8) {

		file_0 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y, event_handler_file0);
		Label_file_0 = label_for_file(mks_global.mks_src, Label_file_0, caving_first_file_x, caving_first_file_y+30, filename_dis_str[0], caving_file_name_show);

	
		file_1 = lv_imgbtn_creat_mks(mks_global.mks_src, file_1, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y, event_handler_file1);
		Label_file_1 = label_for_file(mks_global.mks_src, 
				Label_file_1, 
				caving_first_file_label_x + 120, 
				caving_first_file_label_y, 
				// mks_file_list.filename_str[1], 
				filename_dis_str[1],
				caving_file_name_show);

		file_2 = lv_imgbtn_creat_mks(mks_global.mks_src, file_0, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y, event_handler_file2);
		Label_file_2 = label_for_file(mks_global.mks_src, 
				Label_file_2, 
				caving_first_file_label_x + 240, 
				caving_first_file_label_y, 
				// mks_file_list.filename_str[2], 
				filename_dis_str[2],
				caving_file_name_show);

		file_3 = lv_imgbtn_creat_mks(mks_global.mks_src, file_3, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y, event_handler_file3);
		Label_file_3 = label_for_file(mks_global.mks_src, 
				Label_file_3, 
				caving_first_file_label_x + 360, 
				caving_first_file_label_y, 
				// mks_file_list.filename_str[3], 
				filename_dis_str[3],
				caving_file_name_show);

		file_4 = lv_imgbtn_creat_mks(mks_global.mks_src, file_4, &file, &file, LV_ALIGN_CENTER, caving_first_file_x, caving_first_file_y + 105, event_handler_file4);
		Label_file_4 = label_for_file(mks_global.mks_src, 
				Label_file_4, 
				caving_first_file_label_x, 
				caving_first_file_label_y + 105, 
				// mks_file_list.filename_str[4], 
				filename_dis_str[4],
				caving_file_name_show);

		file_5 = lv_imgbtn_creat_mks(mks_global.mks_src, file_5, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 120, caving_first_file_y + 105, event_handler_file5);
		Label_file_5 = label_for_file(mks_global.mks_src, 
				Label_file_5, 
				caving_first_file_label_x + 120, 
				caving_first_file_label_y + 105, 
				// mks_file_list.filename_str[5], 
				filename_dis_str[5],
				caving_file_name_show);

		file_6 = lv_imgbtn_creat_mks(mks_global.mks_src, file_6, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 240, caving_first_file_y + 105, event_handler_file6);
		Label_file_6 = label_for_file(mks_global.mks_src, 
				Label_file_6, 
				caving_first_file_label_x + 240, 
				caving_first_file_label_y + 105, 
				// mks_file_list.filename_str[6], 
				filename_dis_str[6],
				caving_file_name_show);

		file_7 = lv_imgbtn_creat_mks(mks_global.mks_src, file_7, &file, &file, LV_ALIGN_CENTER, caving_first_file_x + 360, caving_first_file_y + 105, event_handler_file7);
		Label_file_7 = label_for_file(mks_global.mks_src, 
				Label_file_7, 
				caving_first_file_label_x + 360, 
				caving_first_file_label_y + 105, 
				// mks_file_list.filename_str[7], 
				filename_dis_str[7],
				caving_file_name_show);
	}
	
}

void mks_del_file_obj(void) {

	lv_obj_t** rows[FILE_SLOTS] = { &file_0, &file_1, &file_2, &file_3, &file_4, &file_5, &file_6, &file_7 };
	lv_obj_t** labs[FILE_SLOTS] = { &Label_file_0, &Label_file_1, &Label_file_2, &Label_file_3,
									&Label_file_4, &Label_file_5, &Label_file_6, &Label_file_7 };

	for(uint8_t i = 0; i < MKS_FILE_NUM && i < mks_file_list.file_begin_num; i++) {
		if(*rows[i] != NULL) {
			lv_obj_del(*rows[i]);   // sus etiquetas (marca, nombre y tamano) son hijas: se borran con ella
			*rows[i] = NULL;
			*labs[i] = NULL;
		}
	}
}

void disable_file_click() {

	if(mks_file_list.file_begin_num == 1) {
		lv_obj_set_click(move_page.Back, true);
	}
	else if(mks_file_list.file_begin_num == 2) {

	}
}

static void event_btn_cancle(lv_obj_t* obj, lv_event_t event) {

    if (event == LV_EVENT_RELEASED) {
		file_popup_select_flag = false;
        lv_obj_del(caving_Popup);
	}
}

void start_print(void) { 

	char str_cmd[255] = "[ESP220]";
	char line_num[50];
	file_popup_select_flag = false;
	mks_grbl.is_mks_ts35_flag = true;

	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING; 
	mks_ui_page.wait_count = DEFAULT_UI_COUNT;
 
	strcat(str_cmd, file_print_send);
	strcat(str_cmd,"\n");
	MKS_GRBL_CMD_SEND(str_cmd);
	grbl_send(CLIENT_SERIAL, str_cmd);
	mks_draw_print();
}

static void event_btn_sure(lv_obj_t* obj, lv_event_t event) {
	char str_cmd[255] = "[ESP220]";
	char line_num[50];
    if (event == LV_EVENT_RELEASED) {

		file_popup_select_flag = false;

		if(sys.state != State::Idle) {
			lv_obj_del(caving_Popup);
			return ;
		}
		mks_grbl.is_mks_ts35_flag = true;

		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING; 
		mks_ui_page.wait_count = DEFAULT_UI_COUNT;
        lv_obj_del(caving_Popup);
		mks_clear_craving();

		ddxd = sd_get_current_line_number();
		sprintf(line_num ,"%d", ddxd);
        tf.writeFile("/PLA.txt", line_num);

		strcat(str_cmd, file_print_send);
		strcat(str_cmd,"\n");
		MKS_GRBL_CMD_SEND(str_cmd);
		grbl_send(CLIENT_SERIAL, str_cmd);
		mks_draw_print();
	}
}

static void event_btn_sure_alarm(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		lv_obj_del(caving_Popup);
	}
}

static void event_fram_size_yes(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		lv_obj_del(com_p1.com_popup_src);
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
        mks_ui_page.wait_count = 1;
		mks_draw_frame();
		lv_refr_now(lv_refr_get_disp_refreshing());
		(frame_ctrl.file_name);
		file_popup_select_flag = false;
	}
}

static void event_fram_size_no(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		file_popup_select_flag = false;
		lv_obj_del(com_p1.com_popup_src);
	}
}

static void event_btn_frame(lv_obj_t* obj, lv_event_t event) {

	uint32_t file_size = mks_file_list.file_size[mks_file_list.file_choose];   

	if (event == LV_EVENT_RELEASED) {

		lv_obj_del(caving_Popup);

		if(file_size >= 1024*1024) {
			mks_draw_common_popup(mc_language.dis_warning, 
								mc_language.dis_file_too_big,
								mc_language.dis_continue_sure,
								event_fram_size_yes,
								event_fram_size_no);
		}else {
			
			mks_draw_frame();
			lv_refr_now(lv_refr_get_disp_refreshing());
			// mks_run_frame(frame_ctrl.file_name);
			file_popup_select_flag = false;
		}
	}
}

void mks_draw_caving_popup(uint8_t text, char *srt) {

	char file_name[128];

	if(file_popup_select_flag == true) return;

	file_popup_select_flag = true;

	caving_Popup = lv_obj_create(mks_global.mks_src, NULL);

	lv_obj_set_size(caving_Popup ,350, 200);
	lv_obj_set_pos(caving_Popup, 80,50);

	lv_style_copy(&popup_style, &lv_style_scr);
	popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5); 
    popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5); 
	popup_style.text.color = LV_COLOR_BLACK;
	popup_style.body.radius = 17;
	lv_obj_set_style(caving_Popup, &popup_style);
	
	lv_style_copy(&btn_style, &lv_style_scr);
    btn_style.body.main_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
    btn_style.body.grad_color = LV_COLOR_MAKE(0x3F, 0x46, 0x66);
	btn_style.body.radius = 10;
    btn_style.body.opa = LV_OPA_COVER; // 设置背景色完全不透明
    btn_style.text.color = LV_COLOR_WHITE;
	
	btn_popup_sure = mks_lv_btn_set(caving_Popup, btn_popup_sure, 100,40,10,130,event_btn_sure);
	lv_btn_set_style(btn_popup_sure, LV_BTN_STYLE_REL, &btn_style);
    lv_btn_set_style(btn_popup_sure,LV_BTN_STYLE_PR,&btn_style);
	mks_lvgl_long_sroll_label_with_wight_set_center(btn_popup_sure, Label_popup_sure, 50, 0, mc_language.yes,50);


	btn_popup_frame = mks_lv_btn_set(caving_Popup, btn_popup_frame, 100,40,125,130,event_btn_frame);
	lv_btn_set_style(btn_popup_frame, LV_BTN_STYLE_REL, &btn_style);
    lv_btn_set_style(btn_popup_frame,LV_BTN_STYLE_PR,&btn_style);
	mks_lvgl_long_sroll_label_with_wight_set_center(btn_popup_frame, Label_popup_sure, 60, 0, mc_language.frame,50);
	
	btn_popup_cancle = mks_lv_btn_set(caving_Popup, btn_popup_cancle, 100,40,240,130,event_btn_cancle);
	lv_btn_set_style(btn_popup_cancle, LV_BTN_STYLE_REL, &btn_style);
    lv_btn_set_style(btn_popup_cancle,LV_BTN_STYLE_PR,&btn_style);
	mks_lvgl_long_sroll_label_with_wight_set_center(btn_popup_cancle, Label_popup_sure, 50, 0, mc_language.cancel,50);

	// memcpy(file_print_send, srt, MKS_FILE_NAME_LENGTH);
	memset(file_print_send, 0, sizeof(file_print_send));
	memset(frame_ctrl.file_name, 0, sizeof(frame_ctrl.file_name));
	
	strcpy(file_print_send, srt);
	strcpy(frame_ctrl.file_name, srt);
	strcpy(file_name, srt);


	if(file_name[0] == '/') file_name[0] = ' ';
	// mks_lvgl_long_sroll_label_with_wight_set(caving_Popup, Label_popup_file_name, 100, 40, file_name, 255);
	// mks_lvgl_long_sroll_label_with_wight_set(caving_Popup, Label_popup, 100, 60, mc_language.carve_file_sure,255);
	label_for_screen(caving_Popup, Label_popup_file_name, 0, -20, file_name);
	label_for_screen(caving_Popup, Label_popup, 0, 0, mc_language.carve_file_sure);
	
}


void get_print_file_name(char *srt) { 

	memset(file_print_send, 0, sizeof(file_print_send));
	memset(frame_ctrl.file_name, 0, sizeof(frame_ctrl.file_name));
	
	strcpy(file_print_send, srt);
	strcpy(frame_ctrl.file_name, srt);
}


void mks_draw_file_loadig(void) {

	caving_read_file_src1 = lv_obj_create(mks_global.mks_src, NULL);

	lv_obj_set_size(caving_read_file_src1 ,350, 200);
	lv_obj_set_pos(caving_read_file_src1, 80,50);

	lv_style_copy(&popup_style, &lv_style_scr);
	popup_style.body.main_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5); 
    popup_style.body.grad_color = LV_COLOR_MAKE(0xCE, 0xD6, 0xE5); 
	popup_style.text.color = LV_COLOR_BLACK;
	popup_style.body.radius = 17;
	lv_obj_set_style(caving_read_file_src1, &popup_style);
	mks_lvgl_long_sroll_label_with_wight_set(caving_read_file_src1, Label_popup, 110, 80, mc_language.dis_file_loading,240);
}

void mks_clear_craving(void) {
	
	lv_obj_clean(mks_global.mks_src);

}
