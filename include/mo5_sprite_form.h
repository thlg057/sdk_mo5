/**
 * @file
 * @brief Sprite engine — form-only rendering (single color, maximum speed).
 *
 * Only the form bank is written. The color bank is initialized once
 * (via mo5_video_init or mo5_fill_rect) and never touched again.
 *
 * Draw  form  : |= form_data  (single pass)
 * Clear form  : = 0x00        (single pass)
 *
 * Use when all sprite pixels share the same foreground color and the
 * background is uniform. Fastest rendering mode.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_SPRITE_FORM_H
#define MO5_SPRITE_FORM_H

#include "mo5_sprite_types.h"

// ============================================================================
// LOW-LEVEL API (raw coordinates)
// ============================================================================

void mo5_draw_sprite_form(unsigned char tx, unsigned char ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height);

void mo5_clear_sprite_form(unsigned char tx, unsigned char ty,
                           unsigned char width_bytes, unsigned char height);

void mo5_move_sprite_form(unsigned char old_tx, unsigned char old_ty,
                          unsigned char new_tx,  unsigned char new_ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height);

// ============================================================================
// ACTOR API (game level)
// ============================================================================

void mo5_actor_draw_form(const MO5_Actor *actor);
void mo5_actor_clear_form(const MO5_Actor *actor);

/**
 * Moves the actor to (new_x, new_y).
 * No-op if position is unchanged. Updates old_pos and pos automatically.
 */
void mo5_actor_move_form(MO5_Actor *actor, unsigned char new_x, unsigned char new_y);

#endif // MO5_SPRITE_FORM_H
