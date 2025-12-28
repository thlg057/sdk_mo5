/*
 * MO5 Mini-stdio
 * -------------------------
 * Basic input/output library for Thomson MO5.
 * Provides simple character and string I/O functions.
 *
 * Features:
 * - putchar, fputs, puts       : Output characters and strings
 * - print integers (signed/unsigned/hex) : Optional integer printing routines
 * - getchar, fgets             : Input characters and strings
 * - Generic print and printnl macros for convenience
 *
 * Note: This is a minimal implementation intended for educational or
 * embedded usage on the MO5 platform.
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