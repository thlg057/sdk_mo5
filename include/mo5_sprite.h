/**
 * @file
 * @brief Sprite engine — MO5_Sprite and MO5_Actor structures, low-level and actor-level drawing API.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */


#ifndef MO5_SPRITE_H
#define MO5_SPRITE_H

#include "mo5_video.h"   // Registres, palette, dimensions écran, row_offsets

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Position en coordonnées écran MO5.
 *
 * x : en octets (1 octet = 8 pixels horizontaux)
 * y : en lignes pixels
 *
 * Attention : x N'EST PAS en pixels.
 * Pour un sprite de 16px de large centré sur 320px : x = (40 - 2) / 2 = 19
 */
typedef struct {
    int x;
    int y;
} MO5_Position;

/**
 * Données graphiques d'un sprite (ressource statique).
 *
 * Un sprite est partageable entre plusieurs acteurs — ne jamais dupliquer
 * les tableaux form/color, utiliser un pointeur vers une instance unique.
 *
 * Généré automatiquement depuis un PNG via : make convert IMG=fichier.png
 * Initialiser avec la macro SPRITE_XXX_INIT définie dans le .h généré.
 */
typedef struct {
    unsigned char *form;    // Bitmap : 1 bit/pixel (1=forme, 0=fond)
    unsigned char *color;   // Attributs couleur : 1 octet par groupe de 8 pixels (FFFFBBBB)
    int width_bytes;        // Largeur en octets (1 octet = 8 pixels)
    int height;             // Hauteur en lignes pixels
} MO5_Sprite;

/**
 * Entité du jeu : associe un sprite à une position.
 *
 * Conserve l'ancienne position (old_pos) pour permettre à mo5_actor_move
 * de ne redessiner que la zone qui a changé.
 *
 * Plusieurs acteurs peuvent pointer vers le même MO5_Sprite.
 */
typedef struct {
    const MO5_Sprite *sprite;   // Pointeur vers les données graphiques (partagé)
    MO5_Position      pos;      // Position courante
    MO5_Position      old_pos;  // Position précédente (gérée par mo5_actor_move)
} MO5_Actor;

// ============================================================================
// API BAS NIVEAU (coordonnées brutes)
// Pour les cas spéciaux (HUD, tiles de décor...).
// Préférer l'API Actor pour les entités de jeu.
// ============================================================================

/**
 * Dessine un sprite à (tx, ty).
 * Optimisé : 1 seul switch PRC pour toutes les couleurs,
 *            1 seul switch PRC pour toutes les formes.
 */
void mo5_draw_sprite(int tx, int ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height);

/**
 * Efface un sprite à (tx, ty).
 * Optimisé : 1 seul switch PRC par passe.
 */
void mo5_clear_sprite(int tx, int ty, int width_bytes, int height);

/**
 * Déplace un sprite de (old_tx, old_ty) vers (new_tx, new_ty).
 *
 * Optimisé : ne cleare que la zone hors recouvrement entre l'ancienne
 * et la nouvelle position (~25% d'écritures VRAM économisées pour un
 * déplacement standard de 1 octet ou 8 lignes).
 *
 * Fallback automatique sur clear+draw si aucun recouvrement
 * (déplacement supérieur à la taille du sprite).
 */
void mo5_move_sprite(int old_tx, int old_ty,
                     int new_tx, int new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height);

// ============================================================================
// API ACTOR (niveau jeu)
// ============================================================================

/**
 * Dessine l'acteur à sa position courante (pos).
 * À utiliser pour le premier affichage uniquement.
 * Dans la boucle de jeu, utiliser mo5_actor_move.
 */
void mo5_actor_draw(const MO5_Actor *actor);

/**
 * Efface l'acteur à sa position courante (pos).
 */
void mo5_actor_clear(const MO5_Actor *actor);

/**
 * Déplace l'acteur vers (new_x, new_y) de façon optimisée.
 *
 * - No-op si la position est identique
 * - Met à jour old_pos et pos automatiquement
 * - Appelle mo5_move_sprite (rendu différentiel)
 */
void mo5_actor_move(MO5_Actor *actor, int new_x, int new_y);

/**
 * Limite la position courante (pos) de l'acteur aux bords de l'écran.
 * Tient compte de la taille du sprite.
 *
 * À appeler sur les coordonnées calculées AVANT mo5_actor_move.
 */
void mo5_actor_clamp(MO5_Actor *actor);

#endif // MO5_SPRITE_H
