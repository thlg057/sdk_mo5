/**
 * @file mo5_audio.c
 * @brief Audio — buzzer système et DAC extension.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_audio.h"

/* =========================================================================
 * API BUZZER
 * ========================================================================= */

/*
 * Méthode recommandée : agir sur le registre STATUS du moniteur $2019 bit 3.
 * bit 3 = 0 → bruitage clavier activé
 * bit 3 = 1 → bruitage clavier désactivé
 * Source : Clefs Pour MO5 (Blanchard, 1985) p.110 et p.118.
 *
 * C'est plus propre que d'agir directement sur PORTB $A7C1 car :
 * - le moniteur gère lui-même le bip via ce flag
 * - on ne perturbe pas la matrice clavier sur PORTB
 */
void mo5_mute_beep(void)
{
    unsigned char val;
    val  = *((unsigned char*)0x2019);
    val |= 0x08;
    *((unsigned char*)0x2019) = val;
}

void mo5_unmute_beep(void)
{
    unsigned char val;
    val  = *((unsigned char*)0x2019);
    val &= ~0x08;
    *((unsigned char*)0x2019) = val;
}

/*
 * Principe : on fait basculer le bit buzzer (BUZZER_REG = $A7C1 bit 0)
 * à la fréquence voulue via des boucles d'attente.
 * Une "période" = un toggle à 1 + attente + un toggle à 0 + attente.
 * L'attente est une boucle vide sur unsigned char (max 255 itérations).
 *
 * Note cmoc : pas de volatile — on écrit directement via le pointeur
 * BUZZER_REG à chaque itération pour forcer la relecture hardware.
 */
void mo5_beep(unsigned char half_period, unsigned char duration)
{
    unsigned char d;
    unsigned char w;

    d = duration;
    while (d--) {
        /* Toggle haut */
        *BUZZER_REG |= BUZZER_BIT;
        w = half_period;
        while (w--) ;

        /* Toggle bas */
        *BUZZER_REG &= ~BUZZER_BIT;
        w = half_period;
        while (w--) ;
    }

    /* Laisser le buzzer en état bas en fin de bip */
    *BUZZER_REG &= ~BUZZER_BIT;
}

void mo5_beep_short(void)
{
    mo5_beep(30, 80);
}

void mo5_beep_error(void)
{
    mo5_beep(120, 150);
}

void mo5_beep_ok(void)
{
    mo5_beep(8, 60);
}

/* =========================================================================
 * API DAC
 * ========================================================================= */

/*
 * Séquence d'initialisation DDR issue du Manuel Technique p.48 :
 *
 *   CLR  $A7CF       ; CRB = 0  → accès DDRB
 *   LDD  #$3F04
 *   STA  $A7CD       ; DDRB : bits B0-B5 en sorties (0x3F)
 *   STB  $A7CF       ; CRB bit2 = 1 → accès PORTB
 */
void mo5_dac_init(void)
{
    *DAC_CRB_REG = 0x00;       /* accès DDRB */
    *DAC_DDR_REG = DAC_DDR_MASK; /* B0-B5 en sorties */
    *DAC_CRB_REG = 0x04;       /* CRB bit2=1 → accès PORTB */
}

void mo5_dac_write(unsigned char val)
{
    *DAC_REG = val & DAC_MAX;
}

void mo5_dac_silence(void)
{
    *DAC_REG = 0x00;
}

/*
 * Lecture d'une table d'échantillons.
 * Bloquant — à appeler depuis le VBL ou une routine dédiée.
 *
 * period=0 : le délai inter-échantillons est réduit au minimum
 * (overhead de boucle + écriture registre, ~8-10 cycles par échantillon).
 */
void mo5_dac_play(const unsigned char *samples, unsigned int count,
                  unsigned char period)
{
    unsigned int  i;
    unsigned char w;

    i = count;
    while (i--) {
        *DAC_REG = *samples++ & DAC_MAX;

        if (period) {
            w = period;
            while (w--) ;
        }
    }

    mo5_dac_silence();
}
