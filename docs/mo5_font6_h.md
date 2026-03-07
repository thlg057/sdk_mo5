# `mo5_font6` — Affichage de texte arcade 8×6 pixels

> Police bitmap compacte, 33 lignes de texte sur 200 lignes écran, fond préservé

---

## Rôle du module

`mo5_font6` affiche du texte en mode graphique avec une police 8×6 pixels (1 octet de large, 6 lignes de haut). Le fond du décor est préservé : seuls les pixels forme sont écrits, la couleur du fond n'est pas touchée.

Par rapport à la police 8×8 (`mo5_font8`), elle permet d'afficher **33 lignes de texte** sur les 200 lignes de l'écran au lieu de 25 — utile pour les HUD denses ou les menus.

```
┌─────────────────────┐
│    Code de jeu      │  score, vies, messages
├─────────────────────┤
│    mo5_font6        │  ← ce module  (police 8×6)
│    mo5_font8        │               (police 8×8)
├─────────────────────┤
│    mo5_video.h      │  VRAM, PRC, row_offsets
└─────────────────────┘
```

---

## Inclusion

```c
#include "mo5_font6.h"   // inclut mo5_defs.h automatiquement
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

### `mo5_font6_puts`

```c
void mo5_font6_puts(unsigned char tx, unsigned char ty,
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
// Affichage du score en haut à gauche
mo5_font6_puts(0, 0, "SCORE:", C_YELLOW);
mo5_font6_puts(6, 0, "000100", C_WHITE);

// Message centré
mo5_font6_puts(8, 100, "GAME OVER", C_RED);

// Vies en haut à droite
mo5_font6_puts(35, 0, "VIE:", C_BLUE);
```

> **Astuce** : `sprintf` peut planter sur MO5. Formater les nombres manuellement :
> ```c
> char buf[4];
> buf[0] = '0' + (score / 100);
> buf[1] = '0' + (score % 100 / 10);
> buf[2] = '0' + (score % 10);
> buf[3] = '\0';
> mo5_font6_puts(6, 0, buf, C_WHITE);
> ```

---

### `mo5_font6_clear`

```c
void mo5_font6_clear(unsigned char tx, unsigned char ty, unsigned char len);
```

Efface une zone de texte en remplaçant `len` caractères par des espaces.

| Paramètre | Description |
|---|---|
| `tx` | Position horizontale en octets (0–39) |
| `ty` | Position verticale en pixels (0–199) |
| `len` | Nombre de caractères à effacer |

```c
// Effacer 6 caractères (une valeur de score)
mo5_font6_clear(6, 0, 6);

// Effacer une ligne entière de 40 octets
mo5_font6_clear(0, 0, 40);
```

---

## Comparaison font6 vs font8

| | `mo5_font6` | `mo5_font8` |
|---|---|---|
| Hauteur caractère | 6 pixels | 8 pixels |
| Lignes de texte disponibles | 33 | 25 |
| Usage typique | HUD compact, menus | Titres, messages larges |

---

## Exemple d'utilisation complète

```c
#include "mo5_font6.h"

// Affichage initial du HUD
void hud_draw(unsigned char score, unsigned char lives)
{
    char buf[4];

    mo5_font6_puts(0,  0, "SCORE:", C_YELLOW);
    mo5_font6_puts(35, 0, "VIE:",   C_YELLOW);

    hud_update(score, lives);
}

// Mise à jour des valeurs du HUD
void hud_update(unsigned char score, unsigned char lives)
{
    char buf[4];

    buf[0] = '0' + (score / 100);
    buf[1] = '0' + (score % 100 / 10);
    buf[2] = '0' + (score % 10);
    buf[3] = '\0';
    mo5_font6_puts(6, 0, buf, C_WHITE);

    buf[0] = '0' + lives;
    buf[1] = '\0';
    mo5_font6_puts(39, 0, buf, C_WHITE);
}
```

---

*Voir `mo5_font8_h.md` pour la police 8×8 pixels.*
*Voir `mo5_video_h.md` pour la palette de couleurs (`C_xxx`) et les dimensions écran.*
