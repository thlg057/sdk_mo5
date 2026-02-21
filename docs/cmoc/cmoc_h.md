# `cmoc.h` — Référence pour le développement Thomson MO5

> **Statut global** : ✅ Largement utilisable. Ce header contient la bibliothèque standard de CMOC. Les fonctions génériques (chaînes, mémoire, maths, tri) sont portables. Certaines fonctions dépendent du terminal/écran CoCo et doivent être utilisées avec précaution ou remplacées.

---

## Types définis

```c
typedef unsigned      size_t;
typedef signed        ssize_t;
```

---

## ✅ Entrées/Sorties texte — À adapter pour le MO5

Ces fonctions écrivent sur la sortie console. Elles **fonctionnent** si tu rediriges la sortie via `setConsoleOutHook()` vers la routine d'affichage du MO5 (`$E803` ou équivalent BASIC Thomson).

| Fonction | Description |
|---|---|
| `int printf(const char *format, ...)` | Formatage vers la sortie standard. Supporte `%u %d %x %X %p %s %c %%` et largeur de champ. |
| `int sprintf(char *dest, const char *format, ...)` | Comme `printf` mais écrit dans un buffer. **Totalement sûr sur MO5.** |
| `int vprintf(const char *format, va_list ap)` | Variante de `printf` avec `va_list`. |
| `int vsprintf(char *dest, const char *format, va_list ap)` | Variante de `sprintf` avec `va_list`. **Totalement sûr sur MO5.** |
| `void putstr(const char *s, size_t n)` | Écrit `n` caractères (y compris les nuls). Dépend du terminal. |
| `void putchar(int c)` | Écrit un caractère. Dépend du terminal. |

> 💡 **Conseil MO5** : Appelle `setConsoleOutHook()` en début de `main()` pour rediriger `printf` vers l'écran Thomson. La routine doit recevoir le caractère dans le registre A et préserver B, X, Y, U.

---

## ✅ Chaînes de caractères — Totalement portables

| Fonction | Description |
|---|---|
| `int strcmp(const char *s1, const char *s2)` | Comparaison de chaînes (sensible à la casse). |
| `int stricmp(const char *s1, const char *s2)` | Comparaison insensible à la casse. |
| `int strncmp(const char *s1, const char *s2, size_t n)` | Comparaison limitée à `n` caractères. |
| `size_t strlen(const char *s)` | Longueur d'une chaîne. |
| `char *strcpy(char *dest, const char *src)` | Copie de chaîne. |
| `char *strncpy(char *dest, const char *src, size_t n)` | Copie limitée à `n` caractères. |
| `char *strcat(char *dest, const char *src)` | Concaténation. |
| `char *strchr(const char *s, int c)` | Cherche la première occurrence de `c`. |
| `char *strrchr(const char *s, int c)` | Cherche la dernière occurrence de `c`. |
| `char *strstr(const char *haystack, const char *needle)` | Cherche une sous-chaîne. |
| `char *strlwr(char *s)` | Convertit en minuscules. |
| `char *strupr(char *s)` | Convertit en majuscules. |
| `size_t strspn(const char *s, const char *accept)` | Longueur du préfixe composé de caractères de `accept`. |
| `size_t strcspn(const char *s, const char *reject)` | Longueur du préfixe sans caractères de `reject`. |
| `char *strtok(char *str, const char *delim)` | Découpe une chaîne. Non réentrant. |
| `char *strpbrk(const char *s, const char *accept)` | Cherche le premier caractère appartenant à `accept`. |

---

## ✅ Mémoire — Totalement portables

| Fonction | Description |
|---|---|
| `int memcmp(const void *s1, const void *s2, size_t n)` | Comparaison de blocs mémoire. |
| `int memicmp(const void *s1, const void *s2, size_t n)` | Comparaison insensible à la casse. |
| `void *memchr(const void *s, int c, size_t n)` | Cherche `c` dans un bloc. |
| `void *memichr(const void *s, int c, size_t n)` | Idem, insensible à la casse. |
| `void *memcpy(void *dest, const void *src, size_t n)` | Copie de bloc mémoire. |
| `void *memmove(void *dest, const void *src, size_t n)` | Copie sûre (zones qui se chevauchent). |
| `void *memset(void *s, int c, size_t n)` | Remplit un bloc avec un octet. |
| `void *memset16(void *dest, unsigned short w, size_t num16BitWords)` | Remplit avec un mot 16 bits. Utile pour la VRAM MO5. |

---

## ✅ Classification de caractères — Totalement portables

| Fonction | Description |
|---|---|
| `int isspace(int c)` | Vrai si espace/tab/newline. |
| `int isalpha(int c)` | Vrai si lettre. |
| `int isalnum(int c)` | Vrai si lettre ou chiffre. |
| `int isdigit(int c)` | Vrai si chiffre décimal. |
| `int tolower(int c)` | Convertit en minuscule. |
| `int toupper(int c)` | Convertit en majuscule. |

---

## ✅ Conversions numériques — Totalement portables

| Fonction | Description |
|---|---|
| `unsigned atoui(const char *s)` | ASCII → entier non signé 16 bits. |
| `int atoi(const char *s)` | ASCII → entier signé 16 bits. |
| `long atol(const char *s)` | ASCII → entier signé 32 bits. |
| `unsigned long atoul(const char *s)` | ASCII → entier non signé 32 bits. |
| `unsigned long atoul16(const char *s)` | ASCII hexadécimal → entier non signé 32 bits. |
| `long atol16(const char *s)` | ASCII hexadécimal → entier signé 32 bits. |
| `char *itoa10(int value, char *str)` | Entier signé 16 bits → ASCII décimal. |
| `char *utoa10(unsigned value, char *str)` | Entier non signé 16 bits → ASCII décimal. |
| `char *ltoa10(long value, char *str)` | Entier signé 32 bits → ASCII décimal. |
| `char *ultoa10(unsigned long value, char *str)` | Entier non signé 32 bits → ASCII décimal. |
| `char *ultoa16(unsigned long value, char *str)` | Entier non signé 32 bits → ASCII hexadécimal majuscule. |
| `char *dwtoa(char *out, unsigned hi, unsigned lo)` | Double-mot (hi:lo) → ASCII décimal. Buffer de 11 octets minimum. |
| `unsigned long strtoul10(const char *nptr, char **endptr)` | Chaîne décimale → `unsigned long`. |
| `unsigned long strtoul16(const char *nptr, char **endptr)` | Chaîne hexa → `unsigned long`. |

