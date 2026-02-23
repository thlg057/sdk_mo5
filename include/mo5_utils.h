/**
 * @file
 * @brief Helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_UTILS_H
#define MO5_UTILS_H

/**
 * Clamps @p value to the range [@p min, @p max].
 *
 * Returns @p min if @p value < @p min,
 *         @p max if @p value > @p max,
 *         @p value otherwise.
 *
 * @param value  Value to clamp.
 * @param min    Lower bound (inclusive).
 * @param max    Upper bound (inclusive).
 * @return       Clamped value in [min, max].
 */
int mo5_clamp(int value, int min, int max);

#endif