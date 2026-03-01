/**
 * @file
 * @brief Sprite rendering on colored background — transparent draw.
 *
 * Principe :
 *   Draw  : si fg == 0 → groupe transparent, on ne touche rien.
 *           si fg != 0 → (VRAM & 0x0F) | fg  sur banque couleur
 *                        VRAM |= form          sur banque forme
 *   Clear : forme = 0x00 (passe unique — banque couleur non touchée)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_sprite_bg.h"

// ============================================================================
// API BAS NIVEAU
// ============================================================================

void mo5_draw_sprite_bg(unsigned char tx,         unsigned char ty,
                        unsigned char *form_data,  unsigned char *color_data,
                        unsigned char width_bytes, unsigned char height) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char  fg;
    unsigned char *color_src = color_data;
    unsigned char *form_src  = form_data;

    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {

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

void mo5_clear_sprite_bg(unsigned char tx,         unsigned char ty,
                         unsigned char width_bytes, unsigned char height) {
    unsigned int  offset;
    unsigned char i, j;

    // Passe unique sur la banque forme : forme = 0x00 → tous les pixels
    // affichent le background. La banque couleur n'est pas touchée.
    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = 0x00;
    }
}

void mo5_move_sprite_bg(unsigned char old_tx,      unsigned char old_ty,
                        unsigned char new_tx,       unsigned char new_ty,
                        unsigned char *form_data,   unsigned char *color_data,
                        unsigned char width_bytes,  unsigned char height) {
    mo5_clear_sprite_bg(old_tx, old_ty, width_bytes, height);
    mo5_draw_sprite_bg(new_tx, new_ty, form_data, color_data, width_bytes, height);
}

// ============================================================================
// API ACTOR
// ============================================================================

void mo5_actor_draw_bg(const MO5_Actor *actor) {
    mo5_draw_sprite_bg(
        actor->pos.x,              actor->pos.y,
        actor->sprite->form,       actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear_bg(const MO5_Actor *actor) {
    mo5_clear_sprite_bg(
        actor->pos.x,              actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_move_bg(MO5_Actor *actor, unsigned char new_x, unsigned char new_y) {
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite_bg(
        actor->old_pos.x,           actor->old_pos.y,
        actor->pos.x,               actor->pos.y,
        actor->sprite->form,        actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}
