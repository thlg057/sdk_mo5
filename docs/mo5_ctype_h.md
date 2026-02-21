# `mo5_ctype.h` — Classification de caractères pour le Thomson MO5

> Fonctions de test de caractères, complémentaires à celles de `cmoc.h`. À utiliser à la place ou en complément des fonctions `is*` de CMOC.

---

## Dépendance

```c
#include "mo5_defs.h"  // inclus automatiquement
```

---

## Fonctions

### `int islower(char c)`

Retourne `TRUE` si `c` est une lettre minuscule (`'a'`–`'z'`), `FALSE` sinon.

### `int isupper(char c)`

Retourne `TRUE` si `c` est une lettre majuscule (`'A'`–`'Z'`), `FALSE` sinon.

### `int isprint(char c)`

Retourne `TRUE` si `c` est un caractère imprimable (ASCII 32–126 inclus), `FALSE` sinon. Les caractères de contrôle (0–31) et DEL (127+) retournent `FALSE`.

### `int ispunct(char c)`

Retourne `TRUE` si `c` est un caractère de ponctuation, `FALSE` sinon.

Plages ASCII couvertes :

| Plage | Exemples |
|---|---|
| 33–47 | `! " # $ % & ' ( ) * + , - . /` |
| 58–64 | `: ; < = > ? @` |
| 91–96 | `[ \ ] ^ _ \`` |
| 123–126 | `{ \| } ~` |

---

## Fonctions disponibles dans `cmoc.h` (ne pas redéfinir)

Ces fonctions couvrent des cas complémentaires et sont déjà fournies par CMOC :

| Fonction | Description |
|---|---|
| `isspace(int c)` | Espace, tabulation, saut de ligne |
| `isalpha(int c)` | Lettre (minuscule ou majuscule) |
| `isalnum(int c)` | Lettre ou chiffre |
| `isdigit(int c)` | Chiffre décimal `'0'`–`'9'` |
| `tolower(int c)` | Convertit en minuscule |
| `toupper(int c)` | Convertit en majuscule |

---

## Exemple d'utilisation

```c
#include "mo5_ctype.h"
#include <cmoc.h>

void analyseChar(char c) {
    if (isupper(c))       fputs("Majuscule");
    else if (islower(c))  fputs("Minuscule");
    else if (isdigit(c))  fputs("Chiffre");
    else if (ispunct(c))  fputs("Ponctuation");
    else if (isprint(c))  fputs("Imprimable");
    else                  fputs("Controle");
}
```
