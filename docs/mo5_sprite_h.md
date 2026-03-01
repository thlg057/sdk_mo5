# `mo5_sprite` — Rendu sprite opaque sur fond uni

> API de dessin et déplacement de sprites pour fond noir (ou uni). Écrit directement en VRAM sans préserver le fond existant.

---

## Rôle du module

`mo5_sprite` fournit deux niveaux d'abstraction pour afficher et déplacer des sprites sur un **fond uni initialisé à `0x00`** :

- **API bas niveau** (`mo5_draw_sprite`, `mo5_clear_sprite`, `mo5_move_sprite`) — accès direct pour les cas spéciaux (HUD, tiles de décor)
- **API Actor** (`mo5_actor_*`) — gestion complète d'une entité de jeu avec position et rendu optimisé

Pour un sprite se déplaçant sur un **décor coloré**, utiliser `mo5_sprite_bg.h` à la place.

```
┌──────────────────────────────────┐
│          Code de jeu             │
│   mo5_actor_move(&player, x, y)  │
├──────────────────────────────────┤
│          API Actor               │  ← à utiliser en priorité
│   mo5_actor_draw/clear/move/clamp│
├──────────────────────────────────┤
│        API bas niveau            │  ← cas spéciaux uniquement
│   mo5_draw/clear/move_sprite     │
├──────────────────────────────────┤
│       mo5_sprite_types.h         │  ← MO5_Sprite, MO5_Actor, MO5_Position
├──────────────────────────────────┤
│          mo5_video.h             │  ← VRAM, PRC, row_offsets
└──────────────────────────────────┘
```

---

## Inclusion

```c
#include "mo5_sprite.h"   // inclut mo5_sprite_types.h et mo5_video.h automatiquement
```

---

## Pipeline PNG → Sprite

### 1. Préparer le PNG

Contraintes du format MO5 :
- Largeur **multiple de 8 pixels** (ajustée automatiquement sinon)
- **2 couleurs maximum** par groupe de 8 pixels horizontaux (clash de couleurs MO5)
- 16 couleurs maximum au total (palette MO5)
- Fond transparent → couleur de fond (noir par défaut)

### 2. Convertir

```bash
make convert IMG=./assets/perso.png
```

Option couleur de fond (couleur MO5, 0–15) :

```bash
make convert IMG=./assets/perso.png BG=4   # fond bleu
```

### 3. Ce que le script génère

Pour `assets/perso.png`, le fichier `assets/perso.h` contiendra :

```c
#define SPRITE_PERSO_WIDTH_BYTES  4
#define SPRITE_PERSO_HEIGHT      32

unsigned char sprite_perso_form[128]  = { /* données bitmap */ };
unsigned char sprite_perso_color[128] = { /* attributs couleur */ };

#define SPRITE_PERSO_INIT \
    { sprite_perso_form, sprite_perso_color, \
      SPRITE_PERSO_WIDTH_BYTES, SPRITE_PERSO_HEIGHT }
```

### 4. Utiliser dans le code

```c
#include "assets/perso.h"

MO5_Sprite sprite_perso = SPRITE_PERSO_INIT;
```

---

## API Actor

### Initialisation

```c
MO5_Sprite spr = SPRITE_PERSO_INIT;

MO5_Actor player;
player.sprite  = &spr;
player.pos.x   = (SCREEN_WIDTH_BYTES - SPRITE_PERSO_WIDTH_BYTES) / 2;
player.pos.y   = (SCREEN_HEIGHT      - SPRITE_PERSO_HEIGHT)      / 2;
player.old_pos = player.pos;   // ← obligatoire

mo5_actor_draw(&player);       // premier affichage
```

---

### `mo5_actor_draw`

```c
void mo5_actor_draw(const MO5_Actor *actor);
```

Dessine le sprite à `pos`. À utiliser **uniquement pour le premier affichage**. Dans la boucle de jeu, utiliser `mo5_actor_move`.

---

