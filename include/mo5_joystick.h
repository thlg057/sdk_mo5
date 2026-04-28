/**
 * @file mo5_joystick.h
 * @brief Lecture des manettes de jeu — PIA extension $A7CC-$A7CF.
 *
 * Polling à appeler une fois par frame (dans la boucle VBL).
 * Fournit l'état courant et la détection d'edge (appui / relâché)
 * pour les directions et le bouton fire.
 *
 * Disponibilité :
 *   Extension jeux/musique optionnelle sur MO5 de base.
 *   Intégrée de série sur : MO5E, MO6, MO5NR.
 *
 * Câblage (Clefs Pour MO5, Blanchard 1985, p.98-99 et p.107) :
 *   $A7CC PORTA bit 0 : manette 0 — HAUT    (actif bas)
 *   $A7CC PORTA bit 1 : manette 0 — BAS     (actif bas)
 *   $A7CC PORTA bit 2 : manette 0 — GAUCHE  (actif bas)
 *   $A7CC PORTA bit 3 : manette 0 — DROITE  (actif bas)
 *   $A7CC PORTA bit 4 : manette 1 — HAUT    (actif bas)
 *   $A7CC PORTA bit 5 : manette 1 — BAS     (actif bas)
 *   $A7CC PORTA bit 6 : manette 1 — GAUCHE  (actif bas)
 *   $A7CC PORTA bit 7 : manette 1 — DROITE  (actif bas)
 *   $A7CD PORTB bit 6 : bouton fire manette 0 (actif bas)
 *   $A7CD PORTB bit 7 : bouton fire manette 1 (actif bas)
 *   $A7CE CRA  CA1    : IRQ bouton manette 0
 *   $A7CF CRB  CB1    : IRQ bouton manette 1
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_JOYSTICK_H
#define MO5_JOYSTICK_H

/* =========================================================================
 * REGISTRES PIA EXTENSION
 * ========================================================================= */

/** PORTA $A7CC — directions des deux manettes (actif bas) */
#define JOY_PORTA    ((unsigned char *)0xA7CC)

/** PORTB $A7CD — boutons fire bits 6-7 (actif bas) + DAC bits 0-5 */
#define JOY_PORTB    ((unsigned char *)0xA7CD)

/** CRA $A7CE — contrôle PORTA */
#define JOY_CRA      ((unsigned char *)0xA7CE)

/** CRB $A7CF — contrôle PORTB */
#define JOY_CRB      ((unsigned char *)0xA7CF)

/* =========================================================================
 * MASQUES DIRECTIONS (actif bas — 0 = pressé, normalisé : 1 = pressé)
 * ========================================================================= */

/** Manette 0 — bits 0-3 de PORTA */
#define JOY1_UP      0x01
#define JOY1_DOWN    0x02
#define JOY1_LEFT    0x04
#define JOY1_RIGHT   0x08

/** Manette 1 — bits 4-7 de PORTA */
#define JOY2_UP      0x10
#define JOY2_DOWN    0x20
#define JOY2_LEFT    0x40
#define JOY2_RIGHT   0x80

/** Boutons fire dans PORTB $A7CD (actifs bas)
 *  Source : Clefs Pour MO5 p.99 et p.107
 *  bit 6 = bouton manette 0
 *  bit 7 = bouton manette 1
 */
#define JOY1_FIRE_BIT  0x40   /* PORTB bit 6 — manette 0 */
#define JOY2_FIRE_BIT  0x80   /* PORTB bit 7 — manette 1 */

/* =========================================================================
 * STRUCTURE
 * ========================================================================= */

/**
 * État d'une manette.
 * Champ `dirs`  : bits directions actifs (1 = pressé, après inversion logique)
 * Champ `fire`  : bouton fire actif (1 = pressé, après inversion logique)
 * Champs `prev_*` : état du frame précédent pour l'edge detection
 */
typedef struct {
    unsigned char dirs;       /* directions courantes (bits normalisés) */
    unsigned char fire;       /* bouton fire courant (0 ou 1) */
    unsigned char prev_dirs;  /* directions frame précédent */
    unsigned char prev_fire;  /* bouton fire frame précédent */
} MO5_Joystick;

/* =========================================================================
 * ÉTAT GLOBAL (une instance par manette)
 * ========================================================================= */

extern MO5_Joystick mo5_joy1;   /* manette 0 */
extern MO5_Joystick mo5_joy2;   /* manette 1 */

/* =========================================================================
 * MACROS DE TEST — MANETTE 0
 * ========================================================================= */

/** Direction maintenue ce frame */
#define MO5_JOY1_UP_HELD()      (mo5_joy1.dirs & JOY1_UP)
#define MO5_JOY1_DOWN_HELD()    (mo5_joy1.dirs & JOY1_DOWN)
#define MO5_JOY1_LEFT_HELD()    (mo5_joy1.dirs & JOY1_LEFT)
#define MO5_JOY1_RIGHT_HELD()   (mo5_joy1.dirs & JOY1_RIGHT)
#define MO5_JOY1_FIRE_HELD()    (mo5_joy1.fire)

