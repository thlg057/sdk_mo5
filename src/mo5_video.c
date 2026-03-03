/**
 * @file
 * @brief Video initialization, screen clear, rectangle fill and VBL synchronization.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_video.h"

void mo5_video_init(unsigned char color)
{
    unsigned char *p;
    unsigned int   n;

    *PRC    = 0x00;
    *VIDEO_REG |= 0x01;

    /* Banque couleur */
    *PRC &= ~0x01;
    p = VRAM;
    n = SCREEN_SIZE_BYTES;
    while (n--) *p++ = color;

    /* Banque forme */
    *PRC |= 0x01;
    p = VRAM;
    n = SCREEN_SIZE_BYTES;
    while (n--) *p++ = 0x00;
}

void mo5_wait_vbl(void)
{
    while ( *VBL_REG &  VBL_BIT) ;
    while (!(*VBL_REG & VBL_BIT)) ;
}

void mo5_clear_screen(unsigned char color)
{
    unsigned char *p;
    unsigned int   n;

    *PRC &= ~0x01;
    p = VRAM;
    n = SCREEN_SIZE_BYTES;
    while (n--) *p++ = color;

    *PRC |= 0x01;
    p = VRAM;
    n = SCREEN_SIZE_BYTES;
    while (n--) *p++ = 0x00;
}

void mo5_fill_rect(unsigned char tx, unsigned char ty,
                   unsigned char w,  unsigned char h,
                   unsigned char color)
{
    unsigned char *row;
    unsigned char *p;
    unsigned char  j;
    unsigned char  rows_left = h;

    *PRC &= ~0x01;
    row = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    while (rows_left--) {
        p = row;
        j = w;
        while (j--) *p++ = color;
        row += SCREEN_WIDTH_BYTES;
    }

    *PRC |= 0x01;
    row = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    rows_left = h;
    while (rows_left--) {
        p = row;
        j = w;
        while (j--) *p++ = 0x00;
        row += SCREEN_WIDTH_BYTES;
    }
}