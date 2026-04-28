# `mo5_music_swi` — Génération musicale via le moniteur ROM

> Interface haut niveau pour le générateur de notes du moniteur MO5 (SWI $1E). Joue des notes Do-Si avec contrôle du timbre, de l'octave et du tempo.

---

## Rôle du module

`mo5_music_swi` encapsule les appels au SWI $1E du moniteur ROM, qui permet de jouer des notes musicales avec les paramètres suivants : note (Do à Si + silences), durée, octave (1–5), timbre et tempo.

```
┌────────────────────────────────────────┐
│            Code applicatif             │
├────────────────────────────────────────┤
│          mo5_music_swi                 │  ← ce module
│  (initialise registres + appel SWI)   │
├────────────────────────────────────────┤
│  Moniteur ROM SWI $1E                  │
│  Registres $2039–$203F                 │
└────────────────────────────────────────┘
```

---

## Différences avec `mo5_audio`

| | `mo5_audio` | `mo5_music_swi` |
|---|---|---|
| **Mécanisme** | Buzzer PIA direct (bit PB0) | Routine moniteur ROM (SWI $1E) |
| **Notes** | Fréquences libres (half_period) | Do–Si chromatic + silence |
| **Timbre** | Onde carrée pure | Timbre ROM (enveloppe fixe) |
| **Octaves** | Libres | 5 octaves (1–5) |
| **Bloquant** | Oui | Oui |
| **Disponibilité** | Tout MO5 | Tout MO5 |
| **Usage typique** | Effets sonores custom, mélodies libres | Mélodies en notation musicale standard |

Les deux sont bloquants. Pour un jeu, les utiliser uniquement pour des effets courts, hors game loop active.

---

## Inclusion

```c
#include "mo5_music_swi.h"
```

Aucune dépendance interne au SDK.

---

## Registres matériels utilisés

Registres RAM du moniteur configurés avant chaque appel SWI.  
Source : Guide du MO5 p.262-263 et p.275-277.

| Macro | Adresse | Rôle |
|-------|---------|------|
| `MO5_TEMPO_H/L` | `$2039–$203A` | Tempo (1=rapide, 5=std, 255=lent) |
| `MO5_DUREE_H/L` | `$203B–$203C` | Durée de la note |
| `MO5_TIMBRE` | `$203D` | Attaque/timbre (0=legato, 200=staccato) |
| `MO5_OCTAVE_H/L` | `$203E–$203F` | Octave (1–5) |

---

## Codes de notes

```c
MO5_NOTE_SILENCE  /* pause */
MO5_NOTE_DO       /* DO  */
MO5_NOTE_DOS      /* DO# */
MO5_NOTE_RE       /* RE  */
MO5_NOTE_RES      /* RE# */
MO5_NOTE_MI       /* MI  */
MO5_NOTE_FA       /* FA  */
MO5_NOTE_FAS      /* FA# */
MO5_NOTE_SOL      /* SOL */
MO5_NOTE_SOLS     /* SOL# */
MO5_NOTE_LA       /* LA  */
MO5_NOTE_LAS      /* LA# */
MO5_NOTE_SI       /* SI  */
MO5_NOTE_UT       /* DO de l'octave supérieure */
```

---

## Durées

```c
MO5_DUR_RONDE           /* 96 — ronde */
MO5_DUR_BLANCHE_P       /* 72 — blanche pointée */
MO5_DUR_BLANCHE         /* 48 — blanche */
MO5_DUR_NOIRE_P         /* 36 — noire pointée */
MO5_DUR_NOIRE           /* 24 — noire (standard à l'init) */
MO5_DUR_CROCHE_P        /* 18 — croche pointée */
MO5_DUR_CROCHE          /* 12 — croche */
MO5_DUR_DCROCHE_P       /*  9 — double croche pointée */
MO5_DUR_DCROCHE         /*  6 — double croche */
MO5_DUR_TCROCHE_P       /*  5 — triple croche pointée */
MO5_DUR_TCROCHE         /*  3 — triple croche */
/* Triolets */
MO5_DUR_NOIRE_TRIO      /* 16 — noire en triolet (3×16 = blanche) */
MO5_DUR_CROCHE_TRIO     /*  8 — croche en triolet */
MO5_DUR_DCROCHE_TRIO    /*  4 — double croche en triolet */
```

---

## Octaves

```c
MO5_OCT_1   /* grave */
MO5_OCT_2
MO5_OCT_3
MO5_OCT_4   /* référence — LA 440 Hz */
MO5_OCT_5   /* aigu */
```

---

## Tempo

```c
MO5_TEMPO_PRESTISSIMO  /*  1 — très rapide */
MO5_TEMPO_ALLEGRO      /*  4 */
MO5_TEMPO_ALLEGRETTO   /*  5 — standard à l'init */
MO5_TEMPO_MODERATO     /*  8 */
MO5_TEMPO_ANDANTE      /* 16 */
MO5_TEMPO_ADAGIO       /* 32 */
MO5_TEMPO_LENTO        /* 48 */
MO5_TEMPO_LARGO        /* 64 — très lent */
```

---

## Timbre / Attaque

```c
MO5_TIMBRE_LEGATO      /*   0 — son continu */
MO5_TIMBRE_NORMAL      /*  10 — légèrement articulé */
MO5_TIMBRE_DETACHE     /* 100 — détaché */
MO5_TIMBRE_STACCATO    /* 200 — très piqué */
```

---

## Structure `MO5_Note`

