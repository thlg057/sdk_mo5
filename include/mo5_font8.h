/**
 * @file mo5_font8.h
 * @brief Affichage de texte en mode graphique MO5 — police arcade 8x8 pixels.
 *
 * Chaque caractere occupe 1 octet de large x 8 lignes pixel.
 * Le fond du decor est preserve : seuls les pixels forme sont ecrits.
 *
 * Usage :
 *   mo5_font8_puts(2, 16, "SCORE 000100", C_YELLOW);
 *   mo5_font8_clear(2, 16, 12);
 *
 * Coordonnees :
 *   tx  en octets  (1 unite = 8 pixels horizontaux, 0..39)
 *   ty  en pixels  (0..199)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_FONT8_H
#define MO5_FONT8_H

#include <mo5_defs.h>

/**
 * Dessine une chaine de caracteres 8x8 en preservant le fond.
 * Retour a la ligne automatique en fin de ligne (tx >= 40).
 *
 * @param tx         Position horizontale en octets (0..39)
 * @param ty         Position verticale en pixels (0..199)
 * @param s          Chaine terminee par \0
 * @param fg_color   Couleur de forme (0..15, constantes C_xxx de mo5_defs.h)
 */
void mo5_font8_puts(unsigned char tx, unsigned char ty,
                    const char *s, unsigned char fg_color);

/**
 * Efface une zone de texte (remplace par des espaces).
 *
 * @param tx         Position horizontale en octets (0..39)
 * @param ty         Position verticale en pixels (0..199)
 * @param len        Nombre de caracteres a effacer
 */
void mo5_font8_clear(unsigned char tx, unsigned char ty, unsigned char len);

#endif /* MO5_FONT8_H */
