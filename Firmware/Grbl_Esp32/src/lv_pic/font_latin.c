/*
 * Fuentes del LCD con letras acentuadas (Latin-1: tildes, enie, ? y ! invertidos).
 *
 * Las fuentes originales (lv_font_roboto_16 y dlc32Font) solo traen ASCII y unos
 * caracteres chinos. Estas dos fuentes envoltorio sirven los caracteres 0xA0..0xFF
 * desde los glifos generados en latin1_roboto_16.c / latin1_roboto_18.c y delegan
 * todo lo demas (ASCII, chino, simbolos) en la fuente original, que no se modifica.
 *
 * Los glifos Latin-1 se generaron con lv_font_conv a partir de Roboto Regular
 * (Apache License 2.0, Google), 4 bpp, sin compresion:
 *   lv_font_conv --font Roboto-Regular.ttf -r 0xA0-0xFF --size 16|18 --bpp 4
 *                --format lvgl --lv-font-name latin1_roboto_16|18 --no-compress
 *
 * dlc32Font es una SimSun de celdas fijas de 10x19 px cuya linea base queda 3 px por
 * encima del fondo de la celda y cuyas mayusculas miden 13 px: equivale a Roboto de
 * 18 px, con los glifos subidos DLC32_BASELINE_OFS px.
 */
#include "lvgl.h"

extern lv_font_t lv_font_roboto_16;
extern lv_font_t dlc32Font;
extern lv_font_t latin1_roboto_16;
extern lv_font_t latin1_roboto_18;

#define LATIN1_FIRST 0xA0
#define LATIN1_LAST 0xFF
#define DLC32_BASELINE_OFS 3

static bool is_latin1(uint32_t c) { return c >= LATIN1_FIRST && c <= LATIN1_LAST; }

/* ---- Roboto 16 con Latin-1 ---- */
static bool roboto16_get_glyph_dsc(const lv_font_t* f, lv_font_glyph_dsc_t* d, uint32_t c, uint32_t n) {
    (void)f;
    if (is_latin1(c)) {
        return latin1_roboto_16.get_glyph_dsc(&latin1_roboto_16, d, c, n);
    }
    return lv_font_roboto_16.get_glyph_dsc(&lv_font_roboto_16, d, c, n);
}

static const uint8_t* roboto16_get_glyph_bitmap(const lv_font_t* f, uint32_t c) {
    (void)f;
    if (is_latin1(c)) {
        return latin1_roboto_16.get_glyph_bitmap(&latin1_roboto_16, c);
    }
    return lv_font_roboto_16.get_glyph_bitmap(&lv_font_roboto_16, c);
}

lv_font_t roboto16Latin = {
    .get_glyph_dsc    = roboto16_get_glyph_dsc,
    .get_glyph_bitmap = roboto16_get_glyph_bitmap,
    .line_height      = 19,
    .base_line        = 4,
};

/* ---- dlc32Font (SimSun) con Latin-1 ---- */
static bool dlc32_get_glyph_dsc(const lv_font_t* f, lv_font_glyph_dsc_t* d, uint32_t c, uint32_t n) {
    (void)f;
    if (is_latin1(c)) {
        if (!latin1_roboto_18.get_glyph_dsc(&latin1_roboto_18, d, c, n)) {
            return false;
        }
        d->ofs_y += DLC32_BASELINE_OFS;
        return true;
    }
    return dlc32Font.get_glyph_dsc(&dlc32Font, d, c, n);
}

static const uint8_t* dlc32_get_glyph_bitmap(const lv_font_t* f, uint32_t c) {
    (void)f;
    if (is_latin1(c)) {
        return latin1_roboto_18.get_glyph_bitmap(&latin1_roboto_18, c);
    }
    return dlc32Font.get_glyph_bitmap(&dlc32Font, c);
}

lv_font_t dlc32FontLatin = {
    .get_glyph_dsc    = dlc32_get_glyph_dsc,
    .get_glyph_bitmap = dlc32_get_glyph_bitmap,
    .line_height      = 19,
    .base_line        = 0,
};
