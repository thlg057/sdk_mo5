/**
 * @file mo5_music_swi.h
 * @brief Génération musicale via le SWI $1E du moniteur ROM MO5.
 *
 * Utilise la routine moniteur de génération de musique (SWI $1E) pour jouer
 * des notes avec contrôle du timbre, de l'octave et du tempo.
 *
 * Différences avec mo5_audio (buzzer direct) :
 *   - mo5_audio : contrôle total de la fréquence, bloquant, buzzer raw
 *   - mo5_music_swi : notes Do-Si, timbre ROM fixe, bloquant, API haut niveau
 *
 * Les deux sont bloquants. Pour un jeu, privilégier des séquences courtes
 * ou déclencher la musique uniquement hors game loop.
 *
 * Source : Guide du MO5 (Deledicq, Cedic-Nathan 1985) p.262-263.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_MUSIC_SWI_H
#define MO5_MUSIC_SWI_H

/* =========================================================================
 * REGISTRES MONITEUR — PARAMÈTRES MUSIQUE
 * Source : Guide du MO5 p.262-263 + p.275-277
 * ========================================================================= */

#define MO5_TEMPO_H   ((unsigned char*)0x2039)  /* octet haut du tempo */
#define MO5_TEMPO_L   ((unsigned char*)0x203A)  /* octet bas du tempo */
#define MO5_DUREE_H   ((unsigned char*)0x203B)  /* octet haut de la durée */
#define MO5_DUREE_L   ((unsigned char*)0x203C)  /* octet bas de la durée */
#define MO5_TIMBRE    ((unsigned char*)0x203D)  /* attaque/timbre */
#define MO5_OCTAVE_H  ((unsigned char*)0x203E)  /* octet haut octave */
#define MO5_OCTAVE_L  ((unsigned char*)0x203F)  /* octet bas octave (1-5) */

/* =========================================================================
 * CODES DE NOTES (registre B pour SWI $1E)
 * Source : Guide du MO5 p.262
 * ========================================================================= */

#define MO5_NOTE_SILENCE  0x00
#define MO5_NOTE_DO       0x01
#define MO5_NOTE_DOS      0x02   /* DO# */
#define MO5_NOTE_RE       0x03
#define MO5_NOTE_RES      0x04   /* RE# */
#define MO5_NOTE_MI       0x05
#define MO5_NOTE_FA       0x06
#define MO5_NOTE_FAS      0x07   /* FA# */
#define MO5_NOTE_SOL      0x08
#define MO5_NOTE_SOLS     0x09   /* SOL# */
#define MO5_NOTE_LA       0x0A
#define MO5_NOTE_LAS      0x0B   /* LA# */
#define MO5_NOTE_SI       0x0C
#define MO5_NOTE_UT       0x0D   /* DO octave supérieure */

/* =========================================================================
 * DURÉES (valeur pour MO5_DUREE_L)
 * Source : Guide du MO5 p.263
 * Ronde = 96 ; subdivisions par puissances de 2 ou 3 (triolets/pointées)
 * ========================================================================= */

#define MO5_DUR_RONDE           96
#define MO5_DUR_BLANCHE_P       72   /* blanche pointée */
#define MO5_DUR_BLANCHE         48
#define MO5_DUR_NOIRE_P         36   /* noire pointée */
#define MO5_DUR_NOIRE           24   /* noire standard (défaut) */
#define MO5_DUR_CROCHE_P        18   /* croche pointée */
#define MO5_DUR_CROCHE          12
#define MO5_DUR_DCROCHE_P        9   /* double croche pointée */
#define MO5_DUR_DCROCHE          6
#define MO5_DUR_TCROCHE_P        5   /* triple croche pointée */
#define MO5_DUR_TCROCHE          3
/* Triolets */
#define MO5_DUR_NOIRE_TRIO      16   /* noire en triolet (3×16=48=blanche) */
#define MO5_DUR_CROCHE_TRIO      8   /* croche en triolet (3×8=24=noire) */
#define MO5_DUR_DCROCHE_TRIO     4   /* double croche en triolet */

