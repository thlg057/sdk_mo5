# `mo5_actor_dr` — Sprites avec dirty rectangle (save/draw/restore)

> Moteur de sprites avancé avec sauvegarde et restauration automatique du fond. Permet la superposition de sprites et le rendu sur décors quelconques sans artefacts.

---

## Rôle du module

`mo5_actor_dr` résout deux problèmes que les autres modules ne gèrent pas :

1. **Sprites superposés** — deux sprites `_bg` qui se chevauchent produisent des couleurs incorrectes. `mo5_actor_dr` restaure le fond exact avant chaque redraw, rendant la superposition transparente.
2. **Décor avec banque forme** — `mo5_sprite_bg` préserve la banque couleur mais écrase la banque forme. `mo5_actor_dr` sauvegarde et restaure les **deux** banques.

Le principe : avant de dessiner, chaque acteur sauvegarde les octets VRAM qu'il va occuper. En début de frame, il restaure ces octets — le décor réapparaît exactement comme avant.

```
┌──────────────────────────────────────┐
│            Code de jeu               │
│  mo5_actor_dr_move(&actor, x, y)     │
├──────────────────────────────────────┤
│           MO5_Actor_DR               │  ← structure avec buffers de sauvegarde
│  init / restore / save_draw / move   │
├──────────────────────────────────────┤
│         mo5_sprite_bg.h              │  ← dessin transparent sous-jacent
├──────────────────────────────────────┤
│         mo5_sprite_types.h           │
├──────────────────────────────────────┤
│            mo5_video.h               │
└──────────────────────────────────────┘
```

---

## Inclusion

```c
#include "mo5_actor_dr.h"   // inclut mo5_sprite_bg.h et ses dépendances
```

---

## Contrainte de taille

Les buffers de sauvegarde sont **statiques** (alloués dans la structure) :

```c
#define MO5_DR_MAX_WIDTH   4    // 32 pixels maximum
#define MO5_DR_MAX_HEIGHT  32
#define MO5_DR_SAVE_SIZE   128  // octets par banque
```

Un sprite `MO5_Actor_DR` occupe en mémoire : `128 + 128 = 256 octets` de buffers + les pointeurs et la position.

> Sprites plus grands que 32×32 pixels : utiliser `mo5_sprite_bg.h` directement (pas de sauvegarde).

---

## Structure `MO5_Actor_DR`

```c
typedef struct {
    const MO5_Sprite *sprite;               // données graphiques (partageable)
    MO5_Position      pos;                  // position courante
    unsigned char     save_color[128];      // VRAM couleur sauvegardée
    unsigned char     save_form[128];       // VRAM forme sauvegardée
} MO5_Actor_DR;
```

Pas de `old_pos` — le dirty rectangle sauvegarde et restaure toujours `pos`, sans avoir besoin de tracker l'ancienne position.

---

## Séquence obligatoire par frame

L'ordre des appels est critique pour la superposition correcte des sprites :

```
Début de frame :
  mo5_actor_dr_restore(&sprite_B)   ← ordre INVERSE du draw
  mo5_actor_dr_restore(&sprite_A)

Logique du jeu :
  mo5_actor_dr_move(&sprite_A, new_x, new_y)
  mo5_actor_dr_move(&sprite_B, new_x, new_y)

Fin de frame :
  mo5_actor_dr_save_draw(&sprite_A)  ← ordre NORMAL
  mo5_actor_dr_save_draw(&sprite_B)  ← sprite_B passe devant sprite_A
```

**Pourquoi l'ordre inverse au restore ?**

Si A est dessiné avant B (B est devant), la zone de chevauchement est dans le buffer de B. En restaurant B en premier, puis A, les deux zones sont correctement restaurées sans interférence.

---

## API

### `mo5_actor_dr_init`

```c
void mo5_actor_dr_init(MO5_Actor_DR *actor, const MO5_Sprite *sprite,
                       unsigned char x, unsigned char y);
```

Initialise l'acteur, sauvegarde la VRAM à `(x, y)` et dessine le sprite. À appeler **une seule fois** avant la boucle de jeu.

```c
MO5_Sprite spr = SPRITE_PERSO_INIT;
MO5_Actor_DR player;

mo5_actor_dr_init(&player, &spr, 20, 84);
```

---

### `mo5_actor_dr_restore`

