# `mo5_sprite_bg` — Rendu sprite transparent sur fond coloré

> API de dessin et déplacement de sprites qui préserve la couleur du décor existant en VRAM. À utiliser quand le fond est un paysage multi-couleurs (herbe, sable, eau...).

---

## Rôle du module

`mo5_sprite_bg` fournit les mêmes deux niveaux d'abstraction que `mo5_sprite`, mais avec un rendu **transparent** : les pixels de fond du sprite laissent apparaître la couleur du décor sous-jacent.

Le mécanisme repose sur des opérations bit à bit sur la banque couleur VRAM :
- **Dessin** : `VRAM |= color_data` — pose uniquement les bits foreground du sprite, sans toucher aux bits background du décor
- **Effacement** : `VRAM &= 0x0F` — remet les bits foreground à 0, les bits background du décor sont conservés

Pour un fond uni noir, utiliser `mo5_sprite.h` à la place (légèrement plus rapide, pas de contrainte sur les assets).

```
┌──────────────────────────────────┐
│          Code de jeu             │
│ mo5_actor_move_bg(&player, x, y) │
├──────────────────────────────────┤
│        API Actor _bg             │  ← à utiliser en priorité
│  mo5_actor_draw/clear/move/clamp_bg │
├──────────────────────────────────┤
│      API bas niveau _bg          │  ← cas spéciaux uniquement
│  mo5_draw/clear/move_sprite_bg   │
├──────────────────────────────────┤
│       mo5_sprite_types.h         │  ← MO5_Sprite, MO5_Actor, MO5_Position
├──────────────────────────────────┤
│          mo5_video.h             │  ← VRAM, PRC, row_offsets
└──────────────────────────────────┘
```

---

## Inclusion

```c
#include "mo5_sprite_bg.h"   // inclut mo5_sprite_types.h et mo5_video.h automatiquement
```

---

## Convention obligatoire sur les assets

