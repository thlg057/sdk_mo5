# `stdarg.h` — Référence pour le développement Thomson MO5

> **Statut global** : ✅ Totalement utilisable. Ce header implémente le support des arguments variables, indépendant de toute plateforme.

---

## Type défini

```c
typedef char *va_list;
```

---

## Macros

| Macro | Signature | Description |
|---|---|---|
| `va_start` | `va_start(ap, lastFixed)` | Initialise `ap` pour pointer sur le premier argument variable, après `lastFixed`. À appeler en début de fonction variadique. |
| `va_arg` | `va_arg(ap, type)` | Lit et retourne l'argument suivant, interprété comme `type`. Avance `ap`. |
| `va_end` | `va_end(ap)` | Finalise l'utilisation de `ap`. Ne fait rien dans cette implémentation, mais doit être appelé par convention. |

---

## Fonction interne (ne pas appeler directement)

```c
char *__va_arg(va_list *app, unsigned int sizeInBytes);
```

Utilisée en interne par la macro `va_arg`. Ne pas appeler directement.

---

## Exemple d'utilisation

```c
#include <stdarg.h>
#include <cmoc.h>

int sum(int count, ...) {
    va_list ap;
    va_start(ap, count);
    int total = 0;
    int i;
    for (i = 0; i < count; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);
    return total;
}
```

---

## Notes MO5

- Sur le MC6809 (MO5), les arguments sont passés sur la pile dans l'ordre d'appel. Cette implémentation de `va_list` repose directement sur l'arithmétique de pointeurs sur la pile, ce qui est **correct et portable** sur cette architecture.
- Pas de dépendance matérielle. Aucune adaptation nécessaire.
- Inclus automatiquement par `cmoc.h`.
