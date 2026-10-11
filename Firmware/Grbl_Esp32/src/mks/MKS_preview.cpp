#include "MKS_preview.h"
#include "MKS_preview_core.h"

#include "../Grbl.h"
#include "../SDCard.h"
#include "../Settings.h"
#include <SD.h>
#include <SPIFFS.h>
#include <math.h>
#include "esp_heap_caps.h"

namespace {

const uint32_t PV_MAGIC = 0x3150584D;   // "MXP1"
const int      PV_SIDE  = 152;          // lado maximo de la miniatura (152 x 152 x 2 bytes = 46 KB de acumulador)
const size_t   PV_MAX_CACHE = 30;       // archivos de cache en SPIFFS (si se llena, se borran todos)

volatile mks_pv_state_t st = PV_IDLE;
volatile uint8_t       prog = 0;
volatile bool          cancel_flag = false;
volatile bool          task_alive = false;
volatile int           err_code = 0;   // por que fallo (se muestra en pantalla y en la consola)

char     req_path[130];
uint32_t req_size = 0;

lv_img_dsc_t dsc;
uint8_t*     dsc_data = NULL;
float        info_w = 0, info_h = 0, info_travel = 0;
uint32_t     info_moves = 0;

struct CacheHdr {
    uint32_t magic;
    uint32_t size;       // tamano del archivo G-code (invalida la cache si cambia)
    uint16_t w, h;
    float    width_mm, height_mm, travel_m;
    uint32_t moves;
};

uint32_t fnv(const char* s, uint32_t size) {
    uint32_t h = 2166136261u ^ size;
    for (; *s; s++) { h ^= (uint8_t)*s; h *= 16777619u; }
    return h;
}

void cache_name(char* out, size_t n, const char* path, uint32_t size) {
    snprintf(out, n, "/pv_%08x", (unsigned)fnv(path, size));
}

void free_image() {
    if (dsc_data) { free(dsc_data); dsc_data = NULL; }
    memset(&dsc, 0, sizeof(dsc));
}

// Paleta de la pantalla (fondo negro, puntos de grabado en blanco): 0 = fuera del recorrido, 1 = sin grabar (negro)
// ... 15 = potencia maxima (blanco). El archivo guarda niveles de potencia, no colores: la web usa la paleta contraria.
// Orden LVGL: B, G, R, A.
void palette(uint8_t* p) {
    for (int i = 0; i < 16; i++) {
        uint8_t c = (i == 0) ? 0 : (uint8_t)((i - 1) * 255 / 14);
        p[i * 4 + 0] = c; p[i * 4 + 1] = c; p[i * 4 + 2] = c; p[i * 4 + 3] = 255;
    }
}

bool build_image(const uint8_t* nib, size_t nib_size, uint16_t w, uint16_t h) {
    free_image();
    dsc_data = (uint8_t*)malloc(64 + nib_size);
    if (!dsc_data) return false;
    palette(dsc_data);
    memcpy(dsc_data + 64, nib, nib_size);
    dsc.header.always_zero = 0;
    dsc.header.cf = LV_IMG_CF_INDEXED_4BIT;
    dsc.header.w = w;
    dsc.header.h = h;
    dsc.data_size = 64 + nib_size;
    dsc.data = dsc_data;
    return true;
}

bool cache_load(const char* path, uint32_t size) {
    char nm[24];
    cache_name(nm, sizeof(nm), path, size);
    if (!SPIFFS.exists(nm)) return false;
    File f = SPIFFS.open(nm, "r");
    if (!f) return false;
    CacheHdr hd;
    bool ok = false;
    if (f.read((uint8_t*)&hd, sizeof(hd)) == sizeof(hd) && hd.magic == PV_MAGIC && hd.size == size && hd.w > 0 && hd.h > 0) {
        size_t nb = ((size_t)hd.w + 1) / 2 * hd.h;
        uint8_t* tmp = (uint8_t*)malloc(nb);
        if (tmp) {
            if (f.read(tmp, nb) == nb && build_image(tmp, nb, hd.w, hd.h)) {
                info_w = hd.width_mm; info_h = hd.height_mm; info_travel = hd.travel_m; info_moves = hd.moves;
                ok = true;
            }
            free(tmp);
        }
    }
    f.close();
    return ok;
}

// Miniatura que sube el navegador a la SD ("/.mx/pv_<hash>.mxp", mismo formato que la cache de SPIFFS).
// El hash es el mismo de cache_name(); asi la pantalla se ahorra leer y calcular el G-code.
bool sd_cache_load(const char* path, uint32_t size) {
    char nm[40];
    snprintf(nm, sizeof(nm), "/.mx/pv_%08x.mxp", (unsigned)fnv(path, size));
    File f = SD.open(nm);
    if (!f) return false;
    CacheHdr hd;
    bool ok = false;
    if (f.read((uint8_t*)&hd, sizeof(hd)) == sizeof(hd) && hd.magic == PV_MAGIC && hd.size == size && hd.w > 0 && hd.h > 0 && hd.w <= 256 && hd.h <= 256) {
        size_t nb = ((size_t)hd.w + 1) / 2 * hd.h;
        uint8_t* tmp = (uint8_t*)malloc(nb);
        if (tmp) {
            if (f.read(tmp, nb) == nb && build_image(tmp, nb, hd.w, hd.h)) {
                info_w = hd.width_mm; info_h = hd.height_mm; info_travel = hd.travel_m; info_moves = hd.moves;
                ok = true;
            }
            free(tmp);
        }
    }
    f.close();
    return ok;
}

void cache_trim() {
    // SPIFFS no tiene carpetas: se cuentan los pv_*; si ya hay demasiados se borran todos
    size_t n = 0;
    File root = SPIFFS.open("/");
    File e = root.openNextFile();
    char names[PV_MAX_CACHE + 2][24];
    while (e) {
        const char* nm = e.name();
        if (strstr(nm, "/pv_") == nm || strstr(nm, "pv_") == nm) {
            if (n < PV_MAX_CACHE + 2) snprintf(names[n], 24, "%s%s", nm[0] == '/' ? "" : "/", nm);
            n++;
        }
        e.close();
        e = root.openNextFile();
    }
    root.close();
    if (n >= PV_MAX_CACHE) {
        for (size_t i = 0; i < n && i < PV_MAX_CACHE + 2; i++) SPIFFS.remove(names[i]);
    }
}

void cache_save(const char* path, uint32_t size, const pv_image_t& im) {
    cache_trim();
    char nm[24];
    cache_name(nm, sizeof(nm), path, size);
    File f = SPIFFS.open(nm, "w");
    if (!f) return;
    CacheHdr hd = { PV_MAGIC, size, im.w, im.h, im.width_mm, im.height_mm, im.travel_m, im.moves };
    f.write((const uint8_t*)&hd, sizeof(hd));
    f.write(im.nib, im.nib_size);
    f.close();
}

struct Rd {
    File     f;
    uint32_t size;
    uint32_t chunks;
};

size_t rd_cb(void* c, uint8_t* b, size_t n) {
    return ((Rd*)c)->f.read(b, n);
}
bool rw_cb(void* c) {
    return ((Rd*)c)->f.seek(0);
}
bool prog_cb(void* c, uint32_t done) {
    Rd* r = (Rd*)c;
    if (r->size) {
        uint32_t p = (uint32_t)(((uint64_t)done * 100) / (r->size ? r->size : 1));
        prog = p > 99 ? 99 : (uint8_t)p;
    }
    if ((++r->chunks & 0xF) == 0) vTaskDelay(1);   // no acapara la CPU
    return !cancel_flag;
}

void pv_task(void*) {
    bool ok = false;
    bool we_set = false;
    SDState sdst = get_sd_state(true);
    if (sdst != SDState::Idle) {
        err_code = (sdst == SDState::NotPresent) ? 11 : 12;   // 11 = sin SD, 12 = SD ocupada
    } else {
        set_sd_state(SDState::BusyParsing);
        we_set = true;
        Rd r;
        if (sd_cache_load(req_path, req_size)) {              // la subio la WebUI: listo sin leer el G-code
            set_sd_state(SDState::Idle);
            st = PV_READY;
            task_alive = false;
            vTaskDelete(NULL);
            return;
        }
        r.f = SD.open(req_path);
        if (!r.f) err_code = 2;                               // 2 = no se pudo abrir el archivo
        r.size = req_size;
        r.chunks = 0;
        if (r.f) {
            float smax = rpm_max ? rpm_max->get() : 1000.0f;
            pv_image_t im;
            // lado segun el mayor bloque libre (acumulador de 2 bytes por pixel, con margen), multiplo de 8
            int side = (int)sqrtf((float)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) * 0.8f / 2.0f);
            side &= ~7;
            if (side > PV_SIDE) side = PV_SIDE;
            if (side < 64) side = 64;
            ok = pv_generate(rd_cb, rw_cb, &r, smax, side, prog_cb, &im);
            if (!ok && !cancel_flag) err_code = 3;            // 3 = el generador no pudo (sin memoria o sin nada que grabar)
            r.f.close();
            if (ok && !cancel_flag) {
                if (build_image(im.nib, im.nib_size, im.w, im.h)) {
                    info_w = im.width_mm; info_h = im.height_mm; info_travel = im.travel_m; info_moves = im.moves;
                    cache_save(req_path, req_size, im);
                } else { ok = false; err_code = 4; }       // 4 = sin memoria para la imagen
            }
            pv_free(&im);
        }
    }
    if (we_set) set_sd_state(SDState::Idle);
    if (!ok && !cancel_flag) {
        grbl_sendf(CLIENT_ALL, "[MSG:Vista previa fallo E%d en %s (heap libre %u, mayor bloque %u)]\r\n", err_code, req_path,
                   (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
    }
    if (cancel_flag) { free_image(); st = PV_IDLE; }
    else st = ok ? PV_READY : PV_FAILED;
    task_alive = false;
    vTaskDelete(NULL);
}

}  // namespace

void mks_pv_cancel(void) {
    cancel_flag = true;
}

void mks_pv_release(void) {
    cancel_flag = true;
    for (int i = 0; i < 100 && task_alive; i++) vTaskDelay(10 / portTICK_PERIOD_MS);   // hasta 1 s
    if (!task_alive) { free_image(); st = PV_IDLE; }
}

void mks_pv_request(const char* sd_path, uint32_t size) {
    mks_pv_release();
    if (task_alive) { st = PV_FAILED; return; }
    free_image();
    // La lista guarda los nombres con un espacio (o "/") delante: la SD necesita "/nombre"
    while (*sd_path == ' ' || *sd_path == '/') sd_path++;
    snprintf(req_path, sizeof(req_path), "/%s", sd_path);
    req_size = size;
    prog = 0;
    err_code = 0;
    cancel_flag = false;
    if (cache_load(req_path, size)) { st = PV_READY; return; }
    st = PV_RUNNING;
    task_alive = true;
    if (xTaskCreate(pv_task, "pvTask", 8192, NULL, 1, NULL) != pdPASS) {
        task_alive = false;
        err_code = 5;   // 5 = no se pudo crear la tarea
        st = PV_FAILED;
    }
}

mks_pv_state_t mks_pv_state(void) { return st; }
uint8_t mks_pv_progress(void) { return prog; }
int mks_pv_error(void) { return err_code; }
const lv_img_dsc_t* mks_pv_image(void) { return dsc_data ? &dsc : NULL; }
void mks_pv_info(float* w, float* h, float* travel, uint32_t* moves) {
    if (w) *w = info_w;
    if (h) *h = info_h;
    if (travel) *travel = info_travel;
    if (moves) *moves = info_moves;
}
