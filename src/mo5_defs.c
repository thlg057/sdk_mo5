/**
 * @file
 * @brief Implementation of low-level I/O via SWI (getchar, putchar, newline, key wait).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */


#include "mo5_defs.h"

char mo5_getchar(void)
{
    asm {
        swi
        fcb $0A
    }
}

//int code = (int)ch;
void mo5_putchar(char c)
{
    asm {
        ldb c
        swi
        fcb $02
    }
}

void mo5_newline(void)
{
    mo5_putchar(MO5_ENTER_CHAR);
    mo5_putchar(MO5_LINE_FEED);
}

void mo5_wait_key(char key)
{
    char ch;
    do {
        ch = mo5_getchar();
    } while (ch != key);
}

char wait_for_key(void) {
    char ch;
    do {
        ch = mo5_getchar();
    } while (ch == 0);
    return ch;
}