#include "MKS_draw_fileview.h"
#include "MKS_draw_inFile.h"
#include "MKS_draw_carving.h"
#include "MKS_draw_frame.h"
#include "MKS_draw_language.h"
#include "MKS_preview.h"

extern void mks_infile_start_dialog(void);   // MKS_draw_inFile.cpp: pregunta las pasadas y arranca el trabajo

#define FV_COL_CARD   LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define FV_COL_SIDE   LV_COLOR_MAKE(0x18, 0x1B, 0x28)
#define FV_COL_TRACK  LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define FV_COL_EMER   LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define FV_COL_MUTED  LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)

enum { FV_BACK, FV_START, FV_POS, FV_COUNT };

static lv_style_t fv_btn_style, fv_btn_pr_style, fv_start_style, fv_card_style, fv_paper_style;
static lv_style_t fv_text_style, fv_dark_style, fv_muted_style, fv_green_style, fv_bar_bg_style, fv_bar_style;
static lv_obj_t*  fv_btn[FV_COUNT];
static lv_obj_t  *fv_paper, *fv_img, *fv_wait, *fv_bar, *fv_info[4];
static bool       fv_shown;
static char       fv_name_txt[64], fv_size_txt[16], fv_buf[4][40];
static uint32_t   fv_last_ms;

static const char* fv_T(const char* es, const char* en, const char* ch) {
	return mks_grbl.language == SimpleChinese ? ch : mks_tr(es, en);
}

