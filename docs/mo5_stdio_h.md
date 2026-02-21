# `mo5_stdio.h` — I/O texte pour le Thomson MO5

> Mini-bibliothèque d'entrée/sortie pour le MO5. Couvre la lecture de caractères et chaînes, l'affichage de texte, et le contrôle de l'écran.

---

## Dépendance

```c
#include "mo5_defs.h"  // inclus automatiquement
```

---

## Macro

### `getchar`

```c
#define getchar mo5_getchar
```

Alias public vers `mo5_getchar()`. **Non bloquant** : retourne `0` si aucune touche n'est pressée. Voir `mo5_defs.h` pour les patterns de lecture bloquante.

---

## Fonctions d'entrée

### `int fgets(char *buffer, int max_length)`

Lit une chaîne depuis le clavier dans `buffer`.

- `max_length` : nombre maximum de caractères à lire, **null terminator non compris** (le buffer doit avoir `max_length + 1` octets).
- Retourne le nombre de caractères lus.
- Termine à l'appui sur `MO5_ENTER_CHAR` (13).
- Gère le **backspace** (`MO5_BACKSPACE_CHAR`) : supprime le dernier caractère et met à jour l'affichage (backspace + espace + backspace).
- **Écho automatique** de chaque caractère imprimable saisi.
- Les caractères non imprimables (contrôle, nul) sont ignorés. Utilise `isprint()` de `mo5_ctype.h`.

> ⚠️ `fgets` utilise `mo5_getchar()` en boucle interne — c'est lui qui assure le blocage jusqu'à la fin de saisie.

---

## Fonctions de sortie

### `void fputs(const char *s)`

Écrit la chaîne `s` (terminée par `'\0'`) caractère par caractère via `putchar` (alias `mo5_putchar`). Sans saut de ligne.

### `void puts(const char *s)`

Écrit la chaîne `s` via `fputs`, puis appelle `mo5_newline()` (CR+LF).

---

## Contrôle écran

### `void clrscr(void)`

Envoie `MO5_CLEAR_SCREEN` (code 12) via `putchar`. Efface l'écran et replace le curseur en position initiale.

---

## Exemple d'utilisation

```c
#include "mo5_stdio.h"

int main(void) {
    char buf[MO5_BUFFER_SIZE];

    clrscr();
    puts("Entrez votre nom :");
    fgets(buf, MO5_BUFFER_SIZE);
    fputs("Bonjour ");
    puts(buf);
    return 0;
}
```
