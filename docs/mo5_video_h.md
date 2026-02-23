# `mo5_video` — Couche hardware MO5

> Registres matériels, palette, dimensions écran, synchronisation VBL, utilitaires vidéo

---

## Rôle du module

`mo5_video` est la **fondation du SDK graphique**. Il encapsule tout ce qui concerne le hardware vidéo du MO5 : accès à la VRAM, palette de couleurs, dimensions écran et synchronisation sur le retour de trame.

Tous les autres modules graphiques (`mo5_sprite`, etc.) dépendent de `mo5_video`.

```
┌─────────────────────┐
│     mo5_sprite      │  sprites, acteurs
├─────────────────────┤
│     mo5_video       │  ← ce module
├─────────────────────┤
│  Hardware MO5/VRAM  │
└─────────────────────┘
```

---

## Inclusion

```c
#include "mo5_video.h"
```

`mo5_sprite.h` l'inclut automatiquement — pas besoin de le répéter si tu utilises déjà les sprites.

---

## Registres matériels

| Macro | Adresse | Rôle |
|---|---|---|
| `PRC` | `$A7C0` | Sélection banque VRAM (bit0 : `0`=couleur, `1`=forme) |
| `VIDEO_REG` | `$A7E7` | Registre mode vidéo / status VBL |
| `VRAM` | `$0000` | Base de la mémoire vidéo |

**Banques VRAM** — le MO5 utilise deux banques superposées accessibles via le bit 0 de `PRC` :

```c
*PRC &= ~0x01;   // Sélectionner banque COULEUR
*PRC |=  0x01;   // Sélectionner banque FORME
```

Ne jamais accéder à la VRAM sans avoir sélectionné la bonne banque au préalable.

---

## Palette (16 couleurs)

| Constante | Valeur | Couleur |
|---|---|---|
| `C_BLACK` | 0 | Noir |
| `C_RED` | 1 | Rouge |
| `C_GREEN` | 2 | Vert |
| `C_YELLOW` | 3 | Jaune |
| `C_BLUE` | 4 | Bleu |
| `C_MAGENTA` | 5 | Magenta |
| `C_CYAN` | 6 | Cyan |
| `C_WHITE` | 7 | Blanc |
| `C_GRAY` | 8 | Gris |
| `C_LIGHT_RED` | 9 | Rouge clair |
| `C_LIGHT_GREEN` | 10 | Vert clair |
| `C_LIGHT_YELLOW` | 11 | Jaune clair |
| `C_LIGHT_BLUE` | 12 | Bleu clair |
| `C_PURPLE` | 13 | Violet |
| `C_LIGHT_CYAN` | 14 | Cyan clair |
| `C_ORANGE` | 15 | Orange |

### Macro `COLOR`

Construit un octet couleur MO5 au format `FFFFBBBB` (forme bits 4–7, fond bits 0–3) :

```c
#define COLOR(bg, fg)

// Exemples
COLOR(C_BLACK,  C_WHITE)    // texte blanc sur fond noir
COLOR(C_BLUE,   C_YELLOW)   // jaune sur bleu
COLOR(C_BLACK,  C_BLACK)    // écran noir (effacement)
```

---

## Dimensions écran

```c
#define SCREEN_WIDTH_BYTES  40    // 320 pixels ÷ 8 pixels/octet
#define SCREEN_HEIGHT      200    // lignes pixels
#define SCREEN_SIZE_BYTES  8000   // 40 × 200
```

### Table `row_offsets`

```c
extern unsigned int row_offsets[SCREEN_HEIGHT];
```

Table précalculée par `mo5_video_init`. Donne l'offset de chaque ligne dans la VRAM sans multiplication :

```c
// Sans table (coûteux — multiplication à chaque accès)
offset = y * 40 + x;

// Avec table (rapide — simple accès indexé)
offset = row_offsets[y] + x;
```

---

## Fonctions

### `mo5_video_init`

```c
void mo5_video_init(unsigned char color);
```

Initialise le mode graphique. **À appeler une seule fois au démarrage**, avant tout autre appel du SDK.

- Précalcule `row_offsets`
- Active le mode bitmap
- Remplit la banque couleur avec `color`
- Efface la banque forme (zéros)

```c
// Démarrage typique
mo5_video_init(COLOR(C_BLACK, C_BLACK));
```

---

### `mo5_wait_vbl`

```c
void mo5_wait_vbl(void);
```

Attend le prochain retour de trame vertical (VBL). Le MO5 fonctionne en PAL à **50 Hz**, soit une trame toutes les 20 ms avec **~20 000 cycles CPU** disponibles par frame.

**À placer en début de boucle principale** pour synchroniser le jeu sur l'affichage et éviter le tearing.

```c
while (1) {
    mo5_wait_vbl();       // ← synchronisation trame
    read_inputs();
    update_logic();
    draw();
}
```

> La fonction attend la fin du VBL courant si on est déjà dedans, puis le début du prochain — la synchronisation est propre quelle que soit la durée de la frame précédente.

---

### `mo5_clear_screen`

```c
void mo5_clear_screen(unsigned char color);
```

Remplit tout l'écran avec une couleur uniforme. Utile lors des transitions entre niveaux ou pour réinitialiser l'affichage.

```c
mo5_clear_screen(COLOR(C_BLACK, C_BLACK));   // écran noir
mo5_clear_screen(COLOR(C_BLUE,  C_BLUE));    // fond bleu
```

---

### `mo5_fill_rect`

```c
void mo5_fill_rect(int tx, int ty, int w, int h, unsigned char color);
```

Remplit un rectangle de l'écran avec une couleur uniforme. Utile pour les zones de score, les barres de vie, les fonds de niveaux par blocs.

| Paramètre | Unité |
|---|---|
| `tx` | octets (1 octet = 8 pixels) |
| `ty` | lignes pixels |
| `w` | octets |
| `h` | lignes pixels |

```c
// Zone de score en bas de l'écran : toute la largeur, 8 lignes
mo5_fill_rect(0, 192, 40, 8, COLOR(C_BLACK, C_WHITE));

// Bloc de fond 32×32 pixels à la position (80px, 40px)
mo5_fill_rect(10, 40, 4, 32, COLOR(C_BLUE, C_BLUE));
```

---

## Exemple d'utilisation complète

```c
#include "mo5_video.h"

int main(void) {
    // Initialisation obligatoire
    mo5_video_init(COLOR(C_BLACK, C_BLACK));

    // Zone de HUD en bas
    mo5_fill_rect(0, 192, 40, 8, COLOR(C_GRAY, C_WHITE));

    // Boucle principale synchronisée sur le VBL
    while (1) {
        mo5_wait_vbl();

        // ... logique de jeu et affichage ...
    }

    return 0;
}
```

---

*Voir `sdk-sprite.md` pour la gestion des sprites et acteurs.*
