/**
 * @file
 * @brief Helpers.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_utils.h"

int mo5_clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
