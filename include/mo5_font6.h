/**
 * @file mo5_font6.h
 * @brief Affichage de texte en mode graphique MO5 — police arcade 8x6 pixels.
 *
 * Chaque caractere occupe 1 octet de large x 6 lignes pixel.
 * Le fond du decor est preserve : seuls les pixels forme sont ecrits.
 * Permet d'afficher 33 lignes de texte sur les 200 lignes de l'ecran
 * (contre 25 avec la police 8x8).
 *
 * Usage :
 *   mo5_font6_puts(2, 12, "SCORE 000100", C_YELLOW);
 *   mo5_font6_clear(2, 12, 12);
 *
 * Coordonnees :
 *   tx  en octets  (1 unite = 8 pixels horizontaux, 0..39)
 *   ty  en pixels  (0..199)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_FONT6_H
#define MO5_FONT6_H

#include <mo5_defs.h>

/**
 * Dessine une chaine de caracteres 8x6 en preservant le fond.
 * Retour a la ligne automatique en fin de ligne (tx >= 40).
 *
 * @param tx         Position horizontale en octets (0..39)
 * @param ty         Position verticale en pixels (0..199)
 * @param s          Chaine terminee par \0
 * @param fg_color   Couleur de forme (0..15, constantes C_xxx de mo5_defs.h)
 */
void mo5_font6_puts(unsigned char tx, unsigned char ty,
                    const char *s, unsigned char fg_color);

/**
 * Efface une zone de texte (remplace par des espaces).
 *
 * @param tx         Position horizontale en octets (0..39)
 * @param ty         Position verticale en pixels (0..199)
 * @param len        Nombre de caracteres a effacer
 */
void mo5_font6_clear(unsigned char tx, unsigned char ty, unsigned char len);

#endif /* MO5_FONT6_H */
