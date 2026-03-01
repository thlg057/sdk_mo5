# `mo5_sprite_form` — Rendu sprite forme seule (mode rapide monochrome)

> API de dessin et déplacement de sprites n'écrivant que la banque forme. La banque couleur est initialisée une fois au démarrage et jamais retouchée. Mode le plus rapide du SDK.

---

## Rôle du module

`mo5_sprite_form` affiche des sprites dont tous les pixels partagent la même couleur de premier plan. Seule la **banque forme** est écrite à chaque frame — la banque couleur est fixée une fois par `mo5_video_init` ou `mo5_fill_rect`.

Ce mode est idéal pour :
- Des projectiles, particules ou effets simples monochromes
- Des sprites dont la couleur ne change jamais
- Tout contexte où la réactivité prime sur la richesse visuelle

Pour des sprites multi-couleurs sur fond coloré, utiliser `mo5_sprite_bg.h` ou `mo5_actor_dr.h`.

```
┌──────────────────────────────────┐
│          Code de jeu             │
│ mo5_actor_move_form(&spr, x, y)  │
├──────────────────────────────────┤
│        API Actor _form           │  ← à utiliser en priorité
│  mo5_actor_draw/clear/move_form  │
├──────────────────────────────────┤
│      API bas niveau _form        │  ← cas spéciaux uniquement
│  mo5_draw/clear/move_sprite_form │
├──────────────────────────────────┤
│       mo5_sprite_types.h         │  ← MO5_Sprite, MO5_Actor, MO5_Position
├──────────────────────────────────┤
│          mo5_video.h             │  ← VRAM, PRC, row_offsets
└──────────────────────────────────┘
```

---

## Inclusion

```c
#include "mo5_sprite_form.h"   // inclut mo5_sprite_types.h et mo5_video.h automatiquement
```

---

## Principe de fonctionnement

La banque couleur VRAM stocke pour chaque groupe de 8 pixels : `FFFFBBBB` (foreground sur les bits 4–7, background sur les bits 0–3). En mode `_form`, cette banque est initialisée une seule fois avec la couleur souhaitée, puis **jamais retouchée**.

Seule la banque forme est modifiée à chaque opération :
- **Dessin** : `forme = form_data` — active les pixels du sprite
- **Effacement** : `forme = 0x00` — tous les pixels affichent le background

```
Banque couleur (fixe) :  0x8F  →  bg=gris (8), fg=blanc (F)
Banque forme   (mobile) : 0xFF  →  tous les pixels affichent fg=blanc
                          0x00  →  tous les pixels affichent bg=gris
```

**Prérequis :** initialiser la couleur avec `mo5_video_init` ou `mo5_fill_rect` **avant** le premier dessin.

---

## Initialisation de la couleur

```c
// Fond gris, sprite blanc — pour toute la fenêtre
mo5_video_init(COLOR(C_GRAY, C_WHITE));

// Ou pour une zone spécifique seulement
mo5_fill_rect(10, 20, 4, 32, COLOR(C_BLACK, C_RED));
```

---

## API Actor

### Initialisation

Les structures `MO5_Sprite` et `MO5_Actor` sont les mêmes que pour les autres modules. Seule la banque `color` du sprite est ignorée à l'affichage — la laisser telle quelle.

```c
MO5_Sprite spr = SPRITE_PERSO_INIT;

MO5_Actor bullet;
bullet.sprite  = &spr;
bullet.pos.x   = 20;
bullet.pos.y   = 10;
bullet.old_pos = bullet.pos;   // ← obligatoire

mo5_actor_draw_form(&bullet);   // premier affichage
```

---

### `mo5_actor_draw_form`

```c
void mo5_actor_draw_form(const MO5_Actor *actor);
```

Écrit la banque forme à `pos`. La banque couleur n'est pas touchée.

À utiliser **uniquement pour le premier affichage**. Dans la boucle de jeu, utiliser `mo5_actor_move_form`.

