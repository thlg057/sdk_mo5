# `mo5_defs.h` — Définitions de base pour le Thomson MO5

> Header fondamental à inclure en premier dans tout projet MO5. Définit le type booléen, les constantes système et les primitives d'I/O de bas niveau via SWI.

---

## Type booléen

```c
typedef enum { FALSE = 0, TRUE = 1 } Boolean;
```

---

## Constantes

### Tailles

| Constante | Valeur | Usage |
|---|---|---|
| `MO5_MAX_NAME_LENGTH` | `30` | Longueur maximale d'un nom |
| `MO5_BUFFER_SIZE` | `32` | Taille standard d'un buffer |

### Codes caractères

| Constante | Valeur ASCII | Description |
|---|---|---|
| `MO5_CLEAR_SCREEN` | `12` | Efface l'écran |
| `MO5_BACKSPACE_CHAR` | `8` | Retour arrière |
| `MO5_ENTER_CHAR` | `13` | Entrée / retour chariot |
| `MO5_SPACE_CHAR` | `32` | Espace |
| `MO5_LINE_FEED` | `10` | Saut de ligne |

---

## Fonctions

### `char mo5_getchar(void)`

Lit le clavier via `SWI $0A`. **Non bloquant** : retourne `0` si aucune touche n'est pressée.

```asm
swi
fcb $0A
```

> ⚠️ Pour une lecture bloquante, utiliser une boucle d'attente :
> ```c
> char ch;
> do { ch = mo5_getchar(); } while (ch == 0);
> ```
> Ou `mo5_wait_key()` si disponible dans ton projet.

### `void mo5_putchar(char c)`

Envoie le caractère `c` à l'affichage via `SWI $02`. Le caractère est chargé dans le registre B avant l'appel système.

```asm
ldb c
swi
fcb $02
```

### `void mo5_newline(void)`

Envoie `MO5_ENTER_CHAR` (13) suivi de `MO5_LINE_FEED` (10) via `mo5_putchar`. Produit un saut de ligne complet CR+LF.

### `void mo5_wait_key(char key)`

Attend qu'une touche précise soit pressée. Ignore tous les autres caractères tant que la touche attendue n'est pas saisie.

```c
char ch;
do {
    ch = getchar();
} while (ch != key);
```

> ℹ️ Utiliser cette fonction pour synchroniser l'exécution sur une touche connue (ex : confirmation, navigation menu).

### `char wait_for_key(void)`

Attend n'importe quelle touche via `mo5_getchar()` et retourne le caractère.

```c
char ch;
do {
    ch = mo5_getchar();
} while (ch == 0);
return ch;
```
---

## Notes

- Ce header est la dépendance de base de `mo5_stdio.h` et `mo5_ctype.h`.
- Les deux appels SWI (`$0A` lecture, `$02` écriture) sont les seules interactions avec le hardware dans ce fichier.
- `mo5_putchar` est la primitive sur laquelle repose toute la couche de sortie de `mo5_stdio.h`. `fputs`, `puts`, `clrscr` l'utilisent toutes indirectement.
