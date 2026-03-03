/**
 * @file
 * @brief Sprite engine — transparent rendering over a colored background.
 *
 * Draw  color : (VRAM & 0x0F) | (sprite & 0xF0)
 *               → writes only the sprite foreground
 *               → preserves the background color of the scenery
 *               → ignores the sprite background (robust against asset bugs)
 * Draw  form  : |= form_data
 * Clear form  : = 0x00  (single pass — color bank not touched)
 *
 * Asset convention: color data background bits must be 0x0 (--bg-color 0 at conversion).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_SPRITE_BG_H
#define MO5_SPRITE_BG_H

#include "mo5_sprite_types.h"

// ============================================================================
// ACTOR API (game level)
// ============================================================================

void mo5_actor_draw_bg(const MO5_Actor *actor);
void mo5_actor_clear_bg(const MO5_Actor *actor);

/**
 * Moves the actor to (new_x, new_y), preserving the colored background.
 * No-op if position is unchanged. Updates old_pos and pos automatically.
 */
void mo5_actor_move_bg(MO5_Actor *actor, unsigned char new_x, unsigned char new_y);

#endif // MO5_SPRITE_BG_H
