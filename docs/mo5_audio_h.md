# `mo5_audio` — Buzzer système et DAC extension

> Son sur MO5 : bip 1-bit via le PIA système, et DAC 6 bits via l'extension jeux/musique.

---

## Rôle du module

`mo5_audio` encapsule les deux niveaux de son disponibles sur MO5 :

```
┌─────────────────────────────────────────┐
│            Code de jeu                  │
├─────────────────────────────────────────┤
│  mo5_audio  (buzzer + DAC)              │  ← ce module
├──────────────────┬──────────────────────┤
│  PIA sys $A7C1   │  PIA ext $A7CD       │
│  Buzzer 1-bit    │  DAC 6 bits          │
└──────────────────┴──────────────────────┘
```

| Canal | Registre | Disponibilité | Qualité |
|-------|----------|---------------|---------|
| Buzzer | `$A7C1` bit 0 | Tout MO5 | 1 bit (bip) |
| DAC | `$A7CD` bits 0–5 | MO5+extension, MO5E, MO6 | 6 bits (64 niveaux) |

---

## Inclusion

```c
#include "mo5_audio.h"
```

Aucune dépendance interne au SDK.

---

## Registres matériels

| Macro | Adresse | Rôle |
|-------|---------|------|
| `BUZZER_REG` | `$A7C1` | PIA système PORTB — bit 0 = buzzer |
| `BUZZER_BIT` | `0x01` | Masque du bit buzzer |
| `DAC_REG` | `$A7CD` | PIA extension PORTB — DAC son |
| `DAC_CRB_REG` | `$A7CF` | PIA extension CRB — contrôle DDR/PORTB |
| `DAC_DDR_MASK` | `0x3F` | Bits B0–B5 en sorties |
| `DAC_MAX` | `63` | Valeur maximale du DAC |

> ⚠️ `BUZZER_REG` partage le registre `$A7C1` avec la matrice clavier.
> Les fonctions du module utilisent un read-modify-write pour préserver les autres bits.

---

## API Buzzer

Le buzzer est un son 1-bit généré en faisant basculer le bit 0 de `$A7C1`.
Il est **toujours disponible** sur tout MO5, sans initialisation préalable.

> ⚠️ Toutes les fonctions `mo5_beep*` sont **bloquantes** — le CPU est occupé
> pendant toute la durée du bip. Ne pas appeler depuis le VBL si le budget
> cycles est serré.

---

### `mo5_mute_beep`

```c
void mo5_mute_beep(void);
```

Coupe le bip clavier et tout son buzzer système. Force le bit 0 de `$A7C1` à 0.

**À appeler une fois au démarrage du jeu**, avant la boucle principale, pour supprimer le bip ROM qui sonne à chaque appui de touche.

```c
mo5_video_init(COLOR(C_BLACK, C_BLACK));
mo5_mute_beep();   // ← supprimer le bip clavier
```

---

### `mo5_unmute_beep`

```c
void mo5_unmute_beep(void);
```

Restaure le bip clavier en remettant le bit buzzer à 1. À appeler si on veut rétablir le comportement standard de la ROM après une séquence de jeu.

---

### `mo5_beep`

```c
void mo5_beep(unsigned char half_period, unsigned char duration);
```

Émet un bip de fréquence et durée contrôlées.

| Paramètre | Description |
|-----------|-------------|
| `half_period` | Demi-période en nombre de boucles. Contrôle la hauteur du son. |
| `duration` | Nombre de cycles complets (aller-retour du bit). Contrôle la durée. |

**Valeurs indicatives** (à calibrer sur émulateur) :

| `half_period` | Hauteur perçue |
|:---:|---|
| 8–15 | Aigu |
| 30–60 | Médium |
| 100–150 | Grave |

| `duration` | Durée perçue |
|:---:|---|
| 50–80 | Court |
| 150–200 | Moyen |
| 400–500 | Long |

> Ces valeurs sont des estimations. Le timing exact dépend du code généré
> par cmoc — mesurer sur émulateur pour calibrer.

---

### `mo5_beep_short`

```c
void mo5_beep_short(void);
```

Bip court et médium (son neutre). Équivalent à `mo5_beep(30, 80)`.
Usage : confirmation d'action, tick d'interface.

---

### `mo5_beep_error`

```c
void mo5_beep_error(void);
```

