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

/* Character input/output functions */

/**
 * @brief Reads a single character from the input.
 * @return The character read.
 * @note Blocking call: waits until a character is available.
 * @details public mapping, expands char mo5_getchar()
 */
#define getchar mo5_getchar

/* String input functions */

/**
 * @brief Reads a string from input into a buffer.
 * @param buffer Destination buffer to store input string.
 * @param max_length Maximum number of characters to read (including null terminator).
 * @return Number of characters read, or -1 on error.
 */
int fgets(char* buffer, int max_length);

/* String output functions */

/**
 * @brief Writes a null-terminated string to the output.
 * @param s String to write.
 */
void fputs(const char* s);

/**
 * @brief Writes a null-terminated string followed by a newline.
 * @param s String to write.
 */
void puts(const char* s);

/* Screen control functions */

/**
 * @brief Clears the screen and resets cursor position.
 */
void clrscr(void);

#endif