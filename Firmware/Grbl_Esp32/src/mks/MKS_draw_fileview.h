#ifndef __mks_draw_fileview_h
#define __mks_draw_fileview_h

#include "MKS_draw_lvgl.h"

// Pantalla que se abre al tocar un archivo de la lista: vista previa del grabado, datos del archivo, Iniciar
// (debajo de la vista previa) y Posicionar (la pantalla de mover y poner el cero).
void mks_draw_fileview(char* fn);
void mks_fileview_poll(void);      // la llama el bucle de pantalla (MKS_UI_FileView): refresca el progreso y muestra la imagen

#endif
