# `mo5_joystick` — Lecture des manettes de jeu

> Polling des deux manettes MO5 via le PIA extension. Détection d'edge (appui/relâché) et état maintenu pour les directions et le bouton fire.

---

## Rôle du module

`mo5_joystick` encapsule la lecture hardware des manettes via le PIA extension (`$A7CC–$A7CF`). Il fournit un état normalisé (1 = pressé) avec edge detection par comparaison entre le frame courant et le frame précédent.

```
┌─────────────────────────────────────────┐
│            Code de jeu                  │
├─────────────────────────────────────────┤
│  mo5_joystick  (polling + edge detect)  │  ← ce module
├─────────────────────────────────────────┤
│  PIA extension $A7CC–$A7CF              │
│  PORTA : directions   PORTB : boutons   │
└─────────────────────────────────────────┘
```

---

## Disponibilité hardware

| Matériel | Disponibilité |
|----------|--------------|
| MO5 de base | Optionnel (extension jeux/musique) |
| MO5E, MO6, MO5NR | Intégré de série |

---

## Inclusion

```c
#include "mo5_joystick.h"
```

Aucune dépendance interne au SDK.

---

## Registres matériels

| Macro | Adresse | Rôle |
|-------|---------|------|
| `JOY_PORTA` | `$A7CC` | Directions des deux manettes (actif bas) |
| `JOY_PORTB` | `$A7CD` | Boutons fire bits 6-7 (actif bas) + DAC bits 0-5 |
| `JOY_CRA` | `$A7CE` | Registre de contrôle PORTA |
| `JOY_CRB` | `$A7CF` | Registre de contrôle PORTB |

### Câblage PORTA $A7CC — directions

```
bit 0 : manette 0 — HAUT    (actif bas)
bit 1 : manette 0 — BAS     (actif bas)
bit 2 : manette 0 — GAUCHE  (actif bas)
bit 3 : manette 0 — DROITE  (actif bas)
bit 4 : manette 1 — HAUT    (actif bas)
bit 5 : manette 1 — BAS     (actif bas)
bit 6 : manette 1 — GAUCHE  (actif bas)
bit 7 : manette 1 — DROITE  (actif bas)
```

### Câblage PORTB $A7CD — boutons fire

```
bit 6 : bouton fire manette 0 (actif bas)
bit 7 : bouton fire manette 1 (actif bas)
```

> ⚠️ Les signaux hardware sont **actifs bas** (0 = pressé).
> Le module inverse automatiquement la logique — dans le code, **1 = pressé**.
>
> Source confirmée : Clefs Pour MO5 (Blanchard, 1985) p.99 et p.107 :
> "B6 : Poussoir manette 0 (relié au CA1 du PIA). B7 : Poussoir manette 1."

---

## Structure `MO5_Joystick`

```c
typedef struct {
    unsigned char dirs;       /* directions courantes (bits normalisés, 1=pressé) */
    unsigned char fire;       /* bouton fire courant (0 ou 1) */
    unsigned char prev_dirs;  /* directions du frame précédent */
    unsigned char prev_fire;  /* bouton fire du frame précédent */
} MO5_Joystick;
```

Deux instances globales, accessibles directement si besoin :

```c
extern MO5_Joystick mo5_joy1;   /* manette 0 */
extern MO5_Joystick mo5_joy2;   /* manette 1 */
```

> Utiliser de préférence les macros plutôt que d'accéder aux champs directement.

---

## Fonctions

### `mo5_joystick_init`

```c
void mo5_joystick_init(void);
```

Initialise le PIA extension pour la lecture des manettes :
- Configure PORTA en entrées (DDRA = 0x00)
- Force les bits 6-7 de PORTB en entrées (boutons fire), en préservant les bits 0-5 (DAC)

À appeler **une seule fois au démarrage**, avant la boucle principale.

```c
mo5_video_init(COLOR(C_BLACK, C_BLACK));
mo5_mute_beep();
mo5_joystick_init();   /* ← avant la boucle */
```

> ⚠️ Si `mo5_dac_init()` est aussi appelé, l'appeler **après** `mo5_joystick_init()`
> pour que la séquence DDR du DAC (bits 0-5 en sorties) ne perturbe pas les bits 6-7.

---

### `mo5_joystick_update`

```c
void mo5_joystick_update(void);
```

Lit l'état des deux manettes et met à jour `mo5_joy1` et `mo5_joy2`. À appeler **une seule fois par frame**, en début de boucle après `mo5_wait_vbl()`.

Après cet appel :
- `dirs` et `fire` contiennent l'état **courant**
- `prev_dirs` et `prev_fire` contiennent l'état du **frame précédent**
- Toutes les macros `_PRESSED` / `_RELEASED` / `_HELD` sont utilisables

```c
while (1) {
    mo5_wait_vbl();
    mo5_joystick_update();   /* ← une seule fois, en premier */
    /* ensuite lire via les macros */
}
```

> ⚠️ Ne pas appeler `mo5_joystick_update()` plusieurs fois par frame —
> cela écraserait `prev_dirs` et casserait l'edge detection.

---

## Macros de test

Trois familles de macros disponibles pour chaque direction et pour le bouton fire.

### `_HELD` — état maintenu (enfoncé ce frame)

