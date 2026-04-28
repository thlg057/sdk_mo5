/**
 * @file mo5_audio.h
 * @brief Audio — buzzer système et DAC extension.
 *
 * Deux niveaux de son sur MO5 :
 *
 *   1. BUZZER (PIA système $A7C1 bit 0)
 *      Son 1-bit, toujours disponible sur tout MO5.
 *      Bip simple, bloquant ou via séquence.
 *
 *   2. DAC 6 bits (PIA extension $A7CD bits 0-5)
 *      Disponible sur : MO5 avec extension jeux, MO5E, MO6.
 *      Nécessite une initialisation DDR avant usage.
 *      Valeurs 0-63, sortie analogique filtrée.
 *
 * Sources :
 *   - MAME thomson.cpp : buzzer sur writepb_handler() PIA système
 *   - Manuel Technique MO5 p.48-51 : DAC extension
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Thierry Le Got
 */

#ifndef MO5_AUDIO_H
#define MO5_AUDIO_H

/* =========================================================================
 * REGISTRES
 * ========================================================================= */

/** Registre STATUS du moniteur — bit 3 : 0=bip activé, 1=bip désactivé.
 *  Méthode recommandée pour couper le bip clavier (Clefs Pour MO5 p.110). */
#define MO5_STATUS_REG  ((unsigned char *)0x2019)
#define MO5_BEEP_BIT    0x08

/** PIA système PORTB — bit 0 = buzzer son.
 *  Utilisé directement par mo5_beep() pour générer les fréquences.
 *  Source : Manuel Technique MO5 p.40 + MAME thomson.cpp. */
#define BUZZER_REG      ((unsigned char *)0xA7C1)
#define BUZZER_BIT      0x01

/** PIA extension — DAC son 6 bits */
#define DAC_REG         ((unsigned char *)0xA7CD)
#define DAC_DDR_REG     ((unsigned char *)0xA7CD)   /* même adresse, mode DDR */
#define DAC_CRB_REG     ((unsigned char *)0xA7CF)
#define DAC_DDR_MASK    0x3F                         /* bits 0-5 en sorties */
#define DAC_MAX         63

/* =========================================================================
 * API BUZZER
 * ========================================================================= */

/**
 * Coupe le bip clavier (et tout son buzzer système).
 * Force le bit 0 de $A7C1 à 0.
 * À appeler une fois au démarrage du jeu.
 */
void mo5_mute_beep(void);

/**
 * Restaure le bip clavier (remet le bit buzzer à 1).
 * À appeler si on veut rétablir le comportement ROM standard.
 */
void mo5_unmute_beep(void);

/**
 * Émet un bip simple bloquant.
 * Le CPU est occupé pendant toute la durée du bip.
 *
 * @param half_period  Demi-période en nombre de boucles (~cycles/3).
 *                     Valeurs indicatives : 10=aigu, 50=moyen, 150=grave.
 * @param duration     Nombre de cycles complets (aller-retour).
 *                     Valeurs indicatives : 50=court, 200=moyen, 500=long.
 */
void mo5_beep(unsigned char half_period, unsigned char duration);

/**
 * Émet un bip court prédéfini (effet sonore neutre).
 * Équivalent à mo5_beep(30, 80).
 */
void mo5_beep_short(void);

/**
 * Émet un bip d'erreur (son grave et long).
 * Équivalent à mo5_beep(120, 150).
 */
void mo5_beep_error(void);

/**
 * Émet un bip de validation (son aigu et court).
 * Équivalent à mo5_beep(8, 60).
 */
void mo5_beep_ok(void);

/* =========================================================================
 * API DAC
 * ========================================================================= */

/**
 * Initialise le DAC 6 bits de l'extension jeux/musique.
 * Configure le DDR : bits B0-B5 en sorties, accès PORTB activé.
 * À appeler une fois avant tout usage du DAC.
 *
 * Séquence issue du Manuel Technique p.48 :
 *   CLR  $A7CF   ; CRB=0 → accès DDRB
 *   LDD  #$3F04
 *   STA  $A7CD   ; DDRB : B0-B5 en sorties
 *   STB  $A7CF   ; CRB bit2=1 → accès PORTB
 */
void mo5_dac_init(void);

/**
 * Envoie une valeur au DAC.
 * @param val  Échantillon 6 bits (0-63). Les bits 6-7 sont ignorés.
 *             0 = silence, 32 = mi-amplitude, 63 = pleine amplitude.
 */
void mo5_dac_write(unsigned char val);

/**
 * Remet le DAC à 0 (silence).
 */
void mo5_dac_silence(void);

/**
 * Joue une table d'échantillons via le DAC.
 * Bloquant — le CPU est occupé pendant toute la lecture.
 *
 * @param samples   Pointeur vers le tableau d'échantillons (valeurs 0-63).
 * @param count     Nombre d'échantillons.
 * @param period    Attente entre deux échantillons (en nombre de boucles).
 *                  Contrôle la fréquence d'échantillonnage effective.
 *                  0 = aussi rapide que possible (~8 kHz estimé).
 */
void mo5_dac_play(const unsigned char *samples, unsigned int count,
                  unsigned char period);

#endif /* MO5_AUDIO_H */
