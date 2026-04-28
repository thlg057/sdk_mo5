/**
 * @file mo5_joystick.c
 * @brief Lecture des manettes de jeu — PIA extension $A7CC-$A7CF.
 *
 * Corrections v2 par rapport à la version initiale :
 *   - Boutons fire lus sur PORTB $A7CD bits 6-7 (et non sur CRA)
 *   - Source : Clefs Pour MO5 (Blanchard, 1985) p.99 et p.107
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#include "mo5_joystick.h"

/* =========================================================================
 * ÉTAT GLOBAL
 * ========================================================================= */

MO5_Joystick mo5_joy1;
MO5_Joystick mo5_joy2;

/* =========================================================================
 * IMPLÉMENTATION
 * ========================================================================= */

void mo5_joystick_init(void)
{
    unsigned char crb;

    /* PORTA en entrées :
     * CRA bit2=0 → accès DDRA, écrire 0x00 → toutes broches en entrées
     * CRA bit2=1 → accès ORA */
    *JOY_CRA   = 0x00;
    *JOY_PORTA = 0x00;
    *JOY_CRA   = 0x04;

    /* PORTB : bits 6-7 doivent être en entrées pour lire les boutons fire.
     * Si le DAC a été initialisé (bits 0-5 en sorties via 0x3F dans DDRB),
     * on préserve les bits 0-5 et on force bits 6-7 en entrées.
     * Séquence : CRB bit2=0 → accès DDRB, masquer 6-7, CRB bit2=1 → accès PORTB. */
    crb  = *JOY_CRB;
    crb &= ~0x04;
    *JOY_CRB   = crb;                    /* accès DDRB */
    *JOY_PORTB = (*JOY_PORTB) & 0x3F;   /* bits 6-7 en entrées, 0-5 préservés */
    crb |= 0x04;
    *JOY_CRB   = crb;                    /* accès PORTB */

    /* Initialiser les structures à zéro */
    mo5_joy1.dirs      = 0;
    mo5_joy1.fire      = 0;
    mo5_joy1.prev_dirs = 0;
    mo5_joy1.prev_fire = 0;

    mo5_joy2.dirs      = 0;
    mo5_joy2.fire      = 0;
    mo5_joy2.prev_dirs = 0;
    mo5_joy2.prev_fire = 0;
}

void mo5_joystick_update(void)
{
    unsigned char porta;
    unsigned char portb;
    unsigned char raw1;
    unsigned char raw2;

    /* --- Sauvegarder l'état précédent --- */
    mo5_joy1.prev_dirs = mo5_joy1.dirs;
    mo5_joy1.prev_fire = mo5_joy1.fire;
    mo5_joy2.prev_dirs = mo5_joy2.dirs;
    mo5_joy2.prev_fire = mo5_joy2.fire;

    /* --- Lire PORTA $A7CC : directions des deux manettes ---
     * Actif bas (0 = pressé). On inverse : 1 = pressé.
     * Source : Clefs Pour MO5 p.98
     *   bits 0-3 : manette 0 (haut/bas/gauche/droite)
     *   bits 4-7 : manette 1 (haut/bas/gauche/droite)
     */
    porta = ~(*JOY_PORTA);

    raw1 = porta & 0x0F;   /* manette 0 : bits 0-3 */
    mo5_joy1.dirs = raw1;

    raw2 = porta & 0xF0;   /* manette 1 : bits 4-7 */
    mo5_joy2.dirs = raw2;

    /* --- Lire PORTB $A7CD : boutons fire ---
     * Actif bas (0 = pressé). On inverse : 1 = pressé.
     * Source : Clefs Pour MO5 p.99 et p.107
     *   bit 6 : bouton manette 0
     *   bit 7 : bouton manette 1
     */
    portb = *JOY_PORTB;

    mo5_joy1.fire = (portb & JOY1_FIRE_BIT) ? 0 : 1;
    mo5_joy2.fire = (portb & JOY2_FIRE_BIT) ? 0 : 1;
}
