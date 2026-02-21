# `assert.h` — Référence pour le développement Thomson MO5

> **Statut global** : ✅ Utilisable. Le mécanisme d'assertion est portable, mais la sortie du message d'erreur par défaut dépend de la console. Il est recommandé d'installer un handler personnalisé sur MO5.

---

## Dépendances

- Si `NDEBUG` est défini : toutes les macros sont des no-ops, aucune dépendance.
- Si `NDEBUG` n'est pas défini : inclut `assert-impl.h` automatiquement.

---

## Macro principale

### `assert(cond)`

```c
assert(condition);
```

Si `condition` est fausse (évalue à `0`), déclenche une erreur d'assertion :
- Affiche le fichier, le numéro de ligne et la condition échouée.
- Puis soit appelle `exit(1)`, soit boucle à l'infini, selon la configuration.

En mode `NDEBUG` : **la macro ne fait rien** (zéro coût à l'exécution).

---

## Fonctions de configuration (disponibles si `NDEBUG` non défini)

### `void _FreezeOnFailedAssert(int freeze)`

Détermine le comportement après un assert échoué (sans handler personnalisé) :

| `freeze` | Comportement |
|---|---|
| `0` (défaut) | Appelle `exit(1)` |
| non-nul | Boucle infinie (freeze) |

> 💡 **MO5** : Le freeze est utile en debug pour inspecter l'écran avant que le programme ne quitte.

### `void _SetFailedAssertHandler(_FailedAssertHandler newHandler)`

Installe un handler personnalisé appelé lors d'un assert échoué.

**Signature du handler** :
```c
void monHandler(const char *file, int lineno, const char *condition);
```

Le handler peut choisir de retourner (l'exécution continue) ou de ne pas retourner.

---

## Type

```c
typedef void (*_FailedAssertHandler)(const char *file, int lineno, const char *condition);
```

---

## Exemple d'utilisation sur MO5

```c
#include <assert.h>
#include <cmoc.h>

// Handler personnalisé pour MO5 : affiche sur l'écran Thomson
void mo5AssertHandler(const char *file, int lineno, const char *condition) {
    // Utiliser ta propre fonction d'affichage MO5
    printMO5("ASSERT FAIL\n");
    // Boucle infinie pour pouvoir lire le message
    for (;;) {}
}

int main(void) {
    _SetFailedAssertHandler(mo5AssertHandler);

    int x = 5;
    assert(x > 0);   // OK
    assert(x > 10);  // Déclenche mo5AssertHandler
    return 0;
}
```

---

## Mode release : désactiver les assertions

```c
#define NDEBUG
#include <assert.h>
// assert() ne génère plus aucun code
```

---

## Notes MO5

- La macro `assert` elle-même est **totalement portable**.
- Le comportement par défaut (message + `exit`) suppose une console fonctionnelle. Sur MO5, **installe un handler personnalisé** via `_SetFailedAssertHandler` pour afficher via ta routine d'écran Thomson.
- En production, compile avec `-DNDEBUG` pour éliminer tout le code d'assertion.
