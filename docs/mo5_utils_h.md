# `mo5_utils.h` — Fonctions utilitaires pour le Thomson MO5

> Helpers génériques utilisables dans tout projet MO5 : calcul numérique, bornes, etc.

---

## Dépendance

```c
#include "mo5_utils.h"
```

Aucune dépendance interne au SDK.

---

## Fonctions

### `mo5_clamp`

```c
int mo5_clamp(int value, int min, int max);
```

Contraint `value` dans l'intervalle `[min, max]`.

| Condition | Valeur retournée |
|---|---|
| `value < min` | `min` |
| `value > max` | `max` |
| sinon | `value` |

**Paramètres**

| Paramètre | Description |
|---|---|
| `value` | Valeur à contraindre |
| `min` | Borne inférieure (incluse) |
| `max` | Borne supérieure (incluse) |

**Retour** : valeur clampée dans `[min, max]`.

> ⚠️ Comportement indéfini si `min > max`.

---

## Exemple d'utilisation

```c
#include "mo5_utils.h"

// Limiter la vitesse d'un sprite entre -3 et 3
speed = mo5_clamp(speed, -3, 3);

// Limiter une coordonnée aux bords de l'écran (axe X, en octets)
int tx = mo5_clamp(new_tx, 0, SCREEN_WIDTH_BYTES - sprite_w);
```

---

## Relation avec `mo5_actor_clamp`

`mo5_clamp` est une primitive bas niveau qui opère sur un entier brut.  
`mo5_actor_clamp` (voir `mo5_sprite.h`) s'en distingue : elle clamp la position d'un `MO5_Actor` entier en tenant compte de la taille du sprite.

Utiliser `mo5_clamp` lorsque le clamp porte sur une valeur calculée manuellement (vitesse, indice, coordonnée isolée).

