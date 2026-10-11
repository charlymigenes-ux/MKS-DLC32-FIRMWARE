#ifndef __mks_preview_core_h
#define __mks_preview_core_h

// Generador de miniaturas de G-code (sin dependencias de Arduino ni de LVGL: se prueba tambien en el PC).
// Lee el archivo en un solo pase (o en dos si no trae la cabecera "; Bounds:" de LightBurn), acumula la
// potencia media por pixel y devuelve una imagen de 4 bits: 0 = fuera del recorrido, 1 = blanco ... 15 = negro.

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t w, h;          // pixeles
    float    width_mm, height_mm, travel_m;
    uint32_t moves;
    uint8_t* nib;           // 4 bits por pixel, fila a fila; cada fila ocupa (w + 1) / 2 bytes
    size_t   nib_size;
} pv_image_t;

typedef size_t (*pv_read_t)(void* ctx, uint8_t* buf, size_t n);   // devuelve los bytes leidos; 0 = fin
typedef bool (*pv_rewind_t)(void* ctx);                           // vuelve al principio del archivo
// Se llama tras cada trozo leido con el total de bytes procesados (de los dos pases). false = cancelar.
typedef bool (*pv_progress_t)(void* ctx, uint32_t bytes_done);

// max_side: lado maximo en pixeles (si falta memoria se prueba con la mitad). s_max: valor de S a plena potencia.
bool pv_generate(pv_read_t rd, pv_rewind_t rw, void* ctx, float s_max, int max_side, pv_progress_t prog, pv_image_t* out);
void pv_free(pv_image_t* img);

#endif
