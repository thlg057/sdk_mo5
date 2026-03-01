# `mo5_ctype.h` — Classification de caractères pour le Thomson MO5

> Fonctions de test de caractères, complémentaires à celles de `cmoc.h`. À utiliser à la place ou en complément des fonctions `is*` de CMOC.

---

## Dépendance

```c
#include "mo5_ctype.h"   // aucune dépendance externe
```

---

## Fonctions

### `unsigned char islower(char c)`

Retourne `1` si `c` est une lettre minuscule (`'a'`–`'z'`), `0` sinon.

### `unsigned char isupper(char c)`

Retourne `1` si `c` est une lettre majuscule (`'A'`–`'Z'`), `0` sinon.

### `unsigned char isprint(char c)`

Retourne `1` si `c` est un caractère imprimable (ASCII 32–126 inclus), `0` sinon. Les caractères de contrôle (0–31) et DEL (127+) retournent `0`.

### `unsigned char ispunct(char c)`

Retourne `1` si `c` est un caractère de ponctuation, `0` sinon.

Plages ASCII couvertes :

| Plage | Exemples |
|---|---|
| 33–47 | `! " # $ % & ' ( ) * + , - . /` |
| 58–64 | `: ; < = > ? @` |
| 91–96 | `[ \ ] ^ _ \`` |
| 123–126 | `{ \| } ~` |

---

## Note sur les types de retour

Les fonctions retournent `unsigned char` (valeur `0` ou `1`) et non `int` comme les équivalents CMOC. Ce choix évite les opérations 16 bits inutiles sur le 6809. Toute valeur non nulle est considérée vraie en C, le code existant reste compatible :

```c
if (isprint(ch)) { ... }   // fonctionne identiquement
```

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