```c
void mo5_actor_dr_restore(MO5_Actor_DR *actor);
```

Restaure les deux banques VRAM (couleur + forme) à la position courante. Le sprite disparaît, le fond réapparaît exactement.

À appeler en **début de frame**, dans l'ordre **inverse** du draw.

---

### `mo5_actor_dr_save_draw`

```c
void mo5_actor_dr_save_draw(MO5_Actor_DR *actor);
```

Sauvegarde les deux banques VRAM à la position courante, puis dessine le sprite. Les pixels transparents (fg = 0) laissent apparaître le fond.

À appeler en **fin de frame**, dans l'ordre **normal** (du fond vers le premier plan).

---

### `mo5_actor_dr_move`

```c
void mo5_actor_dr_move(MO5_Actor_DR *actor, unsigned char x, unsigned char y);
```

Met à jour la position de l'acteur. Pris en compte au prochain `mo5_actor_dr_save_draw`. **Le clamp est à gérer côté jeu** avant l'appel.

---

## Pattern boucle de jeu (deux sprites)

```c
// Initialisation (une fois)
mo5_actor_dr_init(&player, &spr_player, 20, 84);
mo5_actor_dr_init(&enemy,  &spr_enemy,  30, 40);

// Boucle
while (1) {
    mo5_wait_vbl();

    // 1. Restore — ordre inverse (enemy devant, donc restore enemy en premier)
    mo5_actor_dr_restore(&enemy);
    mo5_actor_dr_restore(&player);

    // 2. Logique
    key = mo5_wait_for_key();
    update_position_from_key(key, &new_pos, max_x, max_y);
    mo5_actor_dr_move(&player, new_pos.x, new_pos.y);

    // 3. Save + Draw — ordre normal (player d'abord, enemy devant)
    mo5_actor_dr_save_draw(&player);
    mo5_actor_dr_save_draw(&enemy);
}
```

---

## Comparaison des modules sprite

| | `mo5_sprite` | `mo5_sprite_form` | `mo5_sprite_bg` | `mo5_actor_dr` |
|---|---|---|---|---|
| Fond supporté | Uni noir | Uni coloré | Multi-couleurs | Quelconque |
| Banques écrites | couleur + forme | forme seule | couleur + forme | couleur + forme |
| Sprites superposés | ✗ | ✗ | ✗ | ✅ |
| Décor avec forme | ✗ | ✅ | ✗ | ✅ |
| Mémoire extra | aucune | aucune | aucune | 256 o/sprite |
| Vitesse | rapide | très rapide | rapide | plus lent |
| Taille max sprite | illimitée | illimitée | illimitée | 32×32 px |

---

## Pièges courants

**Oublier l'ordre inverse au restore**
```c
// ❌ ordre identique → artefacts dans la zone de chevauchement
mo5_actor_dr_restore(&player);
mo5_actor_dr_restore(&enemy);
mo5_actor_dr_save_draw(&player);
mo5_actor_dr_save_draw(&enemy);

// ✅ restore en ordre inverse
mo5_actor_dr_restore(&enemy);
mo5_actor_dr_restore(&player);
mo5_actor_dr_save_draw(&player);
mo5_actor_dr_save_draw(&enemy);
```

**Appeler move après save_draw**
```c
// ❌ la sauvegarde est déjà faite à l'ancienne position
mo5_actor_dr_save_draw(&player);
mo5_actor_dr_move(&player, new_x, new_y);   // trop tard

// ✅ move avant save_draw
mo5_actor_dr_move(&player, new_x, new_y);
mo5_actor_dr_save_draw(&player);
```

**Underflow sur les coordonnées unsigned char**
```c
// ❌ pos.x = 0, dx = 1 → 0 - 1 = 255 (underflow !)
new_pos.x -= SPEED;

// ✅ tester avant de soustraire
if (new_pos.x >= SPEED) new_pos.x -= SPEED;
else                    new_pos.x = 0;
```

---

*Voir `mo5_sprite_bg_h.md` pour le rendu transparent sans sauvegarde.*
*Voir `mo5_sprite_form_h.md` pour le rendu forme seule (le plus rapide).*
*Voir `mo5_sprite_types_h.md` pour les structures `MO5_Sprite` et `MO5_Position`.*
*Voir `mo5_video_h.md` pour la palette, les registres et la synchronisation VBL.*
