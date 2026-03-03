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

void mo5_draw_sprite_form(unsigned char tx,          unsigned char ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height)
{
    unsigned char *row      = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    unsigned char  rows_left = height;

    *PRC |= 0x01;
    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = width_bytes;
        while (col--) *p++ = *form_data++;
        row += SCREEN_WIDTH_BYTES;
    }
}

void mo5_clear_sprite_form(unsigned char tx,          unsigned char ty,
                           unsigned char width_bytes, unsigned char height)
{
    unsigned char *row      = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    unsigned char  rows_left = height;

    *PRC |= 0x01;
    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = width_bytes;
        while (col--) *p++ = 0x00;
        row += SCREEN_WIDTH_BYTES;
    }
}

void mo5_move_sprite_form(unsigned char old_tx,      unsigned char old_ty,
                          unsigned char new_tx,       unsigned char new_ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height)
{
    mo5_clear_sprite_form(old_tx, old_ty, width_bytes, height);
    mo5_draw_sprite_form (new_tx, new_ty, form_data, width_bytes, height);
}

// ============================================================================
// ACTOR API
// ============================================================================

void mo5_actor_draw_form(const MO5_Actor *actor)
{
    mo5_draw_sprite_form(
        actor->pos.x,               actor->pos.y,
        actor->sprite->form,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear_form(const MO5_Actor *actor)
{
    mo5_clear_sprite_form(
        actor->pos.x,               actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_move_form(MO5_Actor *actor, unsigned char new_x, unsigned char new_y)
{
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite_form(
        actor->old_pos.x,           actor->old_pos.y,
        actor->pos.x,               actor->pos.y,
        actor->sprite->form,
        actor->sprite->width_bytes, actor->sprite->height
    );
}