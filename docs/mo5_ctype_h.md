# `mo5_ctype` — Classification de caractères pour le MO5

> Fonctions de test de caractères optimisées 8-bit, complémentaires à `cmoc.h`

---

## Rôle du module

`mo5_ctype` fournit des fonctions de classification de caractères adaptées au 6809. Les équivalents de la libc standard (`isprint`, `isupper`...) manipulent des `int` — des valeurs 16-bit inutilement coûteuses sur un processeur 8-bit. Ce module retourne des `unsigned char` à la place, évitant toute opération 16-bit superflue.

À utiliser **à la place ou en complément** des fonctions `is*` de CMOC.

```
┌─────────────────────┐
│    Code de jeu      │  lecture clavier, saisie texte
├─────────────────────┤
│    mo5_ctype        │  ← ce module
├─────────────────────┤
│      cmoc.h         │  isspace, isalpha, isdigit, tolower, toupper
└─────────────────────┘
```

---

## Inclusion

```c
#include "mo5_ctype.h"   // aucune dépendance externe
```

---

## Fonctions

### `islower`

```c
unsigned char islower(char c);
```

Retourne `1` si `c` est une lettre minuscule (`'a'`–`'z'`), `0` sinon.

```c
islower('a')   // → 1
islower('Z')   // → 0
islower('3')   // → 0
```

---

### `isupper`

```c
unsigned char isupper(char c);
```

Retourne `1` si `c` est une lettre majuscule (`'A'`–`'Z'`), `0` sinon.

```c
isupper('A')   // → 1
isupper('z')   // → 0
isupper(' ')   // → 0
```

---

### `isprint`

```c
unsigned char isprint(char c);
```

Retourne `1` si `c` est un caractère imprimable (ASCII 32–126 inclus), `0` sinon. Les caractères de contrôle (0–31) et DEL (127+) retournent `0`.

```c
isprint(' ')    // → 1  (espace inclus)
isprint('~')    // → 1
isprint('\n')   // → 0  (caractère de contrôle)
isprint(0)      // → 0
```

---

### `ispunct`

```c
unsigned char ispunct(char c);
```

Retourne `1` si `c` est un caractère de ponctuation, `0` sinon.

Plages ASCII couvertes :

| Plage | Exemples |
|---|---|
| 33–47 | `! " # $ % & ' ( ) * + , - . /` |
| 58–64 | `: ; < = > ? @` |
| 91–96 | `[ \ ] ^ _ `` ` |
| 123–126 | `{ \| } ~` |

```c
ispunct('!')   // → 1
ispunct('.')   // → 1
ispunct('a')   // → 0
ispunct(' ')   // → 0
```

---

## Fonctions disponibles dans `cmoc.h` — ne pas redéfinir

Ces fonctions couvrent les cas complémentaires et sont déjà fournies par CMOC :

| Fonction | Description |
|---|---|
| `isspace(int c)` | Espace, tabulation, saut de ligne |
| `isalpha(int c)` | Lettre minuscule ou majuscule |
| `isalnum(int c)` | Lettre ou chiffre |
| `isdigit(int c)` | Chiffre décimal `'0'`–`'9'` |
| `tolower(int c)` | Convertit en minuscule |
| `toupper(int c)` | Convertit en majuscule |

---

## Note sur les types de retour

Les fonctions retournent `unsigned char` (`0` ou `1`) et non `int` comme les équivalents CMOC. Ce choix évite les opérations 16-bit inutiles sur le 6809. Toute valeur non nulle étant vraie en C, le code existant reste compatible :

```c
if (isprint(ch)) { ... }   // fonctionne identiquement à la libc
```

---

## Exemple d'utilisation complète

```c
#include "mo5_ctype.h"
#include <cmoc.h>

void analyse_char(char c)
{
    if (isupper(c))       { /* lettre majuscule */ }
    else if (islower(c))  { /* lettre minuscule */ }
    else if (isdigit(c))  { /* chiffre          */ }
    else if (ispunct(c))  { /* ponctuation       */ }
    else if (isprint(c))  { /* autre imprimable  */ }
    else                  { /* caractère de contrôle */ }
}
```

---

*Voir `cmoc.h` pour `isspace`, `isalpha`, `isalnum`, `isdigit`, `tolower`, `toupper`.*
