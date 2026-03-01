/**
 * @file
 * @brief Dirty Rectangle sprite engine — per-sprite save/draw/restore.
 *
 * Each actor saves the VRAM area it occupies before drawing,
 * and restores it at the start of the next frame.
 *
 * This enables:
 *   - Arbitrary colored backgrounds (color + form) without artifacts
 *   - Correct layering of multiple overlapping sprites
 *   - Automatic transparency (exact background is restored)
 *
 * Required per-frame sequence:
 *   1. mo5_actor_dr_restore() on all sprites (REVERSE draw order)
 *   2. Game logic + mo5_actor_dr_move()
 *   3. mo5_actor_dr_save_draw() on all sprites (normal draw order)
 *
 * Constraint: sprite must fit within MO5_DR_MAX_WIDTH x MO5_DR_MAX_HEIGHT pixels.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_ACTOR_DR_H
#define MO5_ACTOR_DR_H

#include "mo5_sprite_bg.h"

// ============================================================================
// CONSTANTS
// ============================================================================

#define MO5_DR_MAX_WIDTH   4    // 32 pixels = 4 bytes
#define MO5_DR_MAX_HEIGHT  32
#define MO5_DR_SAVE_SIZE   (MO5_DR_MAX_WIDTH * MO5_DR_MAX_HEIGHT)  // 128 bytes

// ============================================================================
// STRUCTURE
// ============================================================================

/**
 * Dirty Rectangle actor.
 *
 * save_color / save_form: static buffers holding the VRAM content saved
 * beneath the sprite before drawing. Restored at the start of each frame.
 *
 * Must be initialized with mo5_actor_dr_init() before any use.
 */
typedef struct {
    const MO5_Sprite *sprite;
    MO5_Position      pos;
    unsigned char     save_color[MO5_DR_SAVE_SIZE];
    unsigned char     save_form[MO5_DR_SAVE_SIZE];
} MO5_Actor_DR;

// ============================================================================
// API
// ============================================================================

/**
 * Initializes the actor: sets the sprite, positions at (x, y),
 * saves the VRAM, then draws the sprite.
 * Call once before the game loop.
 */
void mo5_actor_dr_init(MO5_Actor_DR *actor, const MO5_Sprite *sprite,
                       unsigned char x, unsigned char y);

/**
 * Restores the saved VRAM at the current position.
 * Call at the start of each frame, in REVERSE draw order
 * to correctly handle sprite layering.
 */
void mo5_actor_dr_restore(MO5_Actor_DR *actor);

/**
 * Saves the VRAM at the current position, then draws the sprite.
 * Call at the end of each frame, in normal draw order.
 */
void mo5_actor_dr_save_draw(MO5_Actor_DR *actor);

/**
 * Updates the actor position.
 * Takes effect on the next mo5_actor_dr_save_draw() call.
 * Clamping must be handled by the caller before calling this function.
 */
void mo5_actor_dr_move(MO5_Actor_DR *actor, unsigned char x, unsigned char y);

#endif // MO5_ACTOR_DR_H
