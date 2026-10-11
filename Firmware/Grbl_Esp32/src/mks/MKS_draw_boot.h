#ifndef __MKS_DRAW_BOOT_H
#define __MKS_DRAW_BOOT_H

// Pantalla de arranque: el logo se ve 4 s con la version abajo a la derecha. Si se mantiene pulsada,
// muestra el QR del repositorio y espera a que se toque el QR para continuar.
void mks_boot_begin(void);            // dibuja el logo y arma el contador
bool mks_boot_tick(void);             // llamar cada 5 ms; true cuando hay que pasar a la pantalla principal
bool mks_boot_backlight_due(void);    // true UNA vez, cuando toca encender la luz de fondo (a los 0,5 s, o antes si se pulsa)

#endif