/* =========================================================================
 * OCTAVES (valeur pour MO5_OCTAVE_L)
 * 5 octaves disponibles (1=grave, 4=LA 440Hz, 5=aigu)
 * ========================================================================= */

#define MO5_OCT_1   1   /* grave */
#define MO5_OCT_2   2
#define MO5_OCT_3   3
#define MO5_OCT_4   4   /* octave de référence — LA 440 Hz */
#define MO5_OCT_5   5   /* aigu */

/* =========================================================================
 * TEMPO (valeur pour MO5_TEMPO_L)
 * 1=très rapide, 5=standard (Allegretto), 255=très lent
 * Correspondances approximatives :
 *   T1 = Prestissimo  T4 = Allegro   T8 = Moderato
 *   T5 = Allegretto   T16 = Andante  T32 = Adagio
 * ========================================================================= */

#define MO5_TEMPO_PRESTISSIMO  1
#define MO5_TEMPO_ALLEGRO      4
#define MO5_TEMPO_ALLEGRETTO   5   /* standard à l'init */
#define MO5_TEMPO_MODERATO     8
#define MO5_TEMPO_ANDANTE     16
#define MO5_TEMPO_ADAGIO      32
#define MO5_TEMPO_LENTO       48
#define MO5_TEMPO_LARGO       64

/* =========================================================================
 * TIMBRE / ATTAQUE (valeur pour MO5_TIMBRE)
 * 0   = son continu (legato)
 * 10  = légèrement articulé
 * 100 = détaché
 * 200 = très piqué (staccato)
 * ========================================================================= */

#define MO5_TIMBRE_LEGATO      0
#define MO5_TIMBRE_NORMAL     10
#define MO5_TIMBRE_DETACHE   100
#define MO5_TIMBRE_STACCATO  200

/* =========================================================================
 * STRUCTURE NOTE
 * ========================================================================= */

/**
 * Une note de la mélodie.
 * Utiliser MO5_NOTE_SILENCE pour un silence (note=0, durée=longueur du silence).
 * Terminer un tableau de notes par { 0, 0, 0, 0 }.
 */
typedef struct {
    unsigned char note;    /* code note (MO5_NOTE_xxx) */
    unsigned char duree;   /* durée (MO5_DUR_xxx) */
    unsigned char octave;  /* octave (MO5_OCT_1 à MO5_OCT_5) */
    unsigned char timbre;  /* attaque (MO5_TIMBRE_xxx ou 0-255) */
} MO5_Note;

/* =========================================================================
 * API
 * ========================================================================= */

/**
 * Joue une note unique via le SWI $1E du moniteur.
 * Bloquant — retourne quand la note est terminée.
 *
 * @param note    Code note (MO5_NOTE_DO … MO5_NOTE_SI, MO5_NOTE_SILENCE)
 * @param duree   Durée (MO5_DUR_NOIRE, MO5_DUR_CROCHE …)
 * @param octave  Octave (MO5_OCT_1 … MO5_OCT_5)
 * @param timbre  Attaque (MO5_TIMBRE_LEGATO … MO5_TIMBRE_STACCATO)
 * @param tempo   Tempo (MO5_TEMPO_ALLEGRO … MO5_TEMPO_LARGO)
 */
void mo5_swi_play_note(unsigned char note,   unsigned char duree,
                       unsigned char octave, unsigned char timbre,
                       unsigned char tempo);

/**
 * Joue une mélodie entière (tableau de MO5_Note terminé par {0,0,0,0}).
 * Bloquant — retourne quand toute la mélodie est terminée.
 *
 * @param melody  Pointeur vers le tableau de notes
 * @param tempo   Tempo global pour toute la mélodie
 */
void mo5_swi_play_melody(const MO5_Note *melody, unsigned char tempo);

/**
 * Joue un silence (pause) de la durée indiquée.
 * Utilise MO5_NOTE_SILENCE via le SWI.
 *
 * @param duree  Durée du silence (MO5_DUR_xxx)
 * @param tempo  Tempo courant
 */
void mo5_swi_silence(unsigned char duree, unsigned char tempo);

#endif /* MO5_MUSIC_SWI_H */
