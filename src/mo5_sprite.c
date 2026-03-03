/**
 * @file
 * @brief Sprite rendering and actor movement with optimized differential VRAM writes.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_sprite.h"

// ============================================================================
// HELPER INTERNE
// ============================================================================

/*
 * Remplit une zone rectangulaire d'une banque déjà sélectionnée.
 * PRC doit être positionné avant l'appel.
 */
static void fill_rect_vram(unsigned char tx,          unsigned char ty,
                           unsigned char width_bytes, unsigned char height,
                           unsigned char value)
{
    unsigned char *row      = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    unsigned char  rows_left = height;

    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = width_bytes;
        while (col--) *p++ = value;
        row += SCREEN_WIDTH_BYTES;
    }
}

/*
 * Copie un buffer src dans une zone rectangulaire d'une banque déjà sélectionnée.
 * PRC doit être positionné avant l'appel.
 * src est avancé par l'appelant si besoin — ici on reçoit un pointeur direct.
 */
static void blit_rect_vram(unsigned char tx,          unsigned char ty,
                           unsigned char width_bytes, unsigned char height,
                           unsigned char *src)
{
    unsigned char *row      = VRAM + (unsigned int)ty * SCREEN_WIDTH_BYTES + tx;
    unsigned char  rows_left = height;

    while (rows_left--) {
        unsigned char *p   = row;
        unsigned char  col = width_bytes;
        while (col--) *p++ = *src++;
        row += SCREEN_WIDTH_BYTES;
    }
}

// ============================================================================
// API BAS NIVEAU
// ============================================================================

void mo5_draw_sprite(unsigned char tx,          unsigned char ty,
                     unsigned char *form_data,  unsigned char *color_data,
                     unsigned char width_bytes, unsigned char height)
{
    *PRC &= ~0x01;
    blit_rect_vram(tx, ty, width_bytes, height, color_data);

    *PRC |= 0x01;
    blit_rect_vram(tx, ty, width_bytes, height, form_data);
}

void mo5_clear_sprite(unsigned char tx,          unsigned char ty,
                      unsigned char width_bytes, unsigned char height)
{
    *PRC &= ~0x01;
    fill_rect_vram(tx, ty, width_bytes, height, 0x00);

    *PRC |= 0x01;
    fill_rect_vram(tx, ty, width_bytes, height, 0x00);
}

void mo5_move_sprite(unsigned char old_tx,      unsigned char old_ty,
                     unsigned char new_tx,       unsigned char new_ty,
                     unsigned char *form_data,   unsigned char *color_data,
                     unsigned char width_bytes,  unsigned char height)
{
    signed char   dx   = (signed char)(new_tx - old_tx);
    signed char   dy   = (signed char)(new_ty - old_ty);
    unsigned char adx  = dx < 0 ? -dx : dx;
    unsigned char ady  = dy < 0 ? -dy : dy;
    unsigned char bank;

    if (adx >= width_bytes || ady >= height) {
        mo5_clear_sprite(old_tx, old_ty, width_bytes, height);
        mo5_draw_sprite (new_tx, new_ty, form_data, color_data, width_bytes, height);
        return;
    }

    /* Les deux passes (couleur puis forme) sont identiques — on boucle. */
    for (bank = 0; bank < 2; bank++) {
        unsigned char *src = bank ? form_data : color_data;

        if (bank) *PRC |= 0x01; else *PRC &= ~0x01;

        /* Clear colonne sortante */
        if (dx != 0) {
            unsigned char clear_x = (dx > 0) ? old_tx
                                              : old_tx + width_bytes - adx;
            fill_rect_vram(clear_x, old_ty, adx, height, 0x00);
        }

        /* Clear ligne sortante */
        if (dy != 0) {
            unsigned char clear_y = (dy > 0) ? old_ty
                                              : old_ty + height - ady;
            fill_rect_vram(old_tx, clear_y, width_bytes, ady, 0x00);
        }

        blit_rect_vram(new_tx, new_ty, width_bytes, height, src);
    }
}

// ============================================================================
// API ACTOR
// ============================================================================

void mo5_actor_draw(const MO5_Actor *actor)
{
    mo5_draw_sprite(
        actor->pos.x,               actor->pos.y,
        actor->sprite->form,        actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear(const MO5_Actor *actor)
{
    mo5_clear_sprite(
        actor->pos.x,               actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_move(MO5_Actor *actor, unsigned char new_x, unsigned char new_y)
{
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite(
        actor->old_pos.x,           actor->old_pos.y,
        actor->pos.x,               actor->pos.y,
        actor->sprite->form,        actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clamp(MO5_Actor *actor)
{
    unsigned char max_x = SCREEN_WIDTH_BYTES - actor->sprite->width_bytes;
    unsigned char max_y = SCREEN_HEIGHT      - actor->sprite->height;

    if (actor->pos.x > max_x) actor->pos.x = max_x;
    if (actor->pos.y > max_y) actor->pos.y = max_y;
}