# `setjmp.h` — Référence pour le développement Thomson MO5

> **Statut global** : ✅ Totalement utilisable. Ce header fournit un mécanisme de saut non-local (équivalent d'un `goto` entre fonctions), indépendant du matériel.

---

## Type défini

```c
typedef unsigned char jmp_buf[6];
```

Un tampon de 6 octets pour sauvegarder le contexte d'exécution (registres PC, U/SP, etc. du MC6809).

---

## Fonctions

### `int setjmp(jmp_buf env)`

Sauvegarde le contexte d'exécution courant dans `env`.

- **Retourne `0`** lors de l'appel initial.
- **Retourne une valeur non nulle** (celle passée à `longjmp`) lorsque l'exécution revient ici via `longjmp`.

### `void longjmp(jmp_buf env, int value)`

Restaure le contexte sauvegardé dans `env` et reprend l'exécution comme si `setjmp` venait de retourner `value`.

> ⚠️ `value` **ne doit pas être `0`**. Si `0` est passé, le comportement est indéfini.

---

## Exemple d'utilisation

```c
#include <setjmp.h>
#include <cmoc.h>

jmp_buf errorContext;

void riskyOperation(void) {
    // En cas d'erreur :
    longjmp(errorContext, 1);
}

int main(void) {
    if (setjmp(errorContext) != 0) {
        printf("Erreur interceptee !\n");
        return 1;
    }
    riskyOperation();
    return 0;
}
```

---

## Notes MO5

- Implémentation de bas niveau adaptée au MC6809. **Aucune dépendance matérielle** envers le CoCo.
- Utile pour la gestion d'erreurs sans exceptions en C.
- Les 6 octets du `jmp_buf` correspondent aux registres sauvegardés par le MC6809 (typiquement PC + U + CC ou similaire selon l'implémentation CMOC).
- Ne pas utiliser `longjmp` pour remonter **au-delà d'une fonction qui a déjà retourné** : comportement indéfini.