/** Direction venant d'être pressée ce frame (edge montant) */
#define MO5_JOY1_UP_PRESSED()    ((mo5_joy1.dirs & JOY1_UP)    && !(mo5_joy1.prev_dirs & JOY1_UP))
#define MO5_JOY1_DOWN_PRESSED()  ((mo5_joy1.dirs & JOY1_DOWN)  && !(mo5_joy1.prev_dirs & JOY1_DOWN))
#define MO5_JOY1_LEFT_PRESSED()  ((mo5_joy1.dirs & JOY1_LEFT)  && !(mo5_joy1.prev_dirs & JOY1_LEFT))
#define MO5_JOY1_RIGHT_PRESSED() ((mo5_joy1.dirs & JOY1_RIGHT) && !(mo5_joy1.prev_dirs & JOY1_RIGHT))
#define MO5_JOY1_FIRE_PRESSED()  (mo5_joy1.fire && !mo5_joy1.prev_fire)

/** Direction venant d'être relâchée ce frame (edge descendant) */
#define MO5_JOY1_UP_RELEASED()    (!(mo5_joy1.dirs & JOY1_UP)    && (mo5_joy1.prev_dirs & JOY1_UP))
#define MO5_JOY1_DOWN_RELEASED()  (!(mo5_joy1.dirs & JOY1_DOWN)  && (mo5_joy1.prev_dirs & JOY1_DOWN))
#define MO5_JOY1_LEFT_RELEASED()  (!(mo5_joy1.dirs & JOY1_LEFT)  && (mo5_joy1.prev_dirs & JOY1_LEFT))
#define MO5_JOY1_RIGHT_RELEASED() (!(mo5_joy1.dirs & JOY1_RIGHT) && (mo5_joy1.prev_dirs & JOY1_RIGHT))
#define MO5_JOY1_FIRE_RELEASED()  (!mo5_joy1.fire && mo5_joy1.prev_fire)

/* =========================================================================
 * MACROS DE TEST — MANETTE 1
 * ========================================================================= */

/** Direction maintenue ce frame */
#define MO5_JOY2_UP_HELD()      (mo5_joy2.dirs & JOY2_UP)
#define MO5_JOY2_DOWN_HELD()    (mo5_joy2.dirs & JOY2_DOWN)
#define MO5_JOY2_LEFT_HELD()    (mo5_joy2.dirs & JOY2_LEFT)
#define MO5_JOY2_RIGHT_HELD()   (mo5_joy2.dirs & JOY2_RIGHT)
#define MO5_JOY2_FIRE_HELD()    (mo5_joy2.fire)

/** Direction venant d'être pressée ce frame (edge montant) */
#define MO5_JOY2_UP_PRESSED()    ((mo5_joy2.dirs & JOY2_UP)    && !(mo5_joy2.prev_dirs & JOY2_UP))
#define MO5_JOY2_DOWN_PRESSED()  ((mo5_joy2.dirs & JOY2_DOWN)  && !(mo5_joy2.prev_dirs & JOY2_DOWN))
#define MO5_JOY2_LEFT_PRESSED()  ((mo5_joy2.dirs & JOY2_LEFT)  && !(mo5_joy2.prev_dirs & JOY2_LEFT))
#define MO5_JOY2_RIGHT_PRESSED() ((mo5_joy2.dirs & JOY2_RIGHT) && !(mo5_joy2.prev_dirs & JOY2_RIGHT))
#define MO5_JOY2_FIRE_PRESSED()  (mo5_joy2.fire && !mo5_joy2.prev_fire)

/** Direction venant d'être relâchée ce frame (edge descendant) */
#define MO5_JOY2_UP_RELEASED()    (!(mo5_joy2.dirs & JOY2_UP)    && (mo5_joy2.prev_dirs & JOY2_UP))
#define MO5_JOY2_DOWN_RELEASED()  (!(mo5_joy2.dirs & JOY2_DOWN)  && (mo5_joy2.prev_dirs & JOY2_DOWN))
#define MO5_JOY2_LEFT_RELEASED()  (!(mo5_joy2.dirs & JOY2_LEFT)  && (mo5_joy2.prev_dirs & JOY2_LEFT))
#define MO5_JOY2_RIGHT_RELEASED() (!(mo5_joy2.dirs & JOY2_RIGHT) && (mo5_joy2.prev_dirs & JOY2_RIGHT))
#define MO5_JOY2_FIRE_RELEASED()  (!mo5_joy2.fire && mo5_joy2.prev_fire)

/* =========================================================================
 * API
 * ========================================================================= */

/**
 * Initialise le PIA extension pour la lecture des manettes.
 * - PORTA en entrées (DDR = 0x00)
 * - PORTB bits 6-7 forcés en entrées (boutons fire), bits 0-5 préservés
 * À appeler une seule fois au démarrage.
 */
void mo5_joystick_init(void);

/**
 * Lit l'état des deux manettes et met à jour mo5_joy1 et mo5_joy2.
 * À appeler UNE FOIS par frame, en début de boucle (après mo5_wait_vbl).
 *
 * Après cet appel :
 *   - dirs et fire contiennent l'état courant
 *   - prev_dirs et prev_fire contiennent l'état du frame précédent
 *   - Les macros _PRESSED / _RELEASED / _HELD sont utilisables
 */
void mo5_joystick_update(void);

#endif /* MO5_JOYSTICK_H */
