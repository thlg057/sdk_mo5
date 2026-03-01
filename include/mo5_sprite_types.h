/**
 * @file
 * @brief Shared sprite engine types — MO5_Position, MO5_Sprite, MO5_Actor.
 *
 * Included automatically by mo5_sprite.h and mo5_sprite_bg.h.
 * Do not include directly in game code.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_SPRITE_TYPES_H
#define MO5_SPRITE_TYPES_H

#include "mo5_video.h"

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Screen coordinates for MO5.
 *
 * x : in bytes (0-39, 1 byte = 8 horizontal pixels)
 * y : in pixel rows (0-199)
 *
 * Warning: x is NOT in pixels.
 * To center a 16px-wide sprite on 320px: x = (40 - 2) / 2 = 19
 */
typedef struct {
    unsigned char x;   // 0-39
    unsigned char y;   // 0-199
} MO5_Position;

/**
 * Sprite graphic data (static resource).
 *
 * A sprite can be shared between multiple actors — never duplicate
 * the form/color arrays, use a pointer to a single instance.
 *
 * Auto-generated from PNG via: make convert IMG=file.png
 * Initialize with the SPRITE_XXX_INIT macro defined in the generated .h.
 */
typedef struct {
    unsigned char *form;        // Bitmap: 1 bit/pixel (1=shape, 0=background)
    unsigned char *color;       // Color attributes: 1 byte per 8-pixel group (FFFFBBBB)
    unsigned char  width_bytes; // Width in bytes (1-40)
    unsigned char  height;      // Height in pixel rows (1-200)
} MO5_Sprite;

/**
 * Game entity: associates a sprite with a position.
 *
 * Stores the previous position (old_pos) to allow mo5_actor_move
 * to redraw only the changed area.
 *
 * Multiple actors can point to the same MO5_Sprite.
 */
typedef struct {
    const MO5_Sprite *sprite;   // Pointer to graphic data (shared)
    MO5_Position      pos;      // Current position
    MO5_Position      old_pos;  // Previous position (managed by mo5_actor_move)
} MO5_Actor;

// ============================================================================
// GENERIC ACTOR FUNCTIONS
// ============================================================================

/**
 * Clamps the actor's current position (pos) to the screen boundaries,
 * taking the sprite size into account.
 *
 * Generic: compatible with mo5_sprite.h and mo5_sprite_bg.h.
 * Call on computed coordinates BEFORE mo5_actor_move / mo5_actor_move_bg.
 */
void mo5_actor_clamp(MO5_Actor *actor);

#endif // MO5_SPRITE_TYPES_H
