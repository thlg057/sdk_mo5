# SDK MO5

Une bibliothèque C légère pour créer des programmes pour le Thomson MO5,
conçue pour être utilisée avec le compilateur **CMOC**.

## 📁 Structure du projet

- `src/` : Fichiers sources (`.c`)
- `include/` : En-têtes publics (`.h`)
- `obj/` : Répertoire temporaire pour les fichiers objets (généré à la compilation)
- `lib/` : Contient la bibliothèque statique finale `libsdk_mo5.a`
- `docs/` : Documentation Markdown des modules (utilisée par le serveur RAG MO5)
- `scripts/` : Scripts utilitaires Python

## 🛠️ Prérequis & Compilation

### Prérequis

- **CMOC** (version 0.1.93 ou supérieure recommandée)
- **lwtools** (pour l'archiveur `lwar`)

### Compiler la bibliothèque

```bash
make
```

### Exporter le SDK

Pour générer un dossier `sdk_mo5` prêt à être distribué (contenant uniquement les `.h` et le `.a`) :

```bash
make export_sdk
```

> C'est cette commande qui est appelée automatiquement par `make install` dans le template de projet.

---

## 📦 Contenu de la bibliothèque

### Entrées/sorties bas niveau — `mo5_defs.h`

Accès direct au moniteur système via interruptions SWI.

| Fonction | Description |
|---|---|
| `mo5_getchar()` | Lecture bloquante d'un caractère clavier |
| `mo5_putchar(c)` | Écriture d'un caractère à l'écran |
| `mo5_newline()` | Passage à la ligne suivante |
| `mo5_wait_key(key)` | Attend qu'une touche précise soit pressée |
| `mo5_wait_for_key()` | Attend une touche et la retourne (majuscule) |

---

### Entrées/sorties haut niveau — `mo5_stdio.h`

Fonctions de chaînes et d'affichage texte.

| Fonction | Description |
|---|---|
| `fgets(buf, max)` | Lecture d'une chaîne avec écho et support Backspace |
| `fputs(s)` | Écriture d'une chaîne |
| `puts(s)` | Écriture d'une chaîne suivie d'un saut de ligne |
| `clrscr()` | Efface l'écran et repositionne le curseur |
| `getchar` | Macro vers `mo5_getchar()` |

---

### Classification de caractères — `mo5_ctype.h`

Validation de caractères conforme aux standards C.

| Fonction | Description |
|---|---|
| `islower(c)` | Lettre minuscule |
| `isupper(c)` | Lettre majuscule |
| `isprint(c)` | Caractère imprimable (32-126) |
| `ispunct(c)` | Caractère de ponctuation |

---

### Utilitaires — `mo5_utils.h`

| Fonction | Description |
|---|---|
| `mo5_clamp(val, min, max)` | Limite une valeur à un intervalle (`unsigned char`) |

---

### Vidéo & hardware — `mo5_video.h`

Abstraction du hardware vidéo : registres VRAM, palette 16 couleurs, dimensions écran, synchronisation VBL.

| Fonction | Description |
|---|---|
| `mo5_video_init(color)` | Initialise le mode graphique (pré-calcule `row_offsets`, remplit la banque couleur) |
| `mo5_wait_vbl()` | Attend le prochain blanc vertical (50 Hz PAL) |
| `mo5_clear_screen(color)` | Efface l'écran entier |
| `mo5_fill_rect(tx, ty, w, h, color)` | Remplit un rectangle |

Constantes de couleur : `C_BLACK`, `C_RED`, `C_GREEN`, `C_YELLOW`, `C_BLUE`, `C_MAGENTA`, `C_CYAN`, `C_WHITE`, `C_GRAY`, `C_LIGHT_RED`, `C_LIGHT_GREEN`, `C_LIGHT_YELLOW`, `C_LIGHT_BLUE`, `C_PURPLE`, `C_LIGHT_CYAN`, `C_ORANGE`.

Macro couleur : `COLOR(bg, fg)`, encode fond et forme en un octet `FFFFBBBB`.

---

### Sprites opaques — `mo5_sprite.h`

Rendu direct sur fond noir (les deux banques VRAM sont écrasées). À utiliser quand le fond est uniforme.

| Fonction | Description |
|---|---|
| `mo5_draw_sprite(...)` | Dessine un sprite (coordonnées brutes) |
| `mo5_clear_sprite(...)` | Efface un sprite |
| `mo5_move_sprite(...)` | Déplace un sprite (optimisé : ne redessine que la zone modifiée) |
| `mo5_actor_draw(actor)` | Dessine un acteur |
| `mo5_actor_clear(actor)` | Efface un acteur |
| `mo5_actor_move(actor, x, y)` | Déplace un acteur |

---

### Sprites transparents — `mo5_sprite_bg.h` ⭐ Recommandé

Rendu transparent sur fond coloré. La couleur de fond du décor est préservée.

Convention : les bits de fond des données couleur du sprite doivent être `0x0`.

| Fonction | Description |
|---|---|
| `mo5_draw_sprite_bg(...)` | Dessine un sprite transparent |
| `mo5_clear_sprite_bg(...)` | Efface un sprite (banque forme uniquement) |
| `mo5_move_sprite_bg(...)` | Déplace un sprite transparent |
| `mo5_actor_draw_bg(actor)` | Dessine un acteur |
| `mo5_actor_clear_bg(actor)` | Efface un acteur |
| `mo5_actor_move_bg(actor, x, y)` | Déplace un acteur en préservant le fond |

---

### Sprites forme seule — `mo5_sprite_form.h`

Rendu le plus rapide : seule la banque forme est écrite. La banque couleur est initialisée une fois et jamais retouchée. À utiliser quand tous les pixels partagent la même couleur de premier plan.

| Fonction | Description |
|---|---|
| `mo5_draw_sprite_form(...)` | Dessine (forme seule) |
| `mo5_clear_sprite_form(...)` | Efface |
| `mo5_move_sprite_form(...)` | Déplace |
| `mo5_actor_draw_form(actor)` | Dessine un acteur |
| `mo5_actor_clear_form(actor)` | Efface un acteur |
| `mo5_actor_move_form(actor, x, y)` | Déplace un acteur |

---

### Dirty Rectangle — `mo5_actor_dr.h`

Moteur de sprites avancé : sauvegarde la zone VRAM avant de dessiner, et la restaure en début de frame. Permet des fonds arbitrairement colorés, une superposition correcte des sprites, et une transparence parfaite.

Séquence obligatoire par frame :
1. `mo5_actor_dr_restore()` sur tous les sprites (ordre inverse)
2. Logique de jeu + `mo5_actor_dr_move()`
3. `mo5_actor_dr_save_draw()` sur tous les sprites (ordre normal)

| Fonction | Description |
|---|---|
| `mo5_actor_dr_init(actor, sprite, x, y)` | Initialise l'acteur et effectue le premier dessin |
| `mo5_actor_dr_restore(actor)` | Restaure la VRAM sauvegardée |
| `mo5_actor_dr_save_draw(actor)` | Sauvegarde la VRAM puis dessine |
| `mo5_actor_dr_move(actor, x, y)` | Met à jour la position |

---

### Affichage texte en mode graphique — `mo5_font6.h` / `mo5_font8.h`

Polices arcade pour afficher du texte sans écraser le fond du décor.

| Module | Taille | Lignes affichables |
|---|---|---|
| `mo5_font6.h` | 8×6 pixels | 33 lignes |
| `mo5_font8.h` | 8×8 pixels | 25 lignes |

| Fonction | Description |
|---|---|
| `mo5_font6_puts(tx, ty, s, fg)` | Affiche une chaîne en police 6px |
| `mo5_font6_clear(tx, ty, len)` | Efface une zone texte |
| `mo5_font8_puts(tx, ty, s, fg)` | Affiche une chaîne en police 8px |
| `mo5_font8_clear(tx, ty, len)` | Efface une zone texte |

Coordonnées : `tx` en octets (0-39), `ty` en pixels (0-199).

---

### Types partagés — `mo5_sprite_types.h`

Inclus automatiquement par les modules sprite. Ne pas inclure directement.

Définit `MO5_Position`, `MO5_Sprite`, `MO5_Actor` et `mo5_actor_clamp()`.

---

## 🐍 Scripts Python

### `png2mo5.py`

Convertit une image PNG en structures C pour le MO5.
Vérifie et encode la contrainte hardware (2 couleurs par bloc de 8 pixels).

```bash
make convert IMG=./assets/player.png
# génère include/assets/player.h
```

### `makefd.py`

Génère une image disquette `.fd` autobootable pour Thomson MO5 à partir
d'un binaire `.BIN`.

Le bootloader Thomson (`BOOTMO.BIN`) est **embarqué directement dans le
script**, plus besoin de cloner ou de compiler le projet externe
[BootFloppyDisk](https://github.com/OlivierP-To8/BootFloppyDisk).
La toolchain est ainsi autonome et ne dépend d'aucun repo tiers.

```bash
python3 scripts/makefd.py output.fd program.BIN
```

> C'est ce script qui est appelé automatiquement par `make` dans le template de projet.

### `fd2sd.py`

Convertit une image disquette `.fd` au format `.sd` compatible SDDrive.

```bash
python3 scripts/fd2sd.py input.fd output.sd
```

---

## 🤖 Intégration avec un assistant IA (MCP)

Les fichiers `docs/` contiennent la documentation Markdown de tous les modules.
Ils sont indexés dans le serveur RAG MO5, votre agent IA peut y faire des recherches en langage naturel.

👉 [retrocomputing-ai.cloud](https://retrocomputing-ai.cloud)  
👉 [npmjs.com/@thlg057/mo5-rag-mcp](https://www.npmjs.com/package/@thlg057/mo5-rag-mcp)

---

## 🔗 L'écosystème complet

- 📦 **Template de projet** : https://github.com/thlg057/mo5_template
- 🎮 **Tutoriel Space Invaders** : https://github.com/thlg057/mo5-space-invaders-tutorial
- 🤖 **Serveur MCP** : https://www.npmjs.com/package/@thlg057/mo5-rag-mcp
- 🌐 **Base de connaissances** : https://retrocomputing-ai.cloud
- 📖 **Blog** : https://thlg057.github.io/mo5-blog/

---

## 📄 Licence

MIT, Copyright (c) 2026 Thierry Le Got