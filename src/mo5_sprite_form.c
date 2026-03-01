/**
 * @file
 * @brief Sprite engine — form-only rendering (single color, maximum speed).
 *
 * Only the form bank is written. The color bank is set once at init
 * and never touched again.
 *
 * Draw  form  : = form_data  (single pass, direct write)
 * Clear form  : = 0x00       (single pass)
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_sprite_form.h"

// ============================================================================
// LOW-LEVEL API
// ============================================================================

void mo5_draw_sprite_form(unsigned char tx, unsigned char ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height) {
    unsigned int   offset;
    unsigned char  i, j;
    unsigned char *form_src = form_data;

    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = *form_src++;  // ← dans la boucle j
    }
}

void mo5_clear_sprite_form(unsigned char tx, unsigned char ty,
                           unsigned char width_bytes, unsigned char height) {
    unsigned int  offset;
    unsigned char i, j;

    // Single pass on form bank: form = 0x00 → all pixels display background.
    // Color bank is not touched.
    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = 0x00;
    }
}

void mo5_move_sprite_form(unsigned char old_tx, unsigned char old_ty,
                          unsigned char new_tx,  unsigned char new_ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height) {
    mo5_clear_sprite_form(old_tx, old_ty, width_bytes, height);
    mo5_draw_sprite_form(new_tx, new_ty, form_data, width_bytes, height);
}

// ============================================================================
// ACTOR API
// ============================================================================

void mo5_actor_draw_form(const MO5_Actor *actor) {
    mo5_draw_sprite_form(
        actor->pos.x, actor->pos.y,
        actor->sprite->form,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear_form(const MO5_Actor *actor) {
    mo5_clear_sprite_form(
        actor->pos.x, actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_move_form(MO5_Actor *actor, unsigned char new_x, unsigned char new_y) {
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite_form(
        actor->old_pos.x, actor->old_pos.y,
        actor->pos.x,     actor->pos.y,
        actor->sprite->form,
        actor->sprite->width_bytes, actor->sprite->height
    );
}