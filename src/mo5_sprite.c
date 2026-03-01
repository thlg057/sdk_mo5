/**
 * @file
 * @brief Sprite rendering and actor movement with optimized differential VRAM writes.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_sprite.h"

// ============================================================================
// API BAS NIVEAU
// ============================================================================

void mo5_draw_sprite(unsigned char tx, unsigned char ty,
                     unsigned char *form_data, unsigned char *color_data,
                     unsigned char width_bytes, unsigned char height) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char *src;

    *PRC &= ~0x01;
    src = color_data;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = *src++;
    }

    *PRC |= 0x01;
    src = form_data;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = *src++;
    }
}

void mo5_clear_sprite(unsigned char tx, unsigned char ty,
                      unsigned char width_bytes, unsigned char height) {
    unsigned int  offset;
    unsigned char i, j;

    *PRC &= ~0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = 0x00;
    }

    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = 0x00;
    }
}

/**
 * Déplace un sprite avec rendu différentiel.
 * Ne cleare que la zone hors recouvrement (~25% d'écritures économisées).
 * dx/dy en signed char : suffisant pour ±127px, fallback clear+draw au-delà.
 */
void mo5_move_sprite(unsigned char old_tx, unsigned char old_ty,
                     unsigned char new_tx,  unsigned char new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     unsigned char width_bytes, unsigned char height) {
    signed char    dx = (signed char)(new_tx - old_tx);
    signed char    dy = (signed char)(new_ty - old_ty);
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char *src;
    unsigned char  adx = dx < 0 ? -dx : dx;
    unsigned char  ady = dy < 0 ? -dy : dy;
    unsigned char  overlap_w, overlap_h;

    if (adx >= width_bytes || ady >= height) {
        mo5_clear_sprite(old_tx, old_ty, width_bytes, height);
        mo5_draw_sprite(new_tx, new_ty, form_data, color_data, width_bytes, height);
        return;
    }

    overlap_w = width_bytes - adx;
    overlap_h = height      - ady;

    // -------------------------------------------------------------------------
    // PASSE COULEUR
    // -------------------------------------------------------------------------
    *PRC &= ~0x01;

    if (dx != 0) {
        unsigned char clear_col = (dx > 0) ? old_tx : (old_tx + width_bytes - adx);
        for (i = 0; i < height; i++) {
            offset = row_offsets[old_ty + i] + clear_col;
            for (j = 0; j < adx; j++)
                VRAM[offset + j] = 0x00;
        }
    }
    if (dy != 0) {
        unsigned char clear_row = (dy > 0) ? old_ty : (old_ty + height - ady);
        for (i = 0; i < ady; i++) {
            offset = row_offsets[clear_row + i] + old_tx;
            for (j = 0; j < width_bytes; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    src = color_data;
    for (i = 0; i < height; i++) {
        offset = row_offsets[new_ty + i] + new_tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = *src++;
    }

    // -------------------------------------------------------------------------
    // PASSE FORME
    // -------------------------------------------------------------------------
    *PRC |= 0x01;

    if (dx != 0) {
        unsigned char clear_col = (dx > 0) ? old_tx : (old_tx + width_bytes - adx);
        for (i = 0; i < height; i++) {
            offset = row_offsets[old_ty + i] + clear_col;
            for (j = 0; j < adx; j++)
                VRAM[offset + j] = 0x00;
        }
    }
    if (dy != 0) {
        unsigned char clear_row = (dy > 0) ? old_ty : (old_ty + height - ady);
        for (i = 0; i < ady; i++) {
            offset = row_offsets[clear_row + i] + old_tx;
            for (j = 0; j < width_bytes; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    src = form_data;
    for (i = 0; i < height; i++) {
        offset = row_offsets[new_ty + i] + new_tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = *src++;
    }
}

// ============================================================================
// API ACTOR
// ============================================================================

void mo5_actor_draw(const MO5_Actor *actor) {
    mo5_draw_sprite(
        actor->pos.x, actor->pos.y,
        actor->sprite->form, actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear(const MO5_Actor *actor) {
    mo5_clear_sprite(
        actor->pos.x, actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_move(MO5_Actor *actor, unsigned char new_x, unsigned char new_y) {
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite(
        actor->old_pos.x, actor->old_pos.y,
        actor->pos.x,     actor->pos.y,
        actor->sprite->form, actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clamp(MO5_Actor *actor) {
    unsigned char max_x = SCREEN_WIDTH_BYTES - actor->sprite->width_bytes;
    unsigned char max_y = SCREEN_HEIGHT      - actor->sprite->height;

    if (actor->pos.x > max_x) actor->pos.x = max_x;
    if (actor->pos.y > max_y) actor->pos.y = max_y;
}