Bip grave et long (son d'erreur). Équivalent à `mo5_beep(120, 150)`.
Usage : collision mortelle, action invalide.

---

### `mo5_beep_ok`

```c
void mo5_beep_ok(void);
```

Bip aigu et court (son de validation). Équivalent à `mo5_beep(8, 60)`.
Usage : score, ramassage d'objet, niveau validé.

---

## API DAC

Le DAC 6 bits est disponible via l'extension jeux/musique (`$A7CD`).
Il permet 64 niveaux d'amplitude — suffisant pour des effets sonores de qualité
ou une musique simple.

> ⚠️ **`mo5_dac_init()` est obligatoire** avant tout appel DAC.
> Sans initialisation DDR, le port est en entrée et les écritures n'ont aucun effet.

---

### `mo5_dac_init`

```c
void mo5_dac_init(void);
```

Initialise le DAC : configure les bits B0–B5 de `$A7CD` en sorties via le DDR,
puis active l'accès au PORTB. À appeler **une seule fois au démarrage**.

Séquence issue du Manuel Technique MO5 p.48 :

```asm
CLR  $A7CF       ; CRB = 0  → accès DDRB
LDD  #$3F04
STA  $A7CD       ; DDRB : bits B0-B5 en sorties
STB  $A7CF       ; CRB bit2 = 1 → accès PORTB
```

```c
// Démarrage typique avec DAC
mo5_video_init(COLOR(C_BLACK, C_BLACK));
mo5_mute_beep();
mo5_dac_init();
```

---

### `mo5_dac_write`

```c
void mo5_dac_write(unsigned char val);
```

Envoie un échantillon au DAC. Les bits 6–7 sont masqués automatiquement.

| Valeur | Amplitude |
|:------:|-----------|
| `0` | Silence (niveau bas) |
| `32` | Mi-amplitude |
| `63` | Pleine amplitude (~450 mV) |

```c
mo5_dac_write(0);    /* silence */
mo5_dac_write(32);   /* mi-amplitude */
mo5_dac_write(63);   /* maximum */
```

---

### `mo5_dac_silence`

```c
void mo5_dac_silence(void);
```

Remet le DAC à 0. À appeler après une séquence sonore pour éviter
un résidu de tension en sortie.

---

### `mo5_dac_play`

```c
void mo5_dac_play(const unsigned char *samples, unsigned int count,
                  unsigned char period);
```

Joue une table d'échantillons via le DAC. **Bloquant** — le CPU est occupé
pendant toute la lecture.

| Paramètre | Description |
|-----------|-------------|
| `samples` | Tableau d'échantillons (valeurs 0–63) |
| `count` | Nombre d'échantillons à jouer |
| `period` | Attente inter-échantillons en boucles. `0` = maximum (~8 kHz estimé) |

La fonction appelle automatiquement `mo5_dac_silence()` à la fin de la lecture.

```c
/* Exemple : onde triangulaire simple */
static const unsigned char triangle[] = {
    0, 8, 16, 24, 32, 40, 48, 56, 63,
    56, 48, 40, 32, 24, 16, 8, 0
};

mo5_dac_play(triangle, 17, 10);
```

> Les données `samples` doivent être déclarées en mémoire statique (`static` ou
> globale) — pas sur la pile, qui est limitée sur 6809.

---

## Patterns d'utilisation

### Démarrage typique

```c
mo5_video_init(COLOR(C_BLACK, C_BLACK));
mo5_mute_beep();     /* supprimer bip ROM */
mo5_dac_init();      /* si extension DAC disponible */
```

### Effets sonores simples (buzzer)

```c
/* Score */
mo5_beep_ok();

/* Mort du joueur */
mo5_beep_error();

/* Son personnalisé */
mo5_beep(20, 120);
```

### Effets sonores DAC

```c
static const unsigned char sfx_explosion[] = {
    63, 55, 48, 40, 32, 24, 16, 8, 4, 2, 0
};

mo5_dac_play(sfx_explosion, 11, 5);
```

### Son en tâche de fond (avancé)

`mo5_dac_play` est bloquant. Pour du son pendant le jeu sans bloquer,
il faut un player appelé depuis le VBL avec un pointeur global sur la
table courante — cette évolution n'est pas encore dans le SDK.

---

## Relation avec les autres modules

| Module | Relation |
|--------|----------|
| `mo5_video` | Indépendant — peut être utilisé sans vidéo |
| `mo5_defs` | Aucune dépendance |
| `mo5_sprite` | Aucune dépendance |

---

*Voir `mo5_hardware_reference.md` pour les détails des registres PIA et la source MAME.*
