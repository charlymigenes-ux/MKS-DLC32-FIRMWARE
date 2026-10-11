#ifndef __mks_preview_h
#define __mks_preview_h

#include "lvgl.h"
#include <stdint.h>

// Vista previa de un archivo de la SD para la pantalla. La genera una tarea aparte (MKS_preview_core) y la
// guarda en SPIFFS; la segunda vez que se pide el mismo archivo sale al instante.

typedef enum { PV_IDLE = 0, PV_RUNNING, PV_READY, PV_FAILED } mks_pv_state_t;

// Pide la miniatura de un archivo de la SD (ruta como "/archivo.gc") de size bytes. Si ya hay una en cache,
// queda lista en el acto; si no, arranca la tarea. Cancela la peticion anterior si seguia en marcha.
void mks_pv_request(const char* sd_path, uint32_t size);
void mks_pv_cancel(void);                 // al salir de la pantalla: la tarea termina sola y libera todo
void mks_pv_release(void);                // libera la imagen (al salir de la pantalla)
mks_pv_state_t mks_pv_state(void);
int mks_pv_error(void);                   // motivo del ultimo fallo (0 = ninguno)
uint8_t mks_pv_progress(void);            // 0..100 mientras PV_RUNNING
const lv_img_dsc_t* mks_pv_image(void);   // valida solo en PV_READY
void mks_pv_info(float* width_mm, float* height_mm, float* travel_m, uint32_t* moves);

#endif