---

### `mo5_actor_clear_form`

```c
void mo5_actor_clear_form(const MO5_Actor *actor);
```

Remet la banque forme à `0x00` sur la zone occupée par le sprite. Tous les pixels affichent le background défini dans la banque couleur.

Utiliser pour retirer définitivement un acteur de l'écran.

---

### `mo5_actor_move_form`

```c
void mo5_actor_move_form(MO5_Actor *actor, unsigned char new_x, unsigned char new_y);
```

Déplace l'acteur vers `(new_x, new_y)` :
- **No-op** si la position n'a pas changé
- Met à jour `old_pos` et `pos` automatiquement
- Clear complet de l'ancienne position, draw à la nouvelle

**Comparaison des écritures VRAM (sprite 32×32, 1 frame) :**

| Mode | Banques écrites | Octets écrits |
|---|---|---|
| `mo5_sprite` (opaque) | couleur + forme | 256 |
| `mo5_sprite_bg` (transparent) | couleur + forme | 128–256 |
| `mo5_sprite_form` (forme seule) | forme uniquement | **128** |

---

## Pattern boucle de jeu

```c
// Calcul de la nouvelle position
MO5_Position new_pos = bullet.pos;
new_pos.y -= BULLET_SPEED;

// Clamp AVANT le move
if (new_pos.y > bullet.pos.y) {   // unsigned : underflow check
    new_pos.y = 0;
}

// Déplacement (no-op si position identique)
mo5_actor_move_form(&bullet, new_pos.x, new_pos.y);
```

---

## API bas niveau

À utiliser uniquement pour les cas spéciaux.

### `mo5_draw_sprite_form`

```c
void mo5_draw_sprite_form(unsigned char tx, unsigned char ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height);
```

Écrit `form_data` dans la banque forme à `(tx, ty)`. La banque couleur n'est pas touchée.

### `mo5_clear_sprite_form`

```c
void mo5_clear_sprite_form(unsigned char tx, unsigned char ty,
                           unsigned char width_bytes, unsigned char height);
```

Remet la banque forme à `0x00` sur la zone spécifiée.

### `mo5_move_sprite_form`

```c
void mo5_move_sprite_form(unsigned char old_tx, unsigned char old_ty,
                          unsigned char new_tx,  unsigned char new_ty,
                          unsigned char *form_data,
                          unsigned char width_bytes, unsigned char height);
```

Clear à l'ancienne position + draw à la nouvelle.

---

## Pièges courants

**Oublier d'initialiser la couleur**
```c
// ❌ banque couleur = 0x00 → foreground et background tous noirs → sprite invisible
mo5_actor_draw_form(&bullet);

// ✅ initialiser la couleur avant le premier draw
mo5_video_init(COLOR(C_BLACK, C_WHITE));
mo5_actor_draw_form(&bullet);
```

**Utiliser sur un fond multi-couleurs**
```c
// ❌ la banque couleur du décor sera utilisée telle quelle → couleur imprévisible
// par zone selon ce qui était dessiné dans la banque couleur

// ✅ pour un fond multi-couleurs, utiliser mo5_sprite_bg.h ou mo5_actor_dr.h
```

**Underflow sur les coordonnées unsigned char**
```c
// ❌ si pos.y = 0 et SPEED = 4 → 0 - 4 = 252 (underflow !)
new_pos.y -= SPEED;

// ✅ tester avant de soustraire
if (new_pos.y >= SPEED) new_pos.y -= SPEED;
else                    new_pos.y = 0;
```

---

*Voir `mo5_sprite_h.md` pour le rendu opaque sur fond uni.*
*Voir `mo5_sprite_bg_h.md` pour le rendu transparent sur fond coloré.*
*Voir `mo5_actor_dr_h.md` pour la gestion de sprites superposés.*
*Voir `mo5_video_h.md` pour `COLOR()`, la palette et `mo5_fill_rect`.*