```c
/* Manette 0 */
MO5_JOY1_UP_HELD()       MO5_JOY1_DOWN_HELD()
MO5_JOY1_LEFT_HELD()     MO5_JOY1_RIGHT_HELD()
MO5_JOY1_FIRE_HELD()

/* Manette 1 */
MO5_JOY2_UP_HELD()       MO5_JOY2_DOWN_HELD()
MO5_JOY2_LEFT_HELD()     MO5_JOY2_RIGHT_HELD()
MO5_JOY2_FIRE_HELD()
```

Retourne vrai tant que la direction/le bouton est enfoncé. Idéal pour le déplacement continu.

```c
if (MO5_JOY1_RIGHT_HELD()) player_move_right();
if (MO5_JOY1_LEFT_HELD())  player_move_left();
```

---

### `_PRESSED` — edge montant (vient d'être pressé ce frame)

```c
/* Manette 0 */
MO5_JOY1_UP_PRESSED()    MO5_JOY1_DOWN_PRESSED()
MO5_JOY1_LEFT_PRESSED()  MO5_JOY1_RIGHT_PRESSED()
MO5_JOY1_FIRE_PRESSED()

/* Manette 1 */
MO5_JOY2_UP_PRESSED()    MO5_JOY2_DOWN_PRESSED()
MO5_JOY2_LEFT_PRESSED()  MO5_JOY2_RIGHT_PRESSED()
MO5_JOY2_FIRE_PRESSED()
```

Retourne vrai **une seule fois**, le frame où la touche passe de relâchée à pressée. Idéal pour le tir, la validation de menu, le saut.

```c
if (MO5_JOY1_FIRE_PRESSED()) game_shoot();
if (MO5_JOY2_FIRE_PRESSED()) game_shoot_player2();
```

---

### `_RELEASED` — edge descendant (vient d'être relâché ce frame)

```c
/* Manette 0 */
MO5_JOY1_UP_RELEASED()    MO5_JOY1_DOWN_RELEASED()
MO5_JOY1_LEFT_RELEASED()  MO5_JOY1_RIGHT_RELEASED()
MO5_JOY1_FIRE_RELEASED()

/* Manette 1 */
MO5_JOY2_UP_RELEASED()    MO5_JOY2_DOWN_RELEASED()
MO5_JOY2_LEFT_RELEASED()  MO5_JOY2_RIGHT_RELEASED()
MO5_JOY2_FIRE_RELEASED()
```

Retourne vrai **une seule fois**, le frame où la touche passe de pressée à relâchée. Utile pour déclencher une action à la fin d'un appui (charge de tir, fin d'animation).

```c
if (MO5_JOY1_FIRE_RELEASED()) game_release_charge();
```

---

## Patterns d'utilisation

### Démarrage typique

```c
mo5_video_init(COLOR(C_BLACK, C_BLACK));
mo5_mute_beep();
mo5_joystick_init();

while (1) {
    mo5_wait_vbl();
    mo5_joystick_update();

    /* déplacement continu */
    if (MO5_JOY1_RIGHT_HELD()) mo5_actor_move(&player, player.pos.x + 1, player.pos.y);
    if (MO5_JOY1_LEFT_HELD())  mo5_actor_move(&player, player.pos.x - 1, player.pos.y);
    if (MO5_JOY1_UP_HELD())    mo5_actor_move(&player, player.pos.x, player.pos.y - 1);
    if (MO5_JOY1_DOWN_HELD())  mo5_actor_move(&player, player.pos.x, player.pos.y + 1);

    /* tir sur appui */
    if (MO5_JOY1_FIRE_PRESSED()) game_spawn_bullet();
}
```

### Sélection clavier / joystick au démarrage

```c
static unsigned char use_joystick;

void game_select_input(void) {
    unsigned char key;
    mo5_font6_puts(2, 90, "JOYSTICK? FIRE=OUI  CLAVIER=AUTRE", C_WHITE);
    mo5_joystick_init();

    while (1) {
        mo5_wait_vbl();
        mo5_joystick_update();

        if (MO5_JOY1_FIRE_PRESSED()) { use_joystick = 1; return; }
        key = mo5_getchar();
        if (key != 0)                { use_joystick = 0; return; }
    }
}
```

### Jeu à deux joueurs

```c
while (1) {
    mo5_wait_vbl();
    mo5_joystick_update();

    /* Joueur 1 — manette 0 */
    if (MO5_JOY1_RIGHT_HELD())   move_player(0, +1, 0);
    if (MO5_JOY1_FIRE_PRESSED()) shoot_player(0);

    /* Joueur 2 — manette 1 */
    if (MO5_JOY2_RIGHT_HELD())   move_player(1, +1, 0);
    if (MO5_JOY2_FIRE_PRESSED()) shoot_player(1);
}
```

---

## Relation avec les autres modules

| Module | Relation |
|--------|----------|
| `mo5_video` | `mo5_wait_vbl()` doit précéder `mo5_joystick_update()` |
| `mo5_audio` | Si DAC utilisé, appeler `mo5_dac_init()` **après** `mo5_joystick_init()` |
| `mo5_sprite` | Indépendant — les positions acteurs sont mises à jour après le update |

---

*Voir `mo5_hardware_reference.md` section 6 pour les détails complets des registres PIA extension.*
*Voir `mo5_audio_h.md` pour l'initialisation DAC et la cohabitation avec le joystick.*