---

## ✅ Mathématiques entières — Totalement portables

| Fonction | Description |
|---|---|
| `int abs(int j)` | Valeur absolue 16 bits. |
| `long int labs(long int j)` | Valeur absolue 32 bits. |
| `unsigned char sqrt16(unsigned short n)` | Racine carrée entière d'un 16 bits. |
| `unsigned short sqrt32(unsigned long n)` | Racine carrée entière d'un 32 bits. |
| `void divmod16(unsigned dividend, unsigned divisor, unsigned *quotient, unsigned *remainder)` | Division + reste 16 bits en une opération. |
| `void divmod8(unsigned char dividend, unsigned char divisor, unsigned char *quotient, unsigned char *remainder)` | Division + reste 8 bits en une opération. |
| `void divdwb(unsigned dividendInQuotientOut[2], unsigned char divisor)` | Division 32 bits par 8 bits. |
| `void divdww(unsigned dividendInQuotientOut[2], unsigned divisor)` | Division 32 bits par 16 bits. |
| `unsigned mulwb(unsigned char *hi, unsigned wordFactor, unsigned char byteFactor)` | Multiplication 16×8 bits avec retenue. |
| `unsigned mulww(unsigned *hi, unsigned factor0, unsigned factor1)` | Multiplication 16×16 bits avec retenue. |

---

## ✅ Arithmétique 32 bits — Totalement portables

| Fonction | Description |
|---|---|
| `void zerodw(unsigned *twoWords)` | Met à zéro un double-mot (2 × 16 bits). |
| `void adddww(unsigned *twoWords, unsigned term)` | Ajoute un 16 bits à un 32 bits. |
| `void subdww(unsigned *twoWords, unsigned term)` | Soustrait un 16 bits d'un 32 bits. |
| `signed char cmpdww(unsigned left[2], unsigned right)` | Compare un 32 bits à un 16 bits. Retourne -1, 0 ou +1. |

---

## ✅ Tri et recherche — Totalement portables

| Fonction | Description |
|---|---|
| `void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))` | Tri rapide générique. Récursif (utilise la pile). |
| `void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))` | Recherche dichotomique. Récursif. |

---

## ✅ Aléatoire — Portable

| Fonction | Description |
|---|---|
| `void srand(unsigned seed)` | Initialise le générateur. |
| `int rand(void)` | Retourne un entier pseudo-aléatoire dans `[0, RAND_MAX]` (`RAND_MAX = 0x7FFF`). |

---

## ✅ Allocation mémoire — Utilisable avec précaution sur MO5

| Fonction | Description |
|---|---|
| `void *sbrk(size_t increment)` | Déplace le pointeur de fin de tas. Retourne `(void *)-1` en cas d'échec. |
| `size_t sbrkmax(void)` | Retourne le nombre d'octets encore allouables. |

> ⚠️ **MO5** : Vérifie que la carte mémoire CMOC place le tas dans une zone RAM libre (pas en conflit avec la VRAM à `$0000–$1FFF` ni le BASIC ROM). Utilise `sbrkmax()` pour connaître l'espace disponible.

---

## ✅ Utilitaires système

| Fonction | Description |
|---|---|
| `void exit(int status)` | Retourne à l'environnement appelant (BASIC Thomson). Ne pas appeler si le BASIC est paginé hors mémoire. |
| `ConsoleOutHook setConsoleOutHook(ConsoleOutHook routine)` | **Crucial pour MO5** : redirige `printf`/`putchar` vers ta propre routine d'affichage. Retourne l'ancienne routine. |
| `void set_null_ptr_handler(void (*newHandler)(void *))` | Installe un handler de déréférencement nul. |
| `void set_stack_overflow_handler(void (*newHandler)(void *, void *))` | Installe un handler de débordement de pile. |

---

## ⚠️ Fonctions dépendantes du terminal CoCo — À remplacer sur MO5

| Fonction | Problème | Alternative MO5 |
|---|---|---|
| `void delay(size_t sixtiethsOfASecond)` | Calibré sur le 60 Hz du CoCo. | Écrire une boucle de délai calibrée pour le MO5 (1 MHz). |
| `unsigned readword(void)` | Lit depuis le terminal CoCo. | Implémenter via le clavier Thomson. |
| `char *readline(void)` | Lit une ligne depuis le terminal CoCo. | Implémenter via le clavier Thomson. |

---

## ❌ Fonctions virgule flottante — Non disponibles sans options spécifiques

Les fonctions `strtof`, `atoff`, `ftoa`, `logf`, `log2f` nécessitent les flags `--mc6839`, `_COCO_BASIC_`, `DRAGON` ou `_CMOC_NATIVE_FLOAT_`. Non pertinentes pour une cible MO5 standard.

---

## Constantes utiles

```c
#define NULL     ((void *) 0)
#define SIZE_MAX  0xFFFFu
#define SSIZE_MAX 0x7FFF
#define RAND_MAX  0x7FFF
#define offsetof(Type, member)  ((unsigned) &((Type *) 0)->member)
```
