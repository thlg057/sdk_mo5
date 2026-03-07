# `mo5_font8` — Affichage de texte arcade 8×8 pixels

> Police bitmap standard, 25 lignes de texte sur 200 lignes écran, fond préservé

---

## Rôle du module

`mo5_font8` affiche du texte en mode graphique avec une police 8×8 pixels (1 octet de large, 8 lignes de haut). Le fond du décor est préservé : seuls les pixels forme sont écrits, la couleur du fond n'est pas touchée.

Par rapport à la police 8×6 (`mo5_font6`), les caractères sont plus grands et plus lisibles — adaptés aux titres, écrans de menu et messages importants.

```
┌─────────────────────┐
│    Code de jeu      │  titres, menus, messages
├─────────────────────┤
│    mo5_font8        │  ← ce module  (police 8×8)
│    mo5_font6        │               (police 8×6)
├─────────────────────┤
│    mo5_video.h      │  VRAM, PRC, row_offsets
└─────────────────────┘
```

---

## Inclusion

```c
#include "mo5_font8.h"   // inclut mo5_defs.h automatiquement
```

---

## Système de coordonnées

| Paramètre | Unité | Plage |
|---|---|---|
| `tx` | octets (1 octet = 8 pixels horizontaux) | 0–39 |
| `ty` | lignes pixels | 0–199 |

Un retour à la ligne automatique est effectué si `tx` dépasse 39 en cours de chaîne.

---

## Fonctions

### `mo5_font8_puts`

```c
void mo5_font8_puts(unsigned char tx, unsigned char ty,
                    const char *s, unsigned char fg_color);
```

Dessine la chaîne `s` à la position `(tx, ty)` en préservant la couleur du fond.

| Paramètre | Description |
|---|---|
| `tx` | Position horizontale en octets (0–39) |
| `ty` | Position verticale en pixels (0–199) |
| `s` | Chaîne terminée par `\0` |
| `fg_color` | Couleur de forme — constantes `C_xxx` de `mo5_defs.h` |

```c
// Titre centré en haut de l'écran
mo5_font8_puts(10, 16, "SPACE INVADERS", C_CYAN);

// Message de victoire au centre
mo5_font8_puts(12, 96, "VICTOIRE !", C_YELLOW);

// Instructions en bas
mo5_font8_puts(4, 184, "APPUYER SUR UNE TOUCHE", C_WHITE);
```

---

### `mo5_font8_clear`

```c
void mo5_font8_clear(unsigned char tx, unsigned char ty, unsigned char len);
```

Efface une zone de texte en remplaçant `len` caractères par des espaces.

| Paramètre | Description |
|---|---|
| `tx` | Position horizontale en octets (0–39) |
| `ty` | Position verticale en pixels (0–199) |
| `len` | Nombre de caractères à effacer |

```c
// Effacer un message de 10 caractères
mo5_font8_clear(12, 96, 10);

// Effacer une ligne entière
mo5_font8_clear(0, 16, 40);
```

---

## Comparaison font8 vs font6

| | `mo5_font8` | `mo5_font6` |
|---|---|---|
| Hauteur caractère | 8 pixels | 6 pixels |
| Lignes de texte disponibles | 25 | 33 |
| Usage typique | Titres, messages larges | HUD compact, menus |

---

## Exemple d'utilisation complète

```c
#include "mo5_font8.h"

void screen_title(void)
{
    mo5_font8_puts(10,  80, "SPACE INVADERS", C_CYAN);
    mo5_font8_puts(8,  104, "APPUYER SUR UNE TOUCHE", C_WHITE);
    mo5_wait_for_key();
    mo5_font8_clear(10,  80, 14);
    mo5_font8_clear(8,  104, 22);
}

void screen_game_over(void)
{
    mo5_font8_puts(13, 96, "GAME OVER", C_RED);
    mo5_wait_for_key();
}

void screen_victory(void)
{
    mo5_font8_puts(14, 96, "VICTOIRE !", C_YELLOW);
    mo5_wait_for_key();
}
```

---

*Voir `mo5_font6_h.md` pour la police 8×6 pixels (HUD compact).*
*Voir `mo5_video_h.md` pour la palette de couleurs (`C_xxx`) et les dimensions écran.*
