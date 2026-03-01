/**
 * @file
 * @brief Character classification functions (islower, isupper, isprint, ispunct).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef CTYPE_H
#define CTYPE_H

/** @return 1 if c is a lowercase letter ('a'-'z'), 0 otherwise. */
unsigned char islower(char c);

/** @return 1 if c is an uppercase letter ('A'-'Z'), 0 otherwise. */
unsigned char isupper(char c);

/** @return 1 if c is a printable character (32-126), 0 otherwise. */
unsigned char isprint(char c);

/** @return 1 if c is a punctuation character, 0 otherwise. */
unsigned char ispunct(char c);

#endif // CTYPE_H