```c
typedef struct {
    unsigned char note;    /* MO5_NOTE_xxx */
    unsigned char duree;   /* MO5_DUR_xxx */
    unsigned char octave;  /* MO5_OCT_1 à MO5_OCT_5 */
    unsigned char timbre;  /* MO5_TIMBRE_xxx ou valeur 0-255 */
} MO5_Note;
```

Un tableau de notes se termine par `{ 0, 0, 0, 0 }`.

---

## Fonctions

### `mo5_swi_play_note`

```c
void mo5_swi_play_note(unsigned char note,   unsigned char duree,
                       unsigned char octave, unsigned char timbre,
                       unsigned char tempo);
```

Joue une note unique. Bloquant — retourne quand la note est terminée.

```c
/* DO noire, octave 4, legato, allegro */
mo5_swi_play_note(MO5_NOTE_DO, MO5_DUR_NOIRE, MO5_OCT_4,
                  MO5_TIMBRE_LEGATO, MO5_TEMPO_ALLEGRO);
```

---

### `mo5_swi_silence`

```c
void mo5_swi_silence(unsigned char duree, unsigned char tempo);
```

Joue un silence de la durée indiquée. Utile pour les pauses entre phrases musicales.

```c
mo5_swi_silence(MO5_DUR_NOIRE, MO5_TEMPO_ALLEGRO);
```

---

### `mo5_swi_play_melody`

```c
void mo5_swi_play_melody(const MO5_Note *melody, unsigned char tempo);
```

Joue un tableau de notes entier. Le tableau doit se terminer par `{ 0, 0, 0, 0 }`. Le tempo est appliqué à toutes les notes de la mélodie.

---

## Exemples complets

### Gamme de DO majeur

```c
static const MO5_Note gamme[] = {
    { MO5_NOTE_DO,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_RE,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_MI,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_FA,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_SOL, MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_LA,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_SI,  MO5_DUR_NOIRE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { MO5_NOTE_UT,  MO5_DUR_BLANCHE, MO5_OCT_4, MO5_TIMBRE_LEGATO },
    { 0, 0, 0, 0 }   /* fin */
};

mo5_swi_play_melody(gamme, MO5_TEMPO_MODERATO);
```

### Mélodie avec silences et changements d'octave

```c
static const MO5_Note menuet[] = {
    { MO5_NOTE_SOL, MO5_DUR_NOIRE,   MO5_OCT_4, MO5_TIMBRE_NORMAL  },
    { MO5_NOTE_RE,  MO5_DUR_CROCHE,  MO5_OCT_5, MO5_TIMBRE_NORMAL  },
    { MO5_NOTE_MI,  MO5_DUR_CROCHE,  MO5_OCT_5, MO5_TIMBRE_NORMAL  },
    { MO5_NOTE_FA,  MO5_DUR_NOIRE_P, MO5_OCT_5, MO5_TIMBRE_NORMAL  },
    { MO5_NOTE_MI,  MO5_DUR_CROCHE,  MO5_OCT_5, MO5_TIMBRE_NORMAL  },
    { MO5_NOTE_RE,  MO5_DUR_BLANCHE, MO5_OCT_5, MO5_TIMBRE_LEGATO  },
    { MO5_NOTE_SILENCE, MO5_DUR_NOIRE, MO5_OCT_4, 0                },
    { 0, 0, 0, 0 }
};

mo5_swi_play_melody(menuet, MO5_TEMPO_ALLEGRETTO);
```

### Note unique à l'écran titre

```c
/* Jingle de bienvenue : DO-MI-SOL-DO */
mo5_swi_play_note(MO5_NOTE_DO,  MO5_DUR_CROCHE, MO5_OCT_4, MO5_TIMBRE_NORMAL, MO5_TEMPO_ALLEGRO);
mo5_swi_play_note(MO5_NOTE_MI,  MO5_DUR_CROCHE, MO5_OCT_4, MO5_TIMBRE_NORMAL, MO5_TEMPO_ALLEGRO);
mo5_swi_play_note(MO5_NOTE_SOL, MO5_DUR_CROCHE, MO5_OCT_4, MO5_TIMBRE_NORMAL, MO5_TEMPO_ALLEGRO);
mo5_swi_play_note(MO5_NOTE_UT,  MO5_DUR_NOIRE,  MO5_OCT_4, MO5_TIMBRE_LEGATO, MO5_TEMPO_ALLEGRO);
```

---

## Précautions

**Bloquant** — aucun code ne s'exécute pendant la lecture d'une note. Ne pas appeler dans la game loop principale.

**Timbre fixe** — le générateur ROM produit un timbre unique (enveloppe de synthèse interne). Pour un son différent, utiliser `mo5_audio` (buzzer direct, onde carrée pure).

**Gamme tempérée** — les fréquences sont celles du tempérament égal standard. Pas de micro-intervalles possibles (contrairement au buzzer direct).

---

## Relation avec les autres modules

| Module | Relation |
|--------|----------|
| `mo5_audio` | Alternative complémentaire — buzzer direct, fréquences libres |
| `mo5_video` | Indépendant — peut s'appeler avant/après init vidéo |
| `mo5_defs` | Indépendant — n'utilise pas `mo5_getchar` ni `mo5_putchar` |

---

*Voir `mo5_hardware_reference.md` sections 11.2 et 10 pour les détails complets des registres SWI musique.*  
*Voir `mo5_audio_h.md` pour le buzzer direct et le DAC.*