### `mo5_actor_clear`

```c
void mo5_actor_clear(const MO5_Actor *actor);
```

Efface le sprite à `pos`. Passe unique sur la banque couleur (fond noir garanti — la banque forme n'est pas touchée). Utiliser pour retirer définitivement un acteur de l'écran.

---

### `mo5_actor_move`

```c
void mo5_actor_move(MO5_Actor *actor, int new_x, int new_y);
```

Déplace l'acteur vers `(new_x, new_y)` de façon **optimisée** :
- **No-op** si la position n'a pas changé
- Met à jour `old_pos` et `pos` automatiquement
- Ne cleare que la zone qui ne sera pas recouverte par le nouveau dessin

**Gain typique (sprite 16×16, déplacement de 1 octet) :**

| Approche | Écritures VRAM | Économie |
|---|---|---|
| `clear` + `draw` naïf | 128 | — |
| `mo5_actor_move` | 96 | ~25% |

```c
// Dans la boucle de jeu
mo5_actor_move(&player, player.pos.x + 1, player.pos.y);
```

---

### `mo5_actor_clamp`

```c
void mo5_actor_clamp(MO5_Actor *actor);
```

Limite `pos` aux bords de l'écran en tenant compte de la taille du sprite.

> À appeler sur les coordonnées calculées **avant** `mo5_actor_move`, pas après.

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

// Déplacement optimisé (no-op si position identique)
mo5_actor_move(&player, new_pos.x, new_pos.y);
```

---

## API bas niveau

À utiliser uniquement pour les cas spéciaux (tiles de décor, HUD, effets).

### `mo5_draw_sprite`

```c
void mo5_draw_sprite(int tx, int ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height);
```

Écrit les banques couleur et forme directement. Le fond du sprite écrase la VRAM existante.

### `mo5_clear_sprite`

```c
void mo5_clear_sprite(int tx, int ty, int width_bytes, int height);
```

Remet la banque couleur à `0x00`. La banque forme n'est pas touchée — avec une couleur `0x00` (foreground et background noirs), le résultat visuel est noir quelle que soit la valeur de la forme.

### `mo5_move_sprite`

```c
void mo5_move_sprite(int old_tx, int old_ty, int new_tx, int new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height);
```

Fallback automatique sur `clear` + `draw` si le déplacement est supérieur à la taille du sprite.

---

## Pièges courants

**Confondre x en octets et x en pixels**
```c
// ❌ x en pixels → sprite affiché au mauvais endroit
player.pos.x = 160;   // 160 octets = bien au-delà de l'écran !

// ✅ x en octets
player.pos.x = 20;    // 20 × 8 = 160 pixels depuis la gauche
```

**Utiliser `mo5_actor_draw` dans la boucle**
```c
// ❌ redessine sans clearer l'ancienne position → fantômes
while (1) { mo5_actor_draw(&player); }

// ✅ rendu différentiel
while (1) { mo5_actor_move(&player, new_x, new_y); }
```

**Clamp après le move**
```c
// ❌ le move a déjà utilisé les coordonnées non clampées
mo5_actor_move(&player, new_x, new_y);
mo5_actor_clamp(&player);   // trop tard

// ✅ clamp sur new_pos avant le move (voir pattern ci-dessus)
```

**Utiliser mo5_sprite sur un fond coloré**
```c
// ❌ la couleur de fond du décor est écrasée par mo5_draw_sprite
mo5_actor_draw(&player);   // sur un paysage coloré → fond abîmé

// ✅ utiliser mo5_sprite_bg.h pour un décor coloré
mo5_actor_draw_bg(&player);
```

---

*Voir `mo5_sprite_bg_h.md` pour le rendu sur fond coloré.*
*Voir `mo5_sprite_types_h.md` pour les structures `MO5_Sprite`, `MO5_Actor`, `MO5_Position`.*
*Voir `mo5_video_h.md` pour la palette, les registres et la synchronisation VBL.*
