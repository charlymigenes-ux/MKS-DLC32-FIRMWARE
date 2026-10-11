#include "MKS_preview_core.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace {

struct Win {   // ventana de dibujo (mm)
    float minx, miny, maxx, maxy;
};

struct Pv {
    // estado del G-code
    bool  rel, laser_on;
    int   mode;
    float s, x, y;
    // salida
    uint32_t moves;
    float    travel;
    // pase 1: limites de los tramos con S > 0
    bool  burn;
    Win   b;
    // pase 2: rejilla
    bool      render;
    Win       win;
    float     scale;
    int       gw, gh;
    uint16_t* acc;       // por pixel: bits 15..5 = suma de la potencia (6 bits por muestra), bits 4..0 = muestras (max. 31)
    float     smax_seen; // mayor S visto al medir (escala la potencia: un archivo con S de 0 a 300 no sale apagado)
    float     s_max;
    // cabecera "; Bounds:"
    bool header;
    Win  hb;
};

void pv_reset_state(Pv* p) {
    p->rel = false;
    p->laser_on = true;
    p->mode = 0;
    p->s = 0;
    p->x = p->y = 0;
    p->moves = 0;
    p->travel = 0;
}

// Numero decimal sin exponente; avanza el puntero. Devuelve false si no hay digitos.
bool num(const char*& c, const char* e, float& v) {
    bool  neg = false;
    float r = 0;
    if (c < e && (*c == '-' || *c == '+')) { neg = (*c == '-'); c++; }
    bool any = false;
    while (c < e && *c >= '0' && *c <= '9') { r = r * 10 + (*c - '0'); c++; any = true; }
    if (c < e && *c == '.') {
        c++;
        float f = 0.1f;
        while (c < e && *c >= '0' && *c <= '9') { r += (*c - '0') * f; f *= 0.1f; c++; any = true; }
    }
    v = neg ? -r : r;
    return any;
}

// pw: potencia de la muestra, 0..63. Con 31 muestras se divide todo a la mitad: la media se conserva.
void plot(Pv* p, int ix, int iy, uint8_t pw) {
    if (ix < 0 || iy < 0 || ix >= p->gw || iy >= p->gh) return;
    uint16_t v = p->acc[(size_t)iy * p->gw + ix];
    uint16_t c = v & 31, sm = v >> 5;
    if (c == 31) { sm >>= 1; c = 15; }
    sm += pw;
    c++;
    p->acc[(size_t)iy * p->gw + ix] = (uint16_t)((sm << 5) | c);
}

void seg(Pv* p, float x0, float y0, float x1, float y1, float s) {
    if (s > 0) {
        if (!p->burn) { p->b = { x0, y0, x0, y0 }; p->burn = true; }
        float xs[2] = { x0, x1 }, ys[2] = { y0, y1 };
        for (int k = 0; k < 2; k++) {
            if (xs[k] < p->b.minx) p->b.minx = xs[k];
            if (xs[k] > p->b.maxx) p->b.maxx = xs[k];
            if (ys[k] < p->b.miny) p->b.miny = ys[k];
            if (ys[k] > p->b.maxy) p->b.maxy = ys[k];
        }
    }
    if (!p->render) return;
    float k = s / p->s_max;
    uint8_t pw = (uint8_t)(k >= 1.0f ? 63 : (k <= 0 ? 0 : k * 63.0f + 0.5f));
    float ax = (x0 - p->win.minx) * p->scale + 0.5f, ay = (p->gh - 1) - ((y0 - p->win.miny) * p->scale) + 0.5f;
    float bx = (x1 - p->win.minx) * p->scale + 0.5f, by = (p->gh - 1) - ((y1 - p->win.miny) * p->scale) + 0.5f;
    float dx = bx - ax, dy = by - ay;
    float m = fabsf(dx) > fabsf(dy) ? fabsf(dx) : fabsf(dy);
    int   steps = m < 1 ? 1 : (int)ceilf(m);
    for (int t = 0; t <= steps; t++) {
        plot(p, (int)(ax + dx * t / steps), (int)(ay + dy * t / steps), pw);
    }
}