static void fv_name_fit(char* out, size_t n, const char* name, int max_cp) {
	size_t o = 0;
	int cp = 0;
	for(size_t i = 0; name[i] != '\0' && o + 1 < n; i++) {
		if(((uint8_t)name[i] & 0xC0) != 0x80) {
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

static void fv_styles_init(void) {
	lv_style_copy(&fv_btn_style, &lv_style_plain_color);
	fv_btn_style.body.main_color = FV_COL_CARD;
	fv_btn_style.body.grad_color = FV_COL_CARD;
	fv_btn_style.body.radius = 10;
	fv_btn_style.body.border.width = 0;
	lv_style_copy(&fv_btn_pr_style, &fv_btn_style);
	fv_btn_pr_style.body.main_color = FV_COL_EMER;
	fv_btn_pr_style.body.grad_color = FV_COL_EMER;
	lv_style_copy(&fv_start_style, &fv_btn_style);
	fv_start_style.body.main_color = FV_COL_EMER;
	fv_start_style.body.grad_color = FV_COL_EMER;
	lv_style_copy(&fv_card_style, &fv_btn_style);
	fv_card_style.body.main_color = FV_COL_SIDE;
	fv_card_style.body.grad_color = FV_COL_SIDE;
	fv_card_style.body.radius = 12;
	lv_style_copy(&fv_paper_style, &lv_style_plain_color);     // fondo de la vista previa: negro, como la paleta
	fv_paper_style.body.main_color = LV_COLOR_BLACK;
	fv_paper_style.body.grad_color = LV_COLOR_BLACK;
	fv_paper_style.body.radius = 8;
	fv_paper_style.body.border.width = 2;
	fv_paper_style.body.border.color = FV_COL_TRACK;

	lv_style_copy(&fv_text_style, &lv_style_plain);
	fv_text_style.text.font = mc_font();
	fv_text_style.text.color = LV_COLOR_WHITE;
	lv_style_copy(&fv_dark_style, &fv_text_style);
	fv_dark_style.text.color = LV_COLOR_MAKE(0x08, 0x0C, 0x18);
	lv_style_copy(&fv_muted_style, &fv_text_style);
	fv_muted_style.text.color = FV_COL_MUTED;
	lv_style_copy(&fv_green_style, &fv_text_style);            // el nombre del archivo, en verde
	fv_green_style.text.color = FV_COL_EMER;

	lv_style_copy(&fv_bar_bg_style, &lv_style_plain_color);
	fv_bar_bg_style.body.main_color = FV_COL_TRACK;
	fv_bar_bg_style.body.grad_color = FV_COL_TRACK;
	fv_bar_bg_style.body.radius = 4;
	lv_style_copy(&fv_bar_style, &fv_bar_bg_style);
	fv_bar_style.body.main_color = FV_COL_EMER;
	fv_bar_style.body.grad_color = FV_COL_EMER;
}

static void fv_leave_to_list(void) {
	mks_pv_release();
	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
	lv_obj_clean(mks_global.mks_src);
	mks_draw_craving();
}

static void fv_event(lv_obj_t* obj, lv_event_t event) {
	if(event != LV_EVENT_RELEASED) return;
	if(obj == fv_btn[FV_BACK]) {
		fv_leave_to_list();
	}
	else if(obj == fv_btn[FV_START]) {
		mks_infile_start_dialog();     // el aviso de pasadas; si se acepta, arranca el trabajo
	}
	else if(obj == fv_btn[FV_POS]) {
		mks_pv_release();
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		mks_lv_clean_ui();
		mks_draw_inFile(mks_file_list.filename_str[mks_file_list.file_choose]);
	}
}

static lv_obj_t* fv_mkbtn(int id, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, const char* text,
						  const lv_style_t* label_style, const lv_style_t* rel_style) {
	lv_obj_t* b = lv_btn_create(mks_global.mks_src, NULL);
	lv_obj_set_size(b, w, h);
	lv_obj_set_pos(b, x, y);
	lv_btn_set_style(b, LV_BTN_STYLE_REL, (lv_style_t*)rel_style);
	lv_btn_set_style(b, LV_BTN_STYLE_PR, &fv_btn_pr_style);
	lv_obj_set_event_cb(b, fv_event);
	lv_cont_set_layout(b, LV_LAYOUT_OFF);
	lv_obj_t* l = lv_label_create(b, NULL);
	lv_label_set_style(l, LV_LABEL_STYLE_MAIN, (lv_style_t*)label_style);
	lv_label_set_text(l, text);
	lv_obj_align(l, b, LV_ALIGN_CENTER, 0, 0);
	fv_btn[id] = b;
	return b;
}

// Fila "etiqueta ............ valor" dentro de la tarjeta de datos
static lv_obj_t* fv_row(lv_obj_t* card, int i, const char* label) {
	lv_obj_t* l = lv_label_create(card, NULL);
	lv_label_set_style(l, LV_LABEL_STYLE_MAIN, &fv_muted_style);
	lv_label_set_text(l, label);
	lv_obj_set_pos(l, 14, 10 + i * 50);
	lv_obj_t* v = lv_label_create(card, NULL);
	lv_label_set_style(v, LV_LABEL_STYLE_MAIN, &fv_text_style);
	lv_label_set_text(v, "-");
	lv_obj_set_pos(v, 14, 34 + i * 50);
	return v;
}

static void fv_show_result(void) {
	const lv_img_dsc_t* d = mks_pv_image();
	if(d == NULL) return;
	lv_obj_set_hidden(fv_wait, true);
	lv_obj_set_hidden(fv_bar, true);
	lv_img_set_src(fv_img, d);
	lv_obj_set_hidden(fv_img, false);
	lv_obj_align(fv_img, fv_paper, LV_ALIGN_CENTER, 0, 0);
	float w, h, t; uint32_t m;
	mks_pv_info(&w, &h, &t, &m);
	snprintf(fv_buf[0], sizeof(fv_buf[0]), "%.1f x %.1f mm", w, h);
	snprintf(fv_buf[1], sizeof(fv_buf[1]), "%.1f m", t);
	snprintf(fv_buf[2], sizeof(fv_buf[2]), "%u", (unsigned)m);
	lv_label_set_text(fv_info[0], fv_buf[0]);
	lv_label_set_text(fv_info[1], fv_buf[1]);
	lv_label_set_text(fv_info[2], fv_buf[2]);
	fv_shown = true;
}

void mks_fileview_poll(void) {
	if(fv_shown || fv_wait == NULL) return;
	uint32_t now = lv_tick_get();
	if(now - fv_last_ms < 120) return;
	fv_last_ms = now;
	switch(mks_pv_state()) {
		case PV_RUNNING: {
			static char txt[40];
			snprintf(txt, sizeof(txt), "%s %u %%", fv_T("Generando", "Building", "\xe7\x94\x9f\xe6\x88\x90\xe4\xb8\xad"), (unsigned)mks_pv_progress());
			lv_label_set_text(fv_wait, txt);
			lv_obj_align(fv_wait, fv_paper, LV_ALIGN_CENTER, 0, -10);
			lv_bar_set_value(fv_bar, mks_pv_progress(), LV_ANIM_OFF);
			break;
		}
		case PV_READY:
			fv_show_result();
			break;
		case PV_FAILED:
			static char ftxt[40];
			snprintf(ftxt, sizeof(ftxt), "%s (E%d)", fv_T("Sin vista previa", "No preview", "\xe6\x97\xa0\xe9\xa2\x84\xe8\xa7\x88"), mks_pv_error());
			lv_label_set_text(fv_wait, ftxt);
			lv_obj_align(fv_wait, fv_paper, LV_ALIGN_CENTER, 0, 0);
			lv_obj_set_hidden(fv_bar, true);
			fv_shown = true;
			break;
		default: break;
	}
}

void mks_draw_fileview(char* fn) {
	fv_styles_init();
	for(int i = 0; i < FV_COUNT; i++) fv_btn[i] = NULL;
	fv_shown = false;
	fv_last_ms = 0;

	// los mismos datos que usa mks_draw_inFile: nombre y tamano del archivo elegido
	memset(frame_ctrl.file_name, 0, sizeof(frame_ctrl.file_name));
	memcpy(frame_ctrl.file_name, fn, 128);
	frame_ctrl.file_size = mks_file_list.file_size[mks_file_list.file_choose];

	// barra superior: Atras, nombre en verde y tamano
	lv_obj_t* back = fv_mkbtn(FV_BACK, 8, 5, 84, 28, mc_language.back, &fv_text_style, &fv_btn_style);
	move_page.Back = back;   // los avisos (popups) la habilitan o deshabilitan con set_click_status()
	const char* shown = (fn[0] == '/' || fn[0] == ' ') ? fn + 1 : fn;
	fv_name_fit(fv_name_txt, sizeof(fv_name_txt), shown, 26);
	lv_obj_t* name = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(name, LV_LABEL_STYLE_MAIN, &fv_green_style);
	lv_label_set_text(name, fv_name_txt);
	lv_obj_set_pos(name, 102, 9);
	uint32_t bytes = frame_ctrl.file_size;
	if(bytes < 1024)         snprintf(fv_size_txt, sizeof(fv_size_txt), "%u B", (unsigned)bytes);
	else if(bytes < 1048576) snprintf(fv_size_txt, sizeof(fv_size_txt), "%.1f KB", bytes / 1024.0f);
	else                     snprintf(fv_size_txt, sizeof(fv_size_txt), "%.2f MB", bytes / 1048576.0f);
	lv_obj_t* size = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(size, LV_LABEL_STYLE_MAIN, &fv_muted_style);
	lv_label_set_text(size, fv_size_txt);
	lv_obj_align(size, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 9);

	// la vista previa (papel) a la izquierda; Iniciar justo debajo
	fv_paper = lv_obj_create(mks_global.mks_src, NULL);
	lv_obj_set_size(fv_paper, 176, 176);
	lv_obj_set_pos(fv_paper, 8, 42);
	lv_obj_set_style(fv_paper, &fv_paper_style);
	fv_img = lv_img_create(fv_paper, NULL);
	lv_obj_set_hidden(fv_img, true);
	fv_wait = lv_label_create(fv_paper, NULL);
	lv_label_set_style(fv_wait, LV_LABEL_STYLE_MAIN, &fv_text_style);
	lv_label_set_text(fv_wait, "");
	fv_bar = lv_bar_create(fv_paper, NULL);
	lv_obj_set_size(fv_bar, 130, 8);
	lv_bar_set_style(fv_bar, LV_BAR_STYLE_BG, &fv_bar_bg_style);
	lv_bar_set_style(fv_bar, LV_BAR_STYLE_INDIC, &fv_bar_style);
	lv_bar_set_range(fv_bar, 0, 100);
	lv_bar_set_value(fv_bar, 0, LV_ANIM_OFF);
	lv_obj_align(fv_bar, fv_paper, LV_ALIGN_CENTER, 0, 14);

	lv_obj_t* go = fv_mkbtn(FV_START, 8, 226, 176, 86, mc_language.start, &fv_dark_style, &fv_start_style);
	lv_btn_set_style(go, LV_BTN_STYLE_PR, &fv_btn_pr_style);

	// datos a la derecha y Posicionar abajo
	lv_obj_t* card = lv_obj_create(mks_global.mks_src, NULL);
	lv_obj_set_size(card, 276, 176);
	lv_obj_set_pos(card, 196, 42);
	lv_obj_set_style(card, &fv_card_style);
	fv_info[0] = fv_row(card, 0, fv_T("Dimensiones", "Size", "\xe5\xb0\xba\xe5\xaf\xb8"));
	fv_info[1] = fv_row(card, 1, fv_T("Recorrido", "Path length", "\xe8\xb7\xaf\xe5\xbe\x84"));
	fv_info[2] = fv_row(card, 2, fv_T("Movimientos", "Moves", "\xe6\x8c\x87\xe4\xbb\xa4"));
	lv_obj_t* pos = fv_mkbtn(FV_POS, 196, 228, 276, 84, fv_T("Posicionar", "Position", "\xe5\xae\x9a\xe4\xbd\x8d"), &fv_text_style, &fv_btn_style);
	(void)pos;

	mks_pv_request(fn, frame_ctrl.file_size);
	mks_ui_page.mks_ui_page = MKS_UI_FileView;
}
