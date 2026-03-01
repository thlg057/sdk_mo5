/**
 * @file
 * @brief Dirty Rectangle sprite engine — implémentation.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_actor_dr.h"

// ============================================================================
// FONCTIONS INTERNES
// ============================================================================

/**
 * Sauvegarde la zone VRAM (couleur + forme) à (tx, ty) dans les buffers.
 */
static void dr_save(MO5_Actor_DR *actor) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char  w = actor->sprite->width_bytes;
    unsigned char  h = actor->sprite->height;
    unsigned char  x = actor->pos.x;
    unsigned char  y = actor->pos.y;
    unsigned char *dst_color = actor->save_color;
    unsigned char *dst_form  = actor->save_form;

    *PRC &= ~0x01;  // banque couleur
    for (i = 0; i < h; i++) {
        offset = row_offsets[y + i] + x;
        for (j = 0; j < w; j++)
            *dst_color++ = VRAM[offset + j];
    }

    *PRC |= 0x01;   // banque forme
    for (i = 0; i < h; i++) {
        offset = row_offsets[y + i] + x;
        for (j = 0; j < w; j++)
            *dst_form++ = VRAM[offset + j];
    }
}

/**
 * Restaure la zone VRAM (couleur + forme) à (tx, ty) depuis les buffers.
 */
static void dr_restore(MO5_Actor_DR *actor) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char  w = actor->sprite->width_bytes;
    unsigned char  h = actor->sprite->height;
    unsigned char  x = actor->pos.x;
    unsigned char  y = actor->pos.y;
    unsigned char *src_color = actor->save_color;
    unsigned char *src_form  = actor->save_form;

    *PRC &= ~0x01;  // banque couleur
    for (i = 0; i < h; i++) {
        offset = row_offsets[y + i] + x;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = *src_color++;
    }

    *PRC |= 0x01;   // banque forme
    for (i = 0; i < h; i++) {
        offset = row_offsets[y + i] + x;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = *src_form++;
    }
}

/**
 * Dessine le sprite à la position courante (transparence via fg).
 */
static void dr_draw(MO5_Actor_DR *actor) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char  w = actor->sprite->width_bytes;
    unsigned char  h = actor->sprite->height;
    unsigned char  x = actor->pos.x;
    unsigned char  y = actor->pos.y;
    unsigned char *color_src = actor->sprite->color;
    unsigned char *form_src  = actor->sprite->form;
    unsigned char  fg;

    for (i = 0; i < h; i++) {
        offset = row_offsets[y + i] + x;
        for (j = 0; j < w; j++) {
            fg = *color_src & 0xF0;
            if (fg) {
                // Banque couleur : bg du décor conservé, fg du sprite injecté
                *PRC &= ~0x01;
                VRAM[offset + j] = (VRAM[offset + j] & 0x0F) | fg;
                // Banque forme : bits du sprite posés sans effacer le décor
                *PRC |= 0x01;
                VRAM[offset + j] |= *form_src;
            }
            color_src++;
            form_src++;
        }
    }
}

// ============================================================================
// API PUBLIQUE
// ============================================================================

void mo5_actor_dr_init(MO5_Actor_DR *actor, const MO5_Sprite *sprite,
                       unsigned char x, unsigned char y) {
    actor->sprite = sprite;
    actor->pos.x  = x;
    actor->pos.y  = y;
    dr_save(actor);
    dr_draw(actor);
}

void mo5_actor_dr_restore(MO5_Actor_DR *actor) {
    dr_restore(actor);
}

void mo5_actor_dr_save_draw(MO5_Actor_DR *actor) {
    dr_save(actor);
    dr_draw(actor);
}

void mo5_actor_dr_move(MO5_Actor_DR *actor, unsigned char x, unsigned char y) {
    actor->pos.x = x;
    actor->pos.y = y;
}