// Procesa una linea [c, e)
void line(Pv* p, const char* c, const char* e) {
    bool  hasx = false, hasy = false, othg = false, gmotion = false;
    float nx = p->x, ny = p->y, v;
    // cabecera de LightBurn: "; Bounds: X21.96 Y8.61 to X98.04 Y111.39"
    if (c < e && *c == ';') {
        if (!p->header) {
            const char* t = c;
            while (t + 7 < e && memcmp(t, "Bounds:", 7) != 0) t++;
            if (t + 7 < e) {
                t += 7;
                float a[4]; int n = 0;
                while (t < e && n < 4) {
                    if (*t == 'X' || *t == 'Y') { t++; if (num(t, e, v)) a[n++] = v; }
                    else t++;
                }
                if (n == 4 && a[2] > a[0] && a[3] > a[1]) { p->header = true; p->hb = { a[0], a[1], a[2], a[3] }; }
            }
        }
        return;
    }
    while (c < e) {
        char ch = *c;
        if (ch == ';') break;
        if (ch == '(') { while (c < e && *c != ')') c++; c++; continue; }
        if (ch >= 'a' && ch <= 'z') ch -= 32;
        if (ch < 'A' || ch > 'Z') { c++; continue; }
        c++;
        if (!num(c, e, v)) continue;
        int iv = (int)(v + 0.5f);
        switch (ch) {
            case 'G':
                if (iv >= 0 && iv <= 3) { p->mode = iv; gmotion = true; }
                else if (iv == 90) p->rel = false;
                else if (iv == 91) p->rel = true;
                else othg = true;
                break;
            case 'M':
                if (iv == 3 || iv == 4) p->laser_on = true;
                else if (iv == 5) p->laser_on = false;
                break;
            case 'S': p->s = v; if (v > p->smax_seen) p->smax_seen = v; break;
            case 'X': nx = p->rel ? p->x + v : v; hasx = true; break;
            case 'Y': ny = p->rel ? p->y + v : v; hasy = true; break;
        }
    }
    if (!(hasx || hasy)) return;
    if (othg && !gmotion) return;   // G92, G54... con X/Y: no es un movimiento
    float dx = nx - p->x, dy = ny - p->y;
    p->travel += sqrtf(dx * dx + dy * dy);
    p->moves++;
    if (p->mode >= 1 && p->laser_on) seg(p, p->x, p->y, nx, ny, p->s);
    p->x = nx;
    p->y = ny;
}

// Un pase completo sobre el archivo. false si se cancelo o fallo.
bool pass(Pv* p, pv_read_t rd, void* ctx, pv_progress_t prog, uint32_t* done, uint32_t stop_after) {
    static const size_t CH = 2048;
    uint8_t* buf = (uint8_t*)malloc(CH + 256);
    if (!buf) return false;
    size_t keep = 0;   // bytes de una linea cortada que pasan al trozo siguiente
    bool   ok = true;
    for (;;) {
        size_t n = rd(ctx, buf + keep, CH);
        if (n == 0) { if (keep) line(p, (const char*)buf, (const char*)buf + keep); break; }
        *done += (uint32_t)n;
        size_t total = keep + n, s = 0;
        for (size_t i = 0; i < total; i++) {
            if (buf[i] == '\n') {
                line(p, (const char*)buf + s, (const char*)buf + i);
                s = i + 1;
            }
        }
        keep = total - s;
        if (keep > 200) keep = 0;   // linea absurda: se descarta
        else if (keep) memmove(buf, buf + s, keep);
        if (prog && !prog(ctx, *done)) { ok = false; break; }
        if (stop_after && p->header && *done >= stop_after) break;   // cabecera + muestra de S: no hace falta mas
    }
    free(buf);
    return ok;
}

}  // namespace

static float w_of(const Pv* p) { return p->win.maxx - p->win.minx; }
static float h_of(const Pv* p) { return p->win.maxy - p->win.miny; }

void pv_free(pv_image_t* img) {
    if (img && img->nib) { free(img->nib); img->nib = NULL; }
}

