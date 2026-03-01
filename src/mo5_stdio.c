/**
 * @file
 * @brief String I/O with echo and backspace support.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_defs.h"
#include <cmoc.h>
#include "mo5_stdio.h"
#include "mo5_ctype.h"

unsigned char fgets(char *buffer, unsigned char max_length) {
    unsigned char pos = 0;
    char          ch;

    while (1) {
        ch = mo5_getchar();

        if (ch == MO5_ENTER_CHAR) {
            break;
        }

        if (ch == MO5_BACKSPACE_CHAR) {
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                mo5_putchar(MO5_BACKSPACE_CHAR);
                mo5_putchar(MO5_SPACE_CHAR);
                mo5_putchar(MO5_BACKSPACE_CHAR);
            }
            continue;
        }

        if (isprint(ch) && pos < max_length) {
            buffer[pos] = ch;
            pos++;
            mo5_putchar(ch);
        }
    }

    buffer[pos] = '\0';
    return pos;
}

void fputs(const char *str) {
    while (*str)
        putchar(*str++);
}

void puts(const char *str) {
    fputs(str);
    mo5_newline();
}

void clrscr(void) {
    putchar(MO5_CLEAR_SCREEN);
}
