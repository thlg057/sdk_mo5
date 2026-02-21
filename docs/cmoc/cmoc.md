# Index CMOC — Headers pour le développement Thomson MO5

Référence rapide de tous les headers CMOC et leur niveau de compatibilité avec le MO5.

---

## Tableau récapitulatif

| Header | Statut | Usage sur MO5 |
|---|---|---|
| [`cmoc.h`](cmoc_h.md) | ✅ Largement utilisable | Fonctions standard : chaînes, mémoire, maths, tri, conversions. Rediriger la sortie via `setConsoleOutHook`. |
| [`stdarg.h`](stdarg_h.md) | ✅ Totalement portable | Arguments variables (`va_list`, `va_start`, `va_arg`, `va_end`). Inclus par `cmoc.h`. |
| [`setjmp.h`](setjmp_h.md) | ✅ Totalement portable | Sauts non-locaux (`setjmp` / `longjmp`). Utile pour la gestion d'erreurs. |
| [`assert.h`](assert_h.md) | ✅ Utilisable | Assertions de debug. Installer un handler personnalisé via `_SetFailedAssertHandler`. |
| [`assert-impl.h`](assert_impl_h.md) | ⚠️ Interne | Ne pas inclure directement. Utilisé automatiquement par `assert.h`. |
| [`cmoc-stdlib-private.h`](cmoc_stdlib_private_h.md) | ⚠️ Interne | Ne pas inclure directement. Symboles internes du runtime CMOC. |
| [`vectrex.h`](vectrex_h.md) | ⚠️ Types réutilisables | Ne pas inclure. S'en inspirer pour définir `byte`, `word`, `uint32_t`, etc. dans ton propre header MO5. |
| [`disk.h`](disk_h.md) | ❌ Non compatible | Disque CoCo Disk Basic uniquement. Registres `$FF40`+. Réécrire pour Thomson. |
| [`dskcon-standalone.h`](dskcon_standalone_h.md) | ❌ Non compatible | Contrôleur WD1793 CoCo uniquement. Registres `$FF40`/`$FF48`. Réécrire pour Thomson. |
| `coco.h` | ❌ Non compatible | Hardware CoCo/Dragon spécifique. Non documenté ici. |

---

## Checklist de démarrage pour un projet MO5

### 1. Toujours inclure

```c
#include <cmoc.h>      // Bibliothèque standard générique
#include <stdarg.h>    // Si fonctions variadiques (inclus par cmoc.h)
```

### 2. Rediriger la sortie console

```c
// En début de main() :
setConsoleOutHook(maRoutineAffichageMO5);
// maRoutineAffichageMO5 doit recevoir le caractère dans le registre A
// et préserver B, X, Y, U
```

### 3. Définir tes propres types de base (inspiré de vectrex.h)

```c
// mo5.h
typedef unsigned char  byte;
typedef signed char    sbyte;
typedef unsigned int   word;
typedef signed int     sword;
typedef unsigned long  uint32_t;
typedef signed long    int32_t;
enum { FALSE = 0, TRUE = 1 };
```

### 4. Assertions en debug

```c
#include <assert.h>
// Installer un handler personnalisé :
_SetFailedAssertHandler(monHandlerMO5);
```

### 5. Désactiver les assertions en release

```c
#define NDEBUG
#include <assert.h>
```

---

## Fonctions CMOC les plus utiles pour le MO5

### Mémoire (indispensables pour la VRAM)
- `memset()` — remplissage rapide (ex: effacer l'écran)
- `memset16()` — remplissage par mots 16 bits
- `memcpy()` / `memmove()` — copie de blocs

### Chaînes
- `sprintf()` — formatage dans un buffer (ne dépend pas du terminal)
- `strlen()`, `strcpy()`, `strcat()`, `strcmp()`

### Maths entières
- `divmod16()` / `divmod8()` — division + reste en une opération (efficace sur MC6809)
- `sqrt16()` / `sqrt32()` — racines carrées entières
- `abs()` / `labs()`

### Conversions
- `itoa10()` / `utoa10()` — entier → chaîne (pour affichage score, etc.)
- `atoi()` / `atoui()` — chaîne → entier (pour parsing)

### Tri et recherche
- `qsort()` / `bsearch()` — tri et recherche génériques

### Aléatoire
- `srand()` / `rand()` — générateur pseudo-aléatoire
