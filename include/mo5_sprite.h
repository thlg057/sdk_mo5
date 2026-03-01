/**
 * @file
 * @brief Sprite engine — opaque rendering on solid background (black).
 *
 * Writes directly to both the color and form VRAM banks.
 * The sprite background overwrites existing VRAM content.
 *
 * Use when the background is solid (initialized to 0x00 by mo5_video_init).
 * For sprites over a colored background, use mo5_sprite_bg.h.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_SPRITE_H
#define MO5_SPRITE_H

#include "mo5_sprite_types.h"

// ============================================================================
// LOW-LEVEL API (raw coordinates)
// ============================================================================

void mo5_draw_sprite(unsigned char tx, unsigned char ty,
                     unsigned char *form_data, unsigned char *color_data,
                     unsigned char width_bytes, unsigned char height);

void mo5_clear_sprite(unsigned char tx, unsigned char ty,
                      unsigned char width_bytes, unsigned char height);

/**
 * Moves a sprite from (old_tx, old_ty) to (new_tx, new_ty).
 * Only clears the non-overlapping area (~25% fewer VRAM writes).
 * Falls back to clear+draw when there is no overlap.
 */
void mo5_move_sprite(unsigned char old_tx, unsigned char old_ty,
                     unsigned char new_tx,  unsigned char new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     unsigned char width_bytes, unsigned char height);

// ============================================================================
// ACTOR API (game level)
// ============================================================================

void mo5_actor_draw(const MO5_Actor *actor);
void mo5_actor_clear(const MO5_Actor *actor);

/**
 * Moves the actor to (new_x, new_y).
 * No-op if position is unchanged. Updates old_pos and pos automatically.
 */
void mo5_actor_move(MO5_Actor *actor, unsigned char new_x, unsigned char new_y);

#endif // MO5_SPRITE_H
