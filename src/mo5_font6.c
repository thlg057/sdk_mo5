/**
 * @file mo5_font6.c
 * @brief Affichage de texte en mode graphique MO5 — police arcade 8x6 pixels.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_font6.h"
#include <mo5_sprite_bg.h>

/* =========================================================================
 * BITMAPS DE LA POLICE  (prives a cette unite de compilation)
 * ========================================================================= */

static unsigned char f_A[]    = {0x18,0x3C,0x66,0x7E,0x66,0x00};
static unsigned char f_B[]    = {0x7C,0x66,0x7C,0x66,0x7C,0x00};
static unsigned char f_C[]    = {0x3C,0x66,0x60,0x66,0x3C,0x00};
static unsigned char f_D[]    = {0x7C,0x66,0x66,0x66,0x7C,0x00};
static unsigned char f_E[]    = {0x7E,0x60,0x78,0x60,0x7E,0x00};
static unsigned char f_F[]    = {0x7E,0x60,0x78,0x60,0x60,0x00};
static unsigned char f_G[]    = {0x3C,0x66,0x6E,0x66,0x3C,0x00};
static unsigned char f_H[]    = {0x66,0x66,0x7E,0x66,0x66,0x00};
static unsigned char f_I[]    = {0x3C,0x18,0x18,0x18,0x3C,0x00};
static unsigned char f_J[]    = {0x0E,0x06,0x06,0x66,0x3C,0x00};
static unsigned char f_K[]    = {0x66,0x6C,0x78,0x6C,0x66,0x00};
static unsigned char f_L[]    = {0x60,0x60,0x60,0x60,0x7E,0x00};
static unsigned char f_M[]    = {0x63,0x77,0x6B,0x63,0x63,0x00};
static unsigned char f_N[]    = {0x66,0x76,0x7E,0x6E,0x66,0x00};
static unsigned char f_O[]    = {0x3C,0x66,0x66,0x66,0x3C,0x00};
static unsigned char f_P[]    = {0x7C,0x66,0x7C,0x60,0x60,0x00};
static unsigned char f_Q[]    = {0x3C,0x66,0x66,0x6E,0x3E,0x00};
static unsigned char f_R[]    = {0x7C,0x66,0x7C,0x6C,0x66,0x00};
static unsigned char f_S[]    = {0x3C,0x60,0x3C,0x06,0x7C,0x00};
static unsigned char f_T[]    = {0x7E,0x18,0x18,0x18,0x18,0x00};
static unsigned char f_U[]    = {0x66,0x66,0x66,0x66,0x3C,0x00};
static unsigned char f_V[]    = {0x66,0x66,0x66,0x3C,0x18,0x00};
static unsigned char f_W[]    = {0x63,0x63,0x6B,0x77,0x63,0x00};
static unsigned char f_X[]    = {0x66,0x3C,0x18,0x3C,0x66,0x00};
static unsigned char f_Y[]    = {0x66,0x66,0x3C,0x18,0x18,0x00};
static unsigned char f_Z[]    = {0x7E,0x0C,0x18,0x30,0x7E,0x00};
static unsigned char f_0[]    = {0x3C,0x6E,0x76,0x66,0x3C,0x00};
static unsigned char f_1[]    = {0x18,0x38,0x18,0x18,0x7E,0x00};
static unsigned char f_2[]    = {0x3C,0x06,0x1C,0x30,0x7E,0x00};
static unsigned char f_3[]    = {0x3C,0x06,0x1C,0x06,0x3C,0x00};
static unsigned char f_4[]    = {0x0C,0x6C,0x7E,0x0C,0x0C,0x00};
static unsigned char f_5[]    = {0x7E,0x60,0x7C,0x06,0x7C,0x00};
static unsigned char f_6[]    = {0x3C,0x60,0x7C,0x66,0x3C,0x00};
static unsigned char f_7[]    = {0x7E,0x06,0x0C,0x18,0x18,0x00};
static unsigned char f_8[]    = {0x3C,0x66,0x3C,0x66,0x3C,0x00};
static unsigned char f_9[]    = {0x3C,0x66,0x3E,0x06,0x3C,0x00};
static unsigned char f_DOT[]  = {0x00,0x00,0x00,0x00,0x18,0x00};
static unsigned char f_EXCL[] = {0x18,0x18,0x18,0x00,0x18,0x00};
static unsigned char f_SPACE[]= {0x00,0x00,0x00,0x00,0x00,0x00};
static unsigned char f_COLON[] = {0x00,0x00,0x18,0x00,0x18,0x00};
static unsigned char f_EQUAL[]    = {0x00,0x7E,0x00,0x7E,0x00,0x00};
static unsigned char f_MINUS[]    = {0x00,0x00,0x7E,0x00,0x00,0x00};
static unsigned char f_UNDER[]    = {0x00,0x00,0x00,0x00,0x7E,0x00};
static unsigned char f_STAR[]     = {0x66,0x3C,0xFF,0x3C,0x66,0x00};
static unsigned char f_SLASH[]    = {0x06,0x0C,0x18,0x30,0x60,0x00};
static unsigned char f_BSLASH[]   = {0x60,0x30,0x18,0x0C,0x06,0x00};

/* =========================================================================
 * TABLE D'ACCES  (privee)
 * ========================================================================= */

static unsigned char *font6_alpha[] = {
    f_A,f_B,f_C,f_D,f_E,f_F,f_G,f_H,f_I,f_J,f_K,f_L,f_M,
    f_N,f_O,f_P,f_Q,f_R,f_S,f_T,f_U,f_V,f_W,f_X,f_Y,f_Z
};

static unsigned char *font6_nums[] = {
    f_0,f_1,f_2,f_3,f_4,f_5,f_6,f_7,f_8,f_9
};

static unsigned char *font6_get(char c)
{
    if (c >= 'A' && c <= 'Z') return font6_alpha[c - 'A'];
    if (c >= 'a' && c <= 'z') return font6_alpha[c - 'a'];
    if (c >= '0' && c <= '9') return font6_nums[c - '0'];
    if (c == '.')              return f_DOT;
    if (c == '!')              return f_EXCL;
    if (c == ':')              return f_COLON;
    if (c == '=')              return f_EQUAL;
    if (c == '-')              return f_MINUS;
    if (c == '_')              return f_UNDER;
    if (c == '*')              return f_STAR;
    if (c == '/')              return f_SLASH;
    if (c == '\\')              return f_BSLASH;
    return f_SPACE;
}

/* =========================================================================
 * RENDU  (putchar prive, puts et clear publics)
 * ========================================================================= */

static void font6_putchar(unsigned char tx, unsigned char ty,
                          char c, unsigned char fg_color)
{
    unsigned char color_data[6];
    unsigned char fg = (fg_color & 0x0F) << 4;
    unsigned char i  = 6;

    while (i--) color_data[i] = fg;

    /* ty en pixels bruts — pas de multiplication */
    mo5_draw_sprite_bg(tx, ty, font6_get(c), color_data, 1, 6);
}

void mo5_font6_puts(unsigned char tx, unsigned char ty,
                    const char *s, unsigned char fg_color)
{
    while (*s) {
        font6_putchar(tx, ty, *s++, fg_color);
        tx++;
        if (tx >= SCREEN_WIDTH_BYTES) { tx = 0; ty += 6; }
    }
}

void mo5_font6_clear(unsigned char tx, unsigned char ty, unsigned char len)
{
    while (len--) {
        font6_putchar(tx++, ty, ' ', 0);
        if (tx >= SCREEN_WIDTH_BYTES) { tx = 0; ty += 6; }
    }
}