bool pv_generate(pv_read_t rd, pv_rewind_t rw, void* ctx, float s_max, int max_side, pv_progress_t prog, pv_image_t* out) {
    memset(out, 0, sizeof(*out));
    Pv* p = (Pv*)calloc(1, sizeof(Pv));
    if (!p) return false;
    p->s_max = s_max > 0 ? s_max : 1000.0f;
    uint32_t done = 0;
    bool ok = false;

    pv_reset_state(p);
    p->smax_seen = 0;
    // Se lee primero la cabecera; si no la hay, un pase completo solo para medir (y se vuelve a empezar)
    p->render = false;
    if (!pass(p, rd, ctx, prog, &done, 65536)) goto fin;
    if (!p->header) {
        // sin cabecera: el pase anterior ya recorrio todo el archivo y midio los tramos con S > 0
        if (!p->burn) goto fin;   // no hay nada que grabar
        p->win = p->b;
    } else {
        p->win = p->hb;
    }
    if (!rw(ctx)) goto fin;

    {
        float w = p->win.maxx - p->win.minx, h = p->win.maxy - p->win.miny;
        if (w < 0.01f) w = 0.01f;
        if (h < 0.01f) h = 0.01f;
        int side = max_side;
        for (;;) {
            float sc = ((float)side - 2) / (w > h ? w : h);
            int   gw = (int)(w * sc) + 3, gh = (int)(h * sc) + 3;
            if (gw > side) gw = side;
            if (gh > side) gh = side;
            p->acc = (uint16_t*)calloc((size_t)gw * gh, sizeof(uint16_t));
            if (p->acc) { p->scale = sc; p->gw = gw; p->gh = gh; break; }
            side -= 16;   // sin memoria: se baja de 16 en 16 (antes se partia por la mitad)
            if (side < 48) goto fin;
        }
    }
    // pase de dibujo: la potencia se escala con el mayor S visto (si no hubo, con el valor recibido)
    if (p->smax_seen > 0) p->s_max = p->smax_seen;
    pv_reset_state(p);
    p->render = true;
    p->burn = false;
    if (!pass(p, rd, ctx, prog, &done, false)) goto fin;

    {
        // niveles automaticos: el 99,5 % de la potencia media queda como negro
        uint32_t hist[256]; memset(hist, 0, sizeof(hist));
        uint32_t seen = 0;
        size_t   np = (size_t)p->gw * p->gh;
        for (size_t i = 0; i < np; i++) {
            uint16_t c = p->acc[i] & 31;
            if (c) { hist[(uint32_t)(p->acc[i] >> 5) * 255 / (63 * c)]++; seen++; }
        }
        float hi = 1.0f;
        uint32_t acc = 0, lim = (uint32_t)(seen * 0.995f);
        for (int i = 0; i < 256; i++) { acc += hist[i]; if (acc >= lim) { hi = (i + 1) / 255.0f; if (hi < 0.05f) hi = 0.05f; break; } }

        size_t rowb = ((size_t)p->gw + 1) / 2;
        out->nib_size = rowb * p->gh;
        out->nib = (uint8_t*)calloc(out->nib_size, 1);
        if (!out->nib) goto fin;
        for (int y = 0; y < p->gh; y++) {
            for (int x = 0; x < p->gw; x++) {
                size_t i = (size_t)y * p->gw + x;
                uint8_t lv = 0;
                uint16_t c = p->acc[i] & 31;
                if (c) {
                    float f = ((float)(p->acc[i] >> 5) / c) / 63.0f / hi;
                    if (f > 1) f = 1;
                    f = powf(f, 0.85f);
                    lv = (uint8_t)(1 + (int)(14.0f * f + 0.5f));
                }
                uint8_t* b = out->nib + y * rowb + (x >> 1);
                if (x & 1) *b |= lv; else *b |= (uint8_t)(lv << 4);   // pixel par en el nibble alto (formato de LVGL)
            }
        }
        out->w = p->gw;
        out->h = p->gh;
        out->width_mm = w_of(p);
        out->height_mm = h_of(p);
        out->travel_m = p->travel / 1000.0f;
        out->moves = p->moves;
        ok = true;
    }
fin:
    free(p->acc);
    free(p);
    if (!ok) pv_free(out);
    return ok;
}
