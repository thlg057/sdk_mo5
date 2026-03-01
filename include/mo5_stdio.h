/**
 * @file
 * @brief High-level string and character I/O (fgets, fputs, puts, clrscr).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_STDIO_H
#define MO5_STDIO_H

#include "mo5_defs.h"

/** Maps getchar to mo5_getchar. Blocking: waits until a character is available. */
#define getchar mo5_getchar

/**
 * Reads a string from input into a buffer, with echo and backspace support.
 *
 * @param buffer      Destination buffer.
 * @param max_length  Maximum number of characters to read (excluding null terminator).
 * @return            Number of characters read.
 */
unsigned char fgets(char *buffer, unsigned char max_length);

/**
 * Writes a null-terminated string to the output.
 *
 * @param s  String to write.
 */
void fputs(const char *s);

/**
 * Writes a null-terminated string followed by a newline.
 *
 * @param s  String to write.
 */
void puts(const char *s);

/** Clears the screen and resets the cursor position. */
void clrscr(void);

#endif // MO5_STDIO_H
