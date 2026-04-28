/**
 * @file mo5_music_swi.c
 * @brief Génération musicale via le SWI $1E du moniteur ROM MO5.
 *
 * Source : Guide du MO5 (Deledicq, Cedic-Nathan 1985) p.262-263.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_music_swi.h"

/* =========================================================================
 * INTERNE — Initialiser les registres moniteur et appeler le SWI $1E
 *
 * Contraintes cmoc C89 :
 *   - Pas de déclaration après un statement
 *   - Pas d'initialisation à la déclaration
 *   - Pas de volatile — lire les registres via pointeurs directs
 *   - Pas de static inline
 * ========================================================================= */

static void swi_play(unsigned char note, unsigned char duree,
                     unsigned char octave, unsigned char timbre,
                     unsigned char tempo)
{
    /* Charger les registres moniteur avant l'appel SWI */
    *MO5_TEMPO_H  = 0;
    *MO5_TEMPO_L  = tempo;
    *MO5_DUREE_H  = 0;
    *MO5_DUREE_L  = duree;
    *MO5_TIMBRE   = timbre;
    *MO5_OCTAVE_H = 0;
    *MO5_OCTAVE_L = octave;

    /* Appel SWI $1E — note passée dans le registre B */
    asm {
        ldb note
        swi
        fcb $1E
    }
}

/* =========================================================================
 * API PUBLIQUE
 * ========================================================================= */

void mo5_swi_play_note(unsigned char note,   unsigned char duree,
                       unsigned char octave, unsigned char timbre,
                       unsigned char tempo)
{
    swi_play(note, duree, octave, timbre, tempo);
}

void mo5_swi_silence(unsigned char duree, unsigned char tempo)
{
    swi_play(MO5_NOTE_SILENCE, duree, MO5_OCT_4, MO5_TIMBRE_LEGATO, tempo);
}

void mo5_swi_play_melody(const MO5_Note *melody, unsigned char tempo)
{
    unsigned char i;
    i = 0;

    while (melody[i].note != 0 || melody[i].duree != 0) {
        swi_play(melody[i].note,
                 melody[i].duree,
                 melody[i].octave,
                 melody[i].timbre,
                 tempo);
        i++;
    }
}
