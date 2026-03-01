/**
 * @file
 * @brief Utility helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_UTILS_H
#define MO5_UTILS_H

/**
 * Clamps @p value to the range [@p min, @p max].
 * Uses unsigned char — suitable for MO5 screen coordinates (0-199 max).
 *
 * @param value  Value to clamp.
 * @param min    Lower bound (inclusive).
 * @param max    Upper bound (inclusive).
 * @return       Clamped value in [min, max].
 */
unsigned char mo5_clamp(unsigned char value,
                        unsigned char min,
                        unsigned char max);

#endif // MO5_UTILS_H
