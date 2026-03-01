# `mo5_sprite_types` — Structures partagées du moteur sprite MO5

> Types de base utilisés par `mo5_sprite.h` et `mo5_sprite_bg.h`. Ne pas inclure directement dans le code de jeu.

---

## Rôle du module

`mo5_sprite_types.h` centralise les structures de données communes aux deux moteurs de rendu sprite. Il est inclus automatiquement par `mo5_sprite.h` et `mo5_sprite_bg.h` — le code de jeu n'a jamais besoin de l'inclure directement.

```
┌─────────────────────────┬─────────────────────────┐
│      mo5_sprite.h       │     mo5_sprite_bg.h      │
│   (fond uni, opaque)    │  (fond coloré, transparent) │
├─────────────────────────┴─────────────────────────┤
│              mo5_sprite_types.h                    │  ← ce fichier
├───────────────────────────────────────────────────┤
│                   mo5_video.h                      │
└───────────────────────────────────────────────────┘
```

---

## Inclusion

```c
// Ne pas inclure directement. Utiliser selon le contexte :
#include "mo5_sprite.h"      // fond uni noir
#include "mo5_sprite_bg.h"   // fond coloré (paysage, décor...)
```

---

## Structures

### `MO5_Position`

```c
typedef struct {
    int x;   // en octets  (1 octet = 8 pixels horizontaux)
    int y;   // en pixels
} MO5_Position;
```

> **Attention :** `x` est en **octets**, pas en pixels. Un sprite de 16px de large centré sur l'écran (40 octets) : `x = (40 - 2) / 2 = 19`, pas `(320 - 16) / 2`.

---

### `MO5_Sprite`

Données graphiques statiques d'un sprite. **Ressource partageable** — ne jamais dupliquer les tableaux `form`/`color`, utiliser un pointeur vers une instance unique.

```c
typedef struct {
    unsigned char *form;    // Bitmap 1 bit/pixel (1=forme, 0=fond)
    unsigned char *color;   // Attributs couleur — 1 octet par groupe de 8 pixels (FFFFBBBB)
    int width_bytes;        // Largeur en octets
    int height;             // Hauteur en lignes pixels
} MO5_Sprite;
```

Généré automatiquement depuis un PNG via `make convert IMG=fichier.png`. Initialiser avec la macro `SPRITE_XXX_INIT` définie dans le `.h` généré.

---

### `MO5_Actor`

Entité du jeu : un sprite + une position courante + l'ancienne position.

```c
typedef struct {
    const MO5_Sprite *sprite;   // Pointeur partageable vers les données graphiques
    MO5_Position      pos;      // Position courante
    MO5_Position      old_pos;  // Position précédente (gérée automatiquement)
} MO5_Actor;
```

**Plusieurs acteurs, un seul sprite :**

```c
// ✅ Correct : 5 ennemis, données graphiques chargées une seule fois
MO5_Sprite spr_ennemi = SPRITE_ENNEMI_INIT;

MO5_Actor ennemis[5];
for (i = 0; i < 5; i++)
    ennemis[i].sprite = &spr_ennemi;

// ❌ Mauvais : duplication inutile de 128+ octets par ennemi
```

---

## Piège courant

**Ne pas initialiser `old_pos`**

```c
// ❌ old_pos indéfinie → premier move() peut clearer n'importe où
player.pos.x = 10; player.pos.y = 20;

// ✅
player.pos.x = 10; player.pos.y = 20;
player.old_pos = player.pos;   // ← obligatoire
```

---

*Voir `mo5_sprite_h.md` et `mo5_sprite_bg_h.md` pour les API de rendu.*
