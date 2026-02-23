# `mo5_sprite` — Sprites et Acteurs MO5

> Structures de données, pipeline PNG → sprite, API de rendu et gestion des entités de jeu

---

## Rôle du module

`mo5_sprite` fournit deux niveaux d'abstraction pour afficher et déplacer des sprites :

- **API bas niveau** (`mo5_draw_sprite`, `mo5_clear_sprite`, `mo5_move_sprite`) — accès direct pour les cas spéciaux (HUD, tiles de décor)
- **API Actor** (`mo5_actor_*`) — gestion complète d'une entité de jeu avec position et rendu optimisé

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
│          mo5_video               │  ← VRAM, PRC, row_offsets
└──────────────────────────────────┘
```

---

## Inclusion

```c
#include "mo5_sprite.h"   // inclut mo5_video.h automatiquement
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

### `MO5_Sprite`

Données graphiques statiques d'un sprite. **Ressource partageable** — ne jamais dupliquer.

```c
typedef struct {
    unsigned char *form;    // Bitmap 1 bit/pixel (1=forme, 0=fond)
    unsigned char *color;   // Attributs couleur — 1 octet par groupe de 8 pixels (FFFFBBBB)
    int width_bytes;        // Largeur en octets
    int height;             // Hauteur en lignes pixels
} MO5_Sprite;
```

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
    ennemis[i].sprite = &spr_ennemi;   // tous pointent vers le même sprite

// ❌ Mauvais : duplication inutile de 128+ octets par ennemi
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

// Macro d'initialisation — à utiliser dans le code
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

Efface le sprite à `pos`. Utiliser pour retirer définitivement un acteur de l'écran (mort, collecte d'objet...).

---

### `mo5_actor_move`

```c
void mo5_actor_move(MO5_Actor *actor, int new_x, int new_y);
```

Déplace l'acteur vers `(new_x, new_y)` de façon **optimisée** :
- **No-op** si la position n'a pas changé
- Met à jour `old_pos` et `pos` automatiquement
- Ne cleare que la zone qui ne sera pas recouverte par le nouveau dessin

**Gain typique (sprite 16×16, déplacement de 8px) :**

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
int max_x = SCREEN_WIDTH_BYTES - player.sprite->width_bytes;
int max_y = SCREEN_HEIGHT      - player.sprite->height;
if (new_pos.x < 0)      new_pos.x = 0;
if (new_pos.x > max_x)  new_pos.x = max_x;
if (new_pos.y < 0)      new_pos.y = 0;
if (new_pos.y > max_y)  new_pos.y = max_y;

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

### `mo5_clear_sprite`

```c
void mo5_clear_sprite(int tx, int ty, int width_bytes, int height);
```

### `mo5_move_sprite`

```c
void mo5_move_sprite(int old_tx, int old_ty, int new_tx, int new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height);
```

Fallback automatique sur `clear` + `draw` si le déplacement est supérieur à la taille du sprite.

---

## Pièges courants

**Ne pas initialiser `old_pos`**
```c
// ❌ old_pos indéfinie → premier move() peut clearer n'importe où
player.pos.x = 10; player.pos.y = 20;

// ✅
player.pos.x = 10; player.pos.y = 20;
player.old_pos = player.pos;
```

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

---

*Voir `sdk-video.md` pour la palette, les registres et la synchronisation VBL.*
