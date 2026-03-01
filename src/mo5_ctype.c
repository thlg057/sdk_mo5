/**
 * @file
 * @brief Implementation of character classification functions.
 *
 * Retourne unsigned char au lieu de int — 0 ou 1 suffit,
 * évite les opérations 16 bits sur 6809.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_ctype.h"

unsigned char islower(char c) {
    return (c >= 'a' && c <= 'z') ? 1 : 0;
}

unsigned char isupper(char c) {
    return (c >= 'A' && c <= 'Z') ? 1 : 0;
}

unsigned char isprint(char c) {
    return (c >= 32 && c <= 126) ? 1 : 0;
}

unsigned char ispunct(char c) {
    return ((c >= 33 && c <= 47)  ||
            (c >= 58 && c <= 64)  ||
            (c >= 91 && c <= 96)  ||
            (c >= 123 && c <= 126)) ? 1 : 0;
}
