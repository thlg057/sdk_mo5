/**
 * @file
 * @brief Helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_utils.h"

/**
 * Limite value entre min et max.
 * unsigned char : adapté aux coordonnées écran MO5 (0-199 max).
 */
unsigned char mo5_clamp(unsigned char value,
                        unsigned char min,
                        unsigned char max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
