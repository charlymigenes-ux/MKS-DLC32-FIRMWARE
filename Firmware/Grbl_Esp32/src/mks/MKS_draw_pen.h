#ifndef mks_draw_pen_h
#define mks_draw_pen_h

#include "MKS_draw_lvgl.h"

// Variante PLOTTER_MODE: pantalla "Pluma y mover" y control de la pluma (eje Z).
// La pluma se sube y se baja con el eje Z en coordenadas de trabajo: Z=0 es "la pluma toca el papel".
// Alturas y velocidades en los ajustes $41 (arriba), $42 (abajo), $43 (velocidad de bajada) y $44 (velocidad de dibujo y de subida).
// La velocidad y el paso personalizado de los movimientos manuales de la pantalla se guardan en $45 y $46.
void mks_draw_pen(void);     // dibuja la pantalla (la limpia quien llama)
void mks_pen_update(void);   // refresca coordenadas y estado de la pluma (la llama la tarea de la interfaz)
void mks_pen_up(void);       // sube la pluma a la altura de $41
void mks_pen_down(void);     // baja la pluma a la altura de $42

// PLOTTER_OR(a, b): en la variante de plotter vale a; en las demas compilaciones, b (el codigo original, sin cambios).
// Se usa en la pantalla de archivo para cambiar "Sonda Z" por "Pluma" y "Aire" por "A origen".
void mks_pen_toggle(void);        // sube la pluma si esta abajo y la baja si esta arriba
void mks_pen_go_origin(void);     // sube la pluma y va al origen de trabajo (X0 Y0)
#ifdef PLOTTER_MODE
#define PLOTTER_OR(plotter_expr, normal_expr) (plotter_expr)
#else
#define PLOTTER_OR(plotter_expr, normal_expr) (normal_expr)
#endif

// Entre pasadas de un trabajo de la SD: sube la pluma y vuelve al origen de trabajo (X0 Y0) antes de empezar de nuevo.
// Solo en PLOTTER_MODE; en las demas compilaciones no hace nada. La llama Protocol.cpp al rebobinar el archivo.
#ifdef PLOTTER_MODE
void mks_pen_between_passes(void);
#define PEN_BETWEEN_PASSES() mks_pen_between_passes()
#else
#define PEN_BETWEEN_PASSES() ((void)0)
#endif

#endif
