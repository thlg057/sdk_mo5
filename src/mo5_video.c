/**
 * @file
 * @brief Video initialization, screen clear, rectangle fill and VBL synchronization.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_video.h"

unsigned int row_offsets[SCREEN_HEIGHT];

void mo5_video_init(unsigned char color) {
    unsigned int i;

    for (i = 0; i < SCREEN_HEIGHT; i++)
        row_offsets[i] = i * SCREEN_WIDTH_BYTES;

    *PRC = 0x00;
    *VIDEO_REG |= 0x01;

    *PRC &= ~0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = color;

    *PRC |= 0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = 0x00;
}

void mo5_wait_vbl(void) {
    while (*VBL_REG & VBL_BIT)
        ;
    while (!(*VBL_REG & VBL_BIT))
        ;
}

void mo5_clear_screen(unsigned char color) {
    unsigned int i;

    *PRC &= ~0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = color;

    *PRC |= 0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = 0x00;
}

void mo5_fill_rect(unsigned char tx, unsigned char ty,
                   unsigned char w,  unsigned char h,
                   unsigned char color) {
    unsigned int  offset;
    unsigned char i, j;

    *PRC &= ~0x01;
    for (i = 0; i < h; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = color;
    }

    *PRC |= 0x01;
    for (i = 0; i < h; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = 0x00;
    }
}
