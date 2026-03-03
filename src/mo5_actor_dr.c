/**
 * @file
 * @brief Dirty Rectangle sprite engine — implémentation.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_actor_dr.h"

// ============================================================================
// HELPERS INTERNES
// ============================================================================

/*
 * Copie width_bytes*height octets entre VRAM et un buffer.
 * dir=0 : VRAM → buf (save)
 * dir=1 : buf → VRAM (restore)
 * PRC doit être positionné avant l'appel.
 */
static void dr_transfer(unsigned char tx,          unsigned char ty,
                        unsigned char width_bytes, unsigned char height,
                        unsigned char *buf,        unsigned char dir)
{
    unsigned char *row      = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    unsigned char  rows_left = height;

    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = width_bytes;

        if (dir) {
            while (col--) *p++ = *buf++;   /* restore : buf → VRAM */
        } else {
            while (col--) *buf++ = *p++;   /* save    : VRAM → buf */
        }

        row += SCREEN_WIDTH_BYTES;
    }
}

// ============================================================================
// FONCTIONS INTERNES
// ============================================================================

static void dr_save(MO5_Actor_DR *actor)
{
    unsigned char w = actor->sprite->width_bytes;
    unsigned char h = actor->sprite->height;
    unsigned char x = actor->pos.x;
    unsigned char y = actor->pos.y;

    *PRC &= ~0x01;
    dr_transfer(x, y, w, h, actor->save_color, 0);

    *PRC |= 0x01;
    dr_transfer(x, y, w, h, actor->save_form,  0);
}

static void dr_restore(MO5_Actor_DR *actor)
{
    unsigned char w = actor->sprite->width_bytes;
    unsigned char h = actor->sprite->height;
    unsigned char x = actor->pos.x;
    unsigned char y = actor->pos.y;

    *PRC &= ~0x01;
    dr_transfer(x, y, w, h, actor->save_color, 1);

    *PRC |= 0x01;
    dr_transfer(x, y, w, h, actor->save_form,  1);
}

static void dr_draw(MO5_Actor_DR *actor)
{
    unsigned char *row       = VRAM + (unsigned int)actor->pos.y * SCREEN_WIDTH_BYTES
                                    + actor->pos.x;
    unsigned char *color_src = actor->sprite->color;
    unsigned char *form_src  = actor->sprite->form;
    unsigned char  rows_left = actor->sprite->height;

    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = actor->sprite->width_bytes;

        while (col--) {
            unsigned char fg = *color_src & 0xF0;

            if (fg) {
                *PRC &= ~0x01;
                *p = (*p & 0x0F) | fg;

                *PRC |= 0x01;
                *p |= *form_src;
            }

            p++;
            color_src++;
            form_src++;
        }

        row += SCREEN_WIDTH_BYTES;
    }
}

// ============================================================================
// API PUBLIQUE
// ============================================================================

void mo5_actor_dr_init(MO5_Actor_DR *actor, const MO5_Sprite *sprite,
                       unsigned char x, unsigned char y)
{
    actor->sprite = sprite;
    actor->pos.x  = x;
    actor->pos.y  = y;
    dr_save(actor);
    dr_draw(actor);
}

void mo5_actor_dr_restore(MO5_Actor_DR *actor)
{
    dr_restore(actor);
}

void mo5_actor_dr_save_draw(MO5_Actor_DR *actor)
{
    dr_save(actor);
    dr_draw(actor);
}

void mo5_actor_dr_move(MO5_Actor_DR *actor, unsigned char x, unsigned char y)
{
    actor->pos.x = x;
    actor->pos.y = y;
}