/**
 * @file
 * @brief Video hardware abstraction — VRAM registers, 16-color palette, screen dimensions, VBL sync.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_VIDEO_H
#define MO5_VIDEO_H

#include <cmoc.h>

// ============================================================================
// HARDWARE REGISTERS
// ============================================================================

#define PRC       ((unsigned char *)0xA7C0)  // VRAM bank select (bit0: 0=color, 1=form)
#define VIDEO_REG ((unsigned char *)0xA7E7)  // Video mode register
#define VRAM      ((unsigned char *)0x0000)  // Video memory base address
#define VBL_REG   ((unsigned char *)0xA7E7)  // VBL status register (bit7=1 during blanking)
#define VBL_BIT   0x80

// ============================================================================
// 16-COLOR PALETTE
// ============================================================================

#define C_BLACK        0
#define C_RED          1
#define C_GREEN        2
#define C_YELLOW       3
#define C_BLUE         4
#define C_MAGENTA      5
#define C_CYAN         6
#define C_WHITE        7
#define C_GRAY         8
#define C_LIGHT_RED    9
#define C_LIGHT_GREEN  10
#define C_LIGHT_YELLOW 11
#define C_LIGHT_BLUE   12
#define C_PURPLE       13
#define C_LIGHT_CYAN   14
#define C_ORANGE       15

/** Builds a MO5 color byte in FFFFBBBB format (fg in bits 4-7, bg in bits 0-3). */
#define COLOR(bg, fg) (unsigned char)(((bg) & 0x0F) | (((fg) & 0x0F) << 4))

// ============================================================================
// SCREEN DIMENSIONS
// ============================================================================

#define SCREEN_WIDTH_BYTES  40    // 320 pixels / 8 pixels per byte
#define SCREEN_HEIGHT       200   // Pixel rows
#define SCREEN_SIZE_BYTES   (SCREEN_WIDTH_BYTES * SCREEN_HEIGHT)  // 8000 bytes

// ============================================================================
// ROW OFFSET TABLE
// Pre-computed in mo5_video_init() to avoid per-scanline multiplication.
// Usage: offset = row_offsets[y] + x
// ============================================================================

extern unsigned int row_offsets[SCREEN_HEIGHT];

// ============================================================================
// FUNCTIONS
// ============================================================================

/**
 * Initializes MO5 graphics mode.
 * - Pre-computes the row_offsets table
 * - Enables bitmap mode
 * - Fills the color bank with @p color
 * - Clears the form bank (all zeros)
 *
 * Must be called ONCE at startup, before any drawing.
 *
 * @param color  Initial background color — use COLOR(bg, fg)
 */
void mo5_video_init(unsigned char color);

/**
 * Waits for the next vertical blanking interval (VBL, 50 Hz PAL).
 *
 * Call at the start of the main loop to synchronize the game
 * with the display refresh rate. Guarantees ~20,000 CPU cycles per frame
 * and prevents tearing.
 *
 * Typical usage:
 *   while (1) {
 *       mo5_wait_vbl();
 *       update_logic();
 *       draw();
 *   }
 */
void mo5_wait_vbl(void);

/**
 * Fills the entire screen (color and form banks) with the given color.
 *
 * @param color  Color byte — use COLOR(bg, fg)
 */
void mo5_clear_screen(unsigned char color);

/**
 * Fills a screen rectangle with a uniform color.
 * Writes to both banks (color and form).
 *
 * @param tx     X position in bytes (0-39)
 * @param ty     Y position in pixel rows (0-199)
 * @param w      Width in bytes
 * @param h      Height in pixel rows
 * @param color  Color byte — use COLOR(bg, fg)
 */
void mo5_fill_rect(unsigned char tx, unsigned char ty,
                   unsigned char w,  unsigned char h,
                   unsigned char color);

#endif // MO5_VIDEO_H
