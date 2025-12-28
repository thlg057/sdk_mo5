#ifndef CTYPE_H
#define CTYPE_H

#include "mo5_defs.h"

/**
 * @brief Checks if the character is a lowercase letter ('a' to 'z').
 * @param c Character to test.
 * @return Boolean value: TRUE if c is lowercase, FALSE otherwise.
 */
int islower(char c);

/**
 * @brief Checks if the character is an uppercase letter ('A' to 'Z').
 * @param c Character to test.
 * @return Boolean value: TRUE if c is uppercase, FALSE otherwise.
 */
int isupper(char c);

/**
 * @brief Checks if the character is printable (excluding control characters).
 * @param c Character to test.
 * @return Boolean value: TRUE if c is printable, FALSE otherwise.
 */
int isprint(char c);

/**
 * @brief Checks if the character is a punctuation character.
 * @param c Character to test.
 * @return Boolean value: TRUE if c is punctuation, FALSE otherwise.
 */
int ispunct(char c);

#endif