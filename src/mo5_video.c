/**
 * @file
 * @brief Video initialization, screen clear, rectangle fill and VBL synchronization.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */


#include "mo5_video.h"

// ============================================================================
// TABLE DES OFFSETS DE LIGNES
// row_offsets[y] = y * SCREEN_WIDTH_BYTES
// Accès : offset = row_offsets[y] + x
// ============================================================================

unsigned int row_offsets[SCREEN_HEIGHT];

// ============================================================================
// INITIALISATION
// ============================================================================

void mo5_video_init(unsigned char color) {
    unsigned int i;

    // Précalcul de la table row_offsets (évite les multiplications à l'affichage)
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        row_offsets[i] = i * SCREEN_WIDTH_BYTES;
    }

    // Activation du mode graphique
    *PRC = 0x00;
    *VIDEO_REG |= 0x01;

    // Remplissage de la banque couleur
    *PRC &= ~0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++) {
        VRAM[i] = color;
    }

    // Effacement de la banque forme
    *PRC |= 0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++) {
        VRAM[i] = 0x00;
    }
}

// ============================================================================
// SYNCHRONISATION VBL
// ============================================================================

/**
 * Attend le prochain retour de trame vertical (50 Hz en PAL = 20 ms).
 *
 * Principe : attendre la fin du VBL courant (si on est dedans),
 * puis attendre le début du prochain. Garantit une synchronisation
 * propre même si la fonction est appelée en milieu de trame.
 */
void mo5_wait_vbl(void) {
    // Attendre la fin du VBL courant si on est déjà dedans
    while (*VBL_REG & VBL_BIT)
        ;
    // Attendre le début du prochain VBL
    while (!(*VBL_REG & VBL_BIT))
        ;
}

// ============================================================================
// UTILITAIRES ÉCRAN
// ============================================================================

void mo5_clear_screen(unsigned char color) {
    unsigned int i;

    *PRC &= ~0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = color;

    *PRC |= 0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++)
        VRAM[i] = 0x00;
}

void mo5_fill_rect(int tx, int ty, int w, int h, unsigned char color) {
    unsigned int offset;
    int i, j;

    // Banque couleur
    *PRC &= ~0x01;
    for (i = 0; i < h; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = color;
    }

    // Banque forme (0x00 = fond uniforme)
    *PRC |= 0x01;
    for (i = 0; i < h; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < w; j++)
            VRAM[offset + j] = 0x00;
    }
}