Les pixels de **fond** du sprite doivent avoir `foreground = 0x0` dans les données couleur (bits 4–7 de l'octet couleur = 0).

```
Octet couleur MO5 : FFFFBBBB
                    ^^^^
                    Ces bits doivent être 0 pour les pixels de fond du sprite
```

Le script de conversion PNG doit garantir cette contrainte. Utiliser l'option `BG=0` (fond noir, foreground = 0) lors de la conversion :

```bash
make convert IMG=./assets/perso.png BG=0
```

**Si cette convention n'est pas respectée**, les pixels de fond du sprite s'accumulent avec les couleurs du décor et produisent des artefacts visuels.

---

## Limitation importante

**Ne pas superposer deux sprites `_bg`** : les opérations `|=` s'accumulent sur les mêmes octets VRAM et produisent des couleurs incorrectes. Si deux entités peuvent se chevaucher, l'une d'elles doit utiliser le background save (non implémenté dans ce module).

---

## API Actor

### Initialisation

Identique à `mo5_sprite.h` — les structures `MO5_Actor` et `MO5_Sprite` sont partagées.

```c
MO5_Sprite spr = SPRITE_PERSO_INIT;

MO5_Actor player;
player.sprite  = &spr;
player.pos.x   = (SCREEN_WIDTH_BYTES - SPRITE_PERSO_WIDTH_BYTES) / 2;
player.pos.y   = (SCREEN_HEIGHT      - SPRITE_PERSO_HEIGHT)      / 2;
player.old_pos = player.pos;   // ← obligatoire

mo5_actor_draw_bg(&player);    // premier affichage
```

---

### `mo5_actor_draw_bg`

```c
void mo5_actor_draw_bg(const MO5_Actor *actor);
```

Dessine le sprite à `pos` en préservant la couleur du fond.

- Banque couleur : `|=` — pose les bits foreground, conserve les bits background du décor
- Banque forme : écriture directe

À utiliser **uniquement pour le premier affichage**. Dans la boucle de jeu, utiliser `mo5_actor_move_bg`.

---

### `mo5_actor_clear_bg`

```c
void mo5_actor_clear_bg(const MO5_Actor *actor);
```

Efface le sprite à `pos` en restaurant visuellement le fond.

- Banque couleur : `&= 0x0F` — remet les bits foreground à 0, les bits background du décor sont conservés
- Banque forme : non touchée (avec foreground = 0, tous les pixels affichent la couleur background du décor)

Utiliser pour retirer définitivement un acteur de l'écran.

---

### `mo5_actor_move_bg`

```c
void mo5_actor_move_bg(MO5_Actor *actor, int new_x, int new_y);
```

Déplace l'acteur vers `(new_x, new_y)` en préservant le fond coloré :
- **No-op** si la position n'a pas changé
- Met à jour `old_pos` et `pos` automatiquement
- Clear différentiel (`&= 0x0F`) sur la zone hors recouvrement
- Draw (`|=`) complet à la nouvelle position

---

## Pattern boucle de jeu

```c
// Calcul de la nouvelle position
MO5_Position new_pos = player.pos;
switch (key) {
    case KEY_LEFT:  new_pos.x -= 1; break;
    case KEY_RIGHT: new_pos.x += 1; break;
    case KEY_UP:    new_pos.y -= 4; break;
    case KEY_DOWN:  new_pos.y += 4; break;
}

// Clamp sur new_pos AVANT le move
new_pos.x = clamp(new_pos.x, 0, SCREEN_WIDTH_BYTES - player.sprite->width_bytes);
new_pos.y = clamp(new_pos.y, 0, SCREEN_HEIGHT      - player.sprite->height);

// Déplacement optimisé avec préservation du fond
mo5_actor_move_bg(&player, new_pos.x, new_pos.y);
```

---

## API bas niveau

À utiliser uniquement pour les cas spéciaux.

### `mo5_draw_sprite_bg`

```c
void mo5_draw_sprite_bg(int tx, int ty,
                        unsigned char *form_data, unsigned char *color_data,
                        int width_bytes, int height);
```

Banque couleur : `|=`. Banque forme : écriture directe.

### `mo5_clear_sprite_bg`

```c
void mo5_clear_sprite_bg(int tx, int ty, int width_bytes, int height);
```

Banque couleur : `&= 0x0F`. Banque forme non touchée.

### `mo5_move_sprite_bg`

```c
void mo5_move_sprite_bg(int old_tx, int old_ty, int new_tx, int new_ty,
                        unsigned char *form_data, unsigned char *color_data,
                        int width_bytes, int height);
```

Fallback automatique sur `clear_bg` + `draw_bg` si le déplacement est supérieur à la taille du sprite.

---

## Comparaison `mo5_sprite` vs `mo5_sprite_bg`

| | `mo5_sprite` | `mo5_sprite_bg` |
|---|---|---|
| Fond supporté | Uni (noir, `0x00`) | Coloré multi-couleurs |
| Écriture couleur | `=` | `\|=` |
| Effacement couleur | `= 0x00` | `&= 0x0F` |
| Contrainte asset | Aucune | foreground fond = `0x0` |
| Superposition sprites | Libre | Non supportée |
| Vitesse | Légèrement plus rapide | — |

---

## Pièges courants

**Oublier la contrainte foreground = 0x0 sur les assets**
```c
// ❌ pixels de fond avec foreground != 0 → artefacts sur le décor
// Les bits foreground du fond s'accumulent avec ceux du décor via |=

// ✅ générer le sprite avec BG=0 (fond noir, foreground = 0)
// make convert IMG=./assets/perso.png BG=0
```

**Superposer deux sprites `_bg`**
```c
// ❌ les |= s'accumulent → couleurs incorrectes dans la zone de chevauchement
mo5_actor_move_bg(&enemy1, x, y);
mo5_actor_move_bg(&enemy2, x, y);   // si les deux sprites se chevauchent

// ✅ un seul sprite _bg à la fois sur la même zone, ou utiliser le background save
```
---

*Voir `mo5_sprite_h.md` pour le rendu sur fond uni.*
*Voir `mo5_sprite_types_h.md` pour les structures `MO5_Sprite`, `MO5_Actor`, `MO5_Position`.*
*Voir `mo5_video_h.md` pour la palette, les registres et la synchronisation VBL.*
