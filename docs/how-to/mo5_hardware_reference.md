# MO5 Hardware Reference — Documentation Technique Complète

> Version fusionnée : V6.0 — V5.0 + Guide du MO5 (Deledicq, Cedic-Nathan 1985)  
> Cible : IA + développeurs C bas niveau (CMOC)  
> Sources : Manuel Technique MO5 (Michel Oury / Cedic-Nathan 1985), Guide du MO5 (Deledicq, Cedic-Nathan 1985), Clefs Pour MO5 (Blanchard, PSI 1985), MAME `thomson.cpp` (Antoine Miné, BSD-3), sessions de débogage, docs SDK projet

---

# 1. CPU & ACCÈS MÉMOIRE

## Matériel

- CPU : Motorola 6809E à ~1 MHz
- RAM partagée entre CPU et vidéo
- Arbitrage géré par **gate-array custom** (≠ TO7 qui utilise un Motorola 6846)

## Modèle réel

```text
MEMORY ACCESS IS ARBITRATED
- CPU access is NOT continuous
- Video hardware performs periodic reads
- Gate-array schedules memory cycles
- Video priority > CPU priority
```

> ❌ NE PAS supposer : E=1 → CPU / E=0 → VIDEO  
> Ce modèle est celui du TO7, **pas du MO5**

## Conséquences pratiques

```text
- Latence mémoire variable
- Boucles serrées = ralentissement possible pendant affichage actif
- Privilégier les accès pendant le VBL (blanking vertical)
- Grouper les écritures RAM en rafales courtes
```

---

# 2. CARTE MÉMOIRE COMPLÈTE

| Plage | Contenu |
|-------|---------|
| `$0000–$1F3F` | RAM vidéo (FORME et COULEUR, bank-switched via `$A7C0` bit 0) |
| `$1F40–$1FFF` | Variables système (moniteur ROM) |
| `$2000–$20FF` | Registres du moniteur (voir section 7) |
| `$2100–$21FF` | Registres de l'application |
| `$2200–$9FFF` | RAM utilisateur |
| `$A000–$A7BF` | DOS (si disquettes) |
| `$A7C0–$A7C3` | PIA système (clavier, son buzzer, cassette, banque vidéo) |
| `$A7C4–$A7CB` | Libre |
| `$A7CC–$A7CF` | PIA extension jeux/musique (manettes, DAC son) |
| `$A7D0–$A7DF` | Contrôleur de disquettes |
| `$A7E0–$A7E3` | PIA interface communication (imprimante parallèle) |
| `$A7E4–$A7E7` | Compteurs crayon optique + gate-array vidéo (VBL) |
| `$A7E8–$A7FF` | Extensions |
| `$A800–$AFFF` | Libre |
| `$B000–$EFFF` | Cartouche ROM |
| `$F000–$FFFF` | ROM moniteur |

> **Pas de `malloc`** — tout est statique. Planifier l'occupation mémoire dès le départ.  
> Budget utilisateur typique : RAM `$2200`–`$9FFF` soit ~32 Ko, dont une partie pour pile et variables système.  
> Source : Clefs Pour MO5 p.78

---

# 3. MÉMOIRE VIDÉO

## Organisation

- Résolution : 320×200 pixels
- 1 octet = 8 pixels horizontaux
- 40 octets par ligne
- Base adresse : `$0000`

## Deux plans mémoire (bank-switched)

| Plan | Rôle | Sélection |
|------|------|-----------|
| FORME | 1 bit par pixel (allumé/éteint) | PIA `$A7C0` bit 0 = 0 |
| COULEUR | 4 bits d'attribut couleur | PIA `$A7C0` bit 0 = 1 |

## Adresse d'un pixel (X, Y)

```c
addr = y * 40 + (x >> 3);      /* octet contenant le pixel */
mask = 0x80 >> (x & 7);        /* masque du bit dans l'octet */
```

> **Optimisation critique** : ne jamais calculer `y * 40` à l'exécution.  
> Utiliser une table précalculée au démarrage :
> ```c
> unsigned int row_offsets[200];
> /* init une seule fois : */
> for (i = 0; i < 200; i++) row_offsets[i] = (unsigned int)i * 40;
> /* usage : */
> addr = row_offsets[y] + (x >> 3);
> ```

## Règle critique d'écriture

```text
WRITE_PIXEL:
1. Sélectionner FORME  (bit 0 = 0)
2. Modifier le bit
3. Sélectionner COULEUR (bit 0 = 1)
4. Écrire l'attribut couleur
```

> ⚠️ Ne jamais écrire en COULEUR sans avoir d'abord sélectionné la bonne banque.

---

# 4. SYNCHRONISATION VBL (VERTICAL BLANK)

## Le registre VBL

Le signal VBL est exposé par le **gate-array vidéo** (≠ PIA) au registre `$A7E7` bit 7 :

| Valeur bit 7 de `$A7E7` | Signification |
|:-----------------------:|---------------|
| `1` | Balayage actif (faisceau sur l'écran) |
| `0` | Retour vertical — fenêtre VBL |

> ⚠️ `$A7E7` appartient au **gate-array vidéo**, pas au PIA système (`$A7C0`).  
> Ne pas confondre ces deux zones d'I/O.

## Timing PAL (50 Hz)

```
|  balayage actif (~18.7ms)  | VBL (~1.2ms) |  balayage actif ...
0ms                        18.7ms          20ms
```

- **Budget total par frame** : ~20 000 cycles à 1 MHz
- **Fenêtre VBL** : ~1 200 cycles — période idéale pour écrire en VRAM

## Fonction d'attente VBL

```c
#define VBL_REG  ((unsigned char *)0xA7E7)
#define VBL_BIT  0x80

void mo5_wait_vbl(void)
{
    while ( *VBL_REG &  VBL_BIT) ;  /* attendre fin balayage actif */
    while (!(*VBL_REG & VBL_BIT)) ; /* attendre début prochain balayage */
}
```

> ⚠️ **cmoc n'a pas `volatile`**. Pour que cmoc relise effectivement le registre
> à chaque itération, ne pas déclarer cette fonction `static inline` — l'inlining
> peut provoquer l'optimisation (mise en cache) de la valeur lue.  
> Pas de timer programmable sur MO5 : l'attente active sur `$A7E7` est la seule
> méthode fiable.

## Registres gate-array complets ($A7E4–$A7E7)

Le gate-array MC1300 ALS expose 4 registres en lecture permettant de connaître
la position exacte du balayage. Source : Manuel Technique MO5 p.116.

| Adresse | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |
|---------|----|----|----|----|----|----|----|----|
| `$A7E4` | T12 | T11 | T10 | T9 | T8 | T7 | T6 | T5 |
| `$A7E5` | T4 | T3 | TL2 | TL1 | TL0 | H1 | H2 | H4 |
| `$A7E6` | LT3 | INILN | — | — | — | — | — | — |
| `$A7E7` | **INITN** | **INILN** | — | — | — | — | — | — |

Significations :
- **T3–T13** : compteur trame (position verticale dans la trame)
- **TL0–TL2** : compteur ligne (position horizontale dans la ligne, en octets)
- **H1/H2/H4** : horloges internes 1/2/4 MHz (position au niveau du pixel)
- **INITN** : 1 = balayage actif (fenêtre écran), 0 = retour vertical (VBL)
- **INILN** : 1 = balayage ligne actif (fenêtre), 0 = retour ligne

> Ces registres permettent théoriquement du **raster timing précis** — par exemple
> déclencher un traitement en milieu d'écran. En pratique pour un jeu, `$A7E7` bit 7
> (INITN) suffit pour la synchronisation VBL standard.

## Deux stratégies de game loop

### Stratégie A — VBL en début de boucle (simple)

```c
while (1) {
    mo5_wait_vbl();
    read_inputs();
    update_logic();
    draw_all();
}
```

Logique + dessin dans les 20ms de la trame complète. Simple, recommandé pour les jeux avec peu de sprites.

### Stratégie B — VBL juste avant le dessin (avancée)

```c
while (1) {
    read_inputs();
    update_logic();      /* pendant le balayage actif (~18.7ms) */
    mo5_wait_vbl();
    draw_all();          /* pendant la fenêtre VBL (~1.2ms) */
}
```

Maximise le temps CPU pour la logique, réserve la fenêtre VBL au dessin. Si `draw_all()` dépasse ~1 200 cycles, des artefacts peuvent apparaître en bas d'écran.

| | Stratégie A | Stratégie B |
|---|---|---|
| Simplicité | ✓ Simple | Plus structuré |
| Budget logique | ~20ms | ~18.7ms dédiés |
| Budget dessin | ~20ms | ~1.2ms dédiés |
| Risque tearing | Faible | Minimal |
| Usage | Jeux simples | Beaucoup d'entités |

## Budget cycles — opérations courantes

| Opération | Coût approx. | Notes |
|-----------|-------------|-------|
| `draw_sprite` 8×8 | ~50 cycles | |
| `clear_sprite` 8×8 | ~65 cycles | |
| `draw_sprite` 16×16 | ~200 cycles | |
| `clear_sprite` 16×16 | ~200 cycles | |
| `draw_sprite` 24×24 | ~450 cycles | |
| Switch banque VRAM | ~5 cycles | Par switch |
| Accès `row_offsets[y]` | ~8 cycles | Vs ~30+ pour `y*40` |
| Collision AABB | ~20 cycles | 4 comparaisons |
| Lecture touche PIA | ~10–20 cycles | |
| `mo5_font6_puts` (1 car.) | ~30–50 cycles | |
| LFSR pseudo-random | ~15 cycles | |

> Formule d'estimation pour un sprite W×H :  
> `coût ≈ (W_bytes × H × 5) + (H × 3)`  
> Valeurs théoriques — mesurer sur émulateur pour valeurs précises.

## Budget type — jeu avec 4 ennemis

| Opération | Coût estimé |
|-----------|------------|
| Joueur `move_sprite` 16×16 | ~200 cycles |
| 4 ennemis `move_sprite` 16×16 | ~800 cycles |
| 3 tirs joueur `move_sprite` 8×8 | ~150 cycles |
| 4 tirs ennemis `move_sprite` 8×8 | ~200 cycles |
| Collisions (paires catégorielles) | ~240 cycles |
| Score + vies (font6) | ~400 cycles |
| **Total estimé** | **~2 000 cycles** |

> Ce budget dépasse la fenêtre VBL seule (~1 200 cycles).  
> Utiliser la stratégie A (20ms), ou `move_sprite` optimisé (clear+draw fusionnés).

---

# 5. MODÈLE COULEUR

- 4 bits par pixel : RVB + intensité
- Conversion via circuit analogique (PROM)
- Le mapping n'est **pas linéaire RGB standard**

## Palette approximative (fiabilité MOYENNE — à vérifier sur vrai hardware)

| Code | Couleur | Codage binaire (R,V,B,demi) |
|------|---------|---------------------------|
| 0 | Noir | 0000 |
| 1 | Rouge | 0001 |
| 2 | Vert | 0010 |
| 3 | Jaune | 0011 |
| 4 | Bleu marine | 0100 |
| 5 | Magenta | 0101 |
| 6 | Bleu clair (Cyan) | 0110 |
| 7 | Blanc | 0111 |
| 8 | Gris | 1000 |
| 9 | Rouge pâle | 1001 |
| 10 | Vert pâle | 1010 |
| 11 | Jaune pâle | 1011 |
| 12 | Bleu | 1100 |
| 13 | Magenta pâle | 1101 |
| 14 | Bleu pâle (Cyan pâle) | 1110 |
| 15 | Orange | 1111 |

Codage binaire des 4 bits (du bit de poids faible au plus fort) :
- bit 0 : présence de **rouge**
- bit 1 : présence de **vert**
- bit 2 : présence de **bleu**
- bit 3 : **demi-teinte** (0=demi, 1=pleine) — exception : `1111` = Orange au lieu de Blanc demi-teinte

> Source confirmée : Clefs Pour MO5 p.140 (Blanchard, 1985) — fiabilité ÉLEVÉE.  
> Le codage est physique (RVB + intensité via PROM analogique), pas un index arbitraire.

---

# 6. PIA — MODÈLE COMPLET

## Architecture générale du 6821

Chaque PIA 6821 expose 4 registres via les bits A0/A1 du bus adresse :

| Offset | Registre | Accès |
|--------|----------|-------|
| +0 | ORA / DDRA | Données port A **ou** direction, selon CRA bit 2 |
| +1 | ORB / DDRB | Données port B **ou** direction, selon CRB bit 2 |
| +2 | CRA | Registre de contrôle A |
| +3 | CRB | Registre de contrôle B |

Règle d'accès DDR/OR :
- CRA bit 2 = 0 → accès **DDRA** (sens des broches : 0=entrée, 1=sortie)
- CRA bit 2 = 1 → accès **ORA** (données)
- Idem pour CRB/DDRB/ORB

> À la mise sous tension : DDR = 0 partout → toutes broches en entrée.  
> Il faut initialiser le DDR avant tout usage en sortie.

---

## PIA Système — $A7C0 à $A7C3

PIA principal, utilisé par le moniteur ROM.  
Le registre 6809 **U est initialisé à `$A7C0`** lors de chaque appel au moniteur (page 34 du manuel).

### $A7C0 — PORTA (ORA / DDRA)

Source : Manuel Technique MO5 p.41-42 (schéma LEP), p.18 (couleur tour), MAME `mo5_sys_porta_out`.

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0 | Sortie | **Sélection banque vidéo** : 0 = COULEUR (RAM couleur), 1 = FORME (RAM point) |
| 1 | Sortie | Couleur du tour — **Rouge** (PT) |
| 2 | Sortie | Couleur du tour — **Vert** (VT) |
| 3 | Sortie | Couleur du tour — **Bleu** (BT) |
| 4 | Sortie | Couleur du tour — **demi-teinte** (RT) |
| 5 | Entrée | Bouton crayon optique (interrupteur) |
| 6 | Sortie | **Écriture cassette** — broche 5 LEP (PA6) |
| 7 | Entrée | **Lecture cassette** — broche 4 LEP (PA7) |

> ⚠️ Les bits 1-4 encodent la couleur du cadre (tour) en RVB+demi-teinte,  
> identique au codage couleur VRAM. Le **bit 0** commute la banque vidéo.  
> Toujours utiliser un masque pour ne modifier qu'un bit à la fois.  
> Moteur cassette : géré via CA2 du CRA (bits CRA3-CRA5).

### $A7C1 — PORTB (ORB / DDRB) ★ BUZZER + CLAVIER

Source : Manuel Technique MO5 p.30-32 (matrice clavier), p.40 (son).

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0 | Sortie | **Son buzzer 1-bit** — PB0, via RC vers amplificateur son |
| 1 | Sortie | Sélection colonne clavier A (via 74LS156) |
| 2 | Sortie | Sélection colonne clavier B (via 74LS156) |
| 3 | Sortie | Sélection colonne clavier C (via 74LS156) |
| 4 | Sortie | Sélection ligne clavier A (via 74LS151) |
| 5 | Sortie | Sélection ligne clavier B (via 74LS151) |
| 6 | Sortie | Sélection ligne clavier C (via 74LS151) |
| 7 | Entrée | **Lecture état ligne clavier** (0 = touche pressée) |

> Le clavier est une matrice 8×8. PB1-PB3 sélectionnent la colonne (via démultiplexeur 74LS156),  
> PB4-PB6 sélectionnent la ligne (via multiplexeur 74LS151), PB7 lit l'état de la ligne.  
> Confirmation : "La sortie PB0 du PORTB du PIA 6821 système [...] sera la sortie SON PROGRAMME du MO5."  
> Source : Manuel Technique MO5 p.40.

**Méthode recommandée pour couper le bip clavier — via registre moniteur :**

La méthode propre est d'agir sur le registre STATUS du moniteur à `$2019` bit 3 :

```c
/* Couper le bip clavier via le registre moniteur $2019 bit 3 */
/* bit 3 = 0 → bruitage activé, bit 3 = 1 → bruitage désactivé */
unsigned char val = *((unsigned char*)0x2019);
val |= 0x08;
*((unsigned char*)0x2019) = val;
```

En BASIC : `POKE &H2019, PEEK(&H2019) + 8`  
Source : Clefs Pour MO5 p.110 et p.118.

**Méthode alternative — directement sur PORTB bit 0 :**
```c
unsigned char val = *((unsigned char*)0xA7C1);
val &= ~0x01;
*((unsigned char*)0xA7C1) = val;
```

> ⚠️ La méthode via `$2019` est préférable car elle désactive le bip  
> au niveau du moniteur sans risquer de perturber la matrice clavier.

### $A7C2 — CRA (Registre de contrôle A)

| Bits | Fonction |
|------|----------|
| 0-1 | CRA0/CRA1 : contrôle entrée CA1 et masque IRQ |
| 2 | 0 = accès DDRA / 1 = accès ORA |
| 3-5 | CRA3/CRA4/CRA5 : contrôle sortie CA2 |
| 6 | IRQA2 : flag interruption CA2 (lecture seule) |
| 7 | IRQA1 : flag interruption CA1 (lecture seule) |

CA2 → **moteur lecteur cassette** (`mo5_set_cassette_motor`).

### $A7C3 — CRB (Registre de contrôle B)

Même structure que CRA, pour le port B.

CB1 → **interruptions 50 Hz** (signal VBL).  
CB2 → **commande d'incrustation vidéo**.

IRQ B → **interruption principale du 6809**.

> Source : Clefs Pour MO5 p.96.  
> ⚠️ Différence TO7/MO5 : c'est **IRQB** (pas IRQA) qui génère l'IRQ principale.  
> Commentaire MAME : `// WARNING: differs from TO7 !`

**Inhibition complète du clavier (cas particulier) :**
```c
/* Mettre CRB à 4 désactive le clavier entier */
/* À utiliser uniquement si jeu manette exclusif */
*((unsigned char*)0xA7C3) = 4;
```
Source : Clefs Pour MO5 p.127.

---

## PIA Extension Jeux/Musique — $A7CC à $A7CF

Extension **optionnelle** sur MO5 de base.  
**Intégrée de série** sur : MO5E, MO6, MO5NR.  
Source : Clefs Pour MO5 p.98-100 et p.107 (câblage exact des connecteurs DIN).

```text
$A7CC  PORTA  (et DDRA)   → directions manettes (actif bas)
$A7CD  PORTB  (et DDRB)   → DAC son 6 bits + boutons fire
$A7CE  CRA                → CA1 = bouton manette 0
$A7CF  CRB                → CB1 = bouton manette 1
```

### $A7CC — PORTA (directions manettes)

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0 | Entrée | Manette 0 — HAUT (actif bas) |
| 1 | Entrée | Manette 0 — BAS (actif bas) |
| 2 | Entrée | Manette 0 — GAUCHE (actif bas) |
| 3 | Entrée | Manette 0 — DROITE (actif bas) |
| 4 | Entrée | Manette 1 — HAUT (actif bas) |
| 5 | Entrée | Manette 1 — BAS (actif bas) |
| 6 | Entrée | Manette 1 — GAUCHE (actif bas) |
| 7 | Entrée | Manette 1 — DROITE (actif bas) |

### $A7CD — PORTB — DAC son 6 bits + boutons fire ★

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0–5 | Sortie (si init DAC) | **DAC numérique/analogique 6 bits** |
| 6 | Entrée | **Bouton fire manette 0** (actif bas) |
| 7 | Entrée | **Bouton fire manette 1** (actif bas) |

> Source confirmée : Clefs Pour MO5 p.99 et p.107 :  
> "B6 : Poussoir manette 0 (relié au CA1 du PIA). B7 : Poussoir manette 1 (relié au CA2 du PIA)."  
> Les bits 6-7 doivent rester en **entrées** même si les bits 0-5 sont configurés en sorties pour le DAC.

### $A7CC — PORTA (manettes)

| Bits | Direction | Fonction |
|------|-----------|----------|
| 0-3 | Entrée | Manette 1 (haut/bas/gauche/droite) |
| 4-7 | Entrée | Manette 2 (haut/bas/gauche/droite) |
| CA1 | Entrée | Bouton manette 1 |
| CA2 | Entrée | Bouton manette 2 |

### $A7CD — PORTB — DAC son 6 bits ★

| Bits | Direction | Fonction |
|------|-----------|----------|
| 0-5 | Sortie | **DAC numérique/analogique 6 bits (son)** |
| 6-7 | Entrée | Non utilisés |

**Initialisation obligatoire** (extrait manuel p. 48) :
```asm
CLR  $A7CF        ; CRB = 0 → accès DDRB
LDD  #$3F04
STA  $A7CD        ; DDRB : bits B0-B5 en sorties (0x3F)
STB  $A7CF        ; CRB bit 2 = 1 → accès PORTB
```

En C (cmoc) :
```c
*((unsigned char*)0xA7CF) = 0x00;   /* accès DDRB */
*((unsigned char*)0xA7CD) = 0x3F;   /* B0-B5 en sorties */
*((unsigned char*)0xA7CF) = 0x04;   /* accès PORTB */
```

Écriture d'un échantillon :
```c
*((unsigned char*)0xA7CD) = valeur & 0x3F;  /* 0 à 63 */
```

---

## Résumé carte mémoire PIA

| Adresse | Nom | Fonction principale | Fiabilité |
|---------|-----|---------------------|-----------|
| `$A7C0` | PIA sys PORTA | Banque vidéo (bit 0), cassette, crayon optique | ÉLEVÉE |
| `$A7C1` | PIA sys PORTB | **Buzzer** (bit 0), matrice clavier | ÉLEVÉE (MAME) |
| `$A7C2` | PIA sys CRA | Contrôle PORTA, CA2 = moteur cassette | ÉLEVÉE |
| `$A7C3` | PIA sys CRB | Contrôle PORTB, IRQ principale | ÉLEVÉE |
| `$A7CC` | PIA ext PORTA | Manettes de jeu (entrées) | ÉLEVÉE (manuel) |
| `$A7CD` | PIA ext PORTB | **DAC son 6 bits** (bits 0-5 sortie) | ÉLEVÉE (manuel) |
| `$A7CE` | PIA ext CRA | Contrôle PORTA extension | ÉLEVÉE |
| `$A7CF` | PIA ext CRB | Contrôle PORTB extension | ÉLEVÉE |

---

# 7. MINI SDK C (CMOC)

## Contraintes cmoc spécifiques

> ⚠️ cmoc compile du **C89/C90 strict**. Deux règles critiques souvent source de bugs :
>
> **1. Toutes les déclarations en tête de bloc, avant tout statement :**
> ```c
> /* ❌ INTERDIT — déclaration après un statement */
> void f(void) {
>     game_init();
>     unsigned char x;   /* crash ou comportement indéfini */
> }
>
> /* ✅ CORRECT */
> void f(void) {
>     unsigned char x;   /* déclarations en premier */
>     game_init();       /* statements ensuite */
> }
> ```
>
> **2. Ne pas initialiser à la déclaration :**
> ```c
> unsigned char x = 42;   /* ❌ peut crasher avec cmoc */
> unsigned char x;
> x = 42;                  /* ✅ */
> ```
>
> cmoc ne génère pas toujours d'erreur visible — le programme se compile mais
> le comportement au runtime est imprévisible.

## Macros de base

```c
/* Accès mémoire */
#define PEEK(addr)      (*(unsigned char*)(addr))
#define POKE(addr, val) (*(unsigned char*)(addr) = (val))

/* Vidéo */
#define SCREEN          ((unsigned char*)0x0000)
#define PIA_PORTA       (*(unsigned char*)0xA7C0)

/* VBL */
#define VBL_REG         ((unsigned char*)0xA7E7)
#define VBL_BIT         0x80

/* Son */
#define BUZZER          (*(unsigned char*)0xA7C1)
#define DAC             (*(unsigned char*)0xA7CD)
```

## Sélection banque vidéo

```c
static inline void mo5_select_form(void)
{
    PIA_PORTA &= ~0x01;
}

static inline void mo5_select_color(void)
{
    PIA_PORTA |= 0x01;
}
```

## Attente VBL

```c
/* Ne pas déclarer static inline — risque d'optimisation de la relecture */
void mo5_wait_vbl(void)
{
    while ( *VBL_REG &  VBL_BIT) ;
    while (!(*VBL_REG & VBL_BIT)) ;
}
```

## Écriture pixel

```c
static unsigned char bit_table[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

void mo5_set_pixel(int x, int y, unsigned char color)
{
    unsigned int  addr = (unsigned int)y * 40 + ((unsigned int)x >> 3);
    unsigned char mask = bit_table[x & 7];

    mo5_select_form();
    SCREEN[addr] |= mask;

    mo5_select_color();
    SCREEN[addr] = color;
}
```

## Couper le bip clavier

Méthode recommandée — via le registre STATUS du moniteur `$2019` bit 3 :

```c
/* bit 3 de $2019 : 0 = bip activé, 1 = bip désactivé */
/* Source : Clefs Pour MO5 p.110 et p.118 */
void mo5_mute_beep(void)
{
    unsigned char val = *((unsigned char*)0x2019);
    val |= 0x08;
    *((unsigned char*)0x2019) = val;
}

void mo5_unmute_beep(void)
{
    unsigned char val = *((unsigned char*)0x2019);
    val &= ~0x08;
    *((unsigned char*)0x2019) = val;
}
```

## Initialiser et jouer un son DAC

```c
void mo5_init_dac(void)
{
    *((unsigned char*)0xA7CF) = 0x00;  /* accès DDRB */
    *((unsigned char*)0xA7CD) = 0x3F;  /* B0-B5 en sorties */
    *((unsigned char*)0xA7CF) = 0x04;  /* accès PORTB */
}

void mo5_dac_write(unsigned char val)
{
    *((unsigned char*)0xA7CD) = val & 0x3F;
}
```

---

# 8. PATTERNS MO5-SPÉCIFIQUES

## 8.1 Minimiser les bank switches

```text
❌ FORM → COLOR → FORM → COLOR  (un pixel à la fois)
✔  FORM (ligne entière) → COLOR (ligne entière)
```

## 8.2 Dirty rectangles (quasi-obligatoire)

```text
Ne mettre à jour que les zones modifiées.
Performance clé sur MO5 vu l'arbitrage mémoire.
```

## 8.3 Sprite masqué

```c
dst = (dst & mask) | sprite;   /* propre mais plus lent */
dst ^= sprite;                  /* rapide mais effacement destructif */
```

## 8.4 Double buffering

```text
⚠️ RAM limitée (32 Ko user) → pas de vrai double buffer possible.
Solution : buffer partiel + dirty rectangles.
```

## 8.5 Accès groupés

```text
Regrouper les écritures mémoire en rafales courtes.
Éviter les accès dispersés pendant l'affichage actif.
Privilégier les gros transferts pendant le VBL.
```

## 8.6 Variables statiques globales (éviter le stack overflow)

cmoc alloue les variables locales sur la pile via `LEAS -N,S`. Un trop grand nombre
de locales dans une fonction peut provoquer un **stack overflow silencieux** — le
programme freeze à l'entrée de la fonction, avant d'exécuter la moindre instruction.

```c
/* ❌ Trop de locales → crash au démarrage de la fonction */
void game_loop(void) {
    unsigned char score, live, new_x, i;
    unsigned char enemies_tick, bullets_tick, result;  /* une de trop */
}

/* ✅ Promouvoir en static global (coût stack = 0) */
static unsigned char gl_score, gl_live, gl_new_x, gl_i;
static unsigned char gl_enemies_tick, gl_bullets_tick, gl_result;

void game_loop(void) { /* frame quasi-vide → stack check passe */ }
```

---

# 10. REGISTRES MONITEUR RAM ($2000–$20FF)

Registres clés utilisables depuis le C. Source : Guide du MO5 p.275-277 + Clefs Pour MO5 p.91-94.

| Adresse | Nom | Taille | Description |
|---------|-----|--------|-------------|
| `$2019` | STATUS | 1 octet | Bits divers système (voir détail ci-dessous) |
| `$2029` | FORME | 1 octet | Code couleur courant pour point/ligne |
| `$202B` | COLOUR | 1 octet | Couleur courante (FFFFBBBB) |
| `$2032–$2033` | PLOTX | 2 octets | Abscisse dernier point allumé |
| `$2034–$2035` | PLOTY | 2 octets | Ordonnée dernier point allumé |
| `$2036` | CHDRAW | 1 octet | 0 = tracé graphique, sinon code caractère |
| `$2037` | KEY | 1 octet | Code matriciel dernière touche enfoncée |
| `$2038` | CMPTKB | 1 octet | Compteur répétition clavier |
| `$2039–$203A` | TEMPO | 2 octets | Tempo musique SWI (1=rapide, 5=std, 255=lent) |
| `$203B–$203C` | DUREE | 2 octets | Durée note musique SWI (RONDE=96, NOIRE=24) |
| `$203D` | TIMBRE | 1 octet | Attaque musique SWI (0=continu, 200=piqué) |
| `$203E–$203F` | OCTAVE | 2 octets | Octave musique SWI (1–5, 4=LA440) |
| `$2061–$2062` | TIMEPT | 2 octets | Adresse routine traitement IRQ 50Hz personnalisée |
| `$2063` | — | 1 octet | ≠0 pour activer l'aiguillage IRQ personnalisé |
| `$206D–$206E` | CHRPTR | 2 octets | Pointeur table décodage clavier (remplaçable) |
| `$2070–$2071` | USERAF | 2 octets | Pointeur générateur caractères utilisateur |
| `$2073–$2074` | GENPTR | 2 octets | Pointeur table caractères standard (remplaçable) |
| `$2076` | LATCLV | 1 octet | Latence répétition clavier (en 1/10 sec, défaut=7) |

## Détail du registre STATUS ($2019)

| Bit | Fonction |
|-----|----------|
| 0 | Touche clavier déjà lue |
| 1 | Répétition clavier |
| 2 | État curseur (0=invisible, 1=visible) |
| 3 | **Bruitage clavier : 0=activé, 1=désactivé** ★ |
| 4 | Lecture clavier graphique sans écriture couleur |
| 5 | Scroll caractère sans couleur |
| 6 | 0=majuscule, 1=minuscule |
| 7 | 0=simple hauteur, 1=double hauteur |

**Usage en C :**
```c
/* Couper le bip clavier */
*((unsigned char*)0x2019) |= 0x08;

/* Remettre le bip clavier */
*((unsigned char*)0x2019) &= ~0x08;

/* Changer la latence de répétition clavier (ici 2 secondes) */
*((unsigned char*)0x2076) = 20;
```

---

# 11. CODES SWI MONITEUR

Appel via `SWI` suivi du code. Deux codes par fonction : JSR (retour après) et JMP (retour avant).  
Source : Guide du MO5 p.274 + Clefs Pour MO5 p.79-80.

| N° | Code JSR | Code JMP | Fonction |
|----|----------|----------|----------|
| 0 | `$02` | `$82` | Affichage d'un caractère (accum. B = code) |
| 1 | `$04` | `$84` | Mise en mémoire couleur |
| 2 | `$06` | `$86` | Mise en mémoire forme |
| 3 | `$08` | `$88` | **Bip sonore** |
| 4 | `$0A` | `$8A` | Lecture clavier (retourne ASCII dans B) |
| 5 | `$0C` | `$8C` | Lecture rapide clavier (test bit Z du CC) |
| 6 | `$0E` | `$8E` | Tracé d'un segment |
| 7 | `$10` | `$90` | Allumage d'un point |
| 8 | `$12` | `$92` | Écriture d'un point caractère |
| 9 | `$14` | `$94` | Lecture couleur d'un point |
| 10 | `$16` | `$96` | Lecture bouton crayon optique |
| 11 | `$18` | `$98` | Lecture crayon optique |
| 12 | `$1A` | `$9A` | Lecture de l'écran |
| 13 | `$1C` | `$9C` | **Lecture manettes de jeu** |
| 14 | `$1E` | `$9E` | **Génération de musique** |
| 15 | `$20` | `$A0` | Lecture/écriture cassette |
| 16 | `$22` | `$A2` | Moteur LEP (marche/arrêt) |
| 17 | `$24` | `$A4` | Interface communication |
| 18 | `$26` | `$A6` | Contrôleur disques |

> Ces codes sont utilisables depuis C via `asm { swi \n fcb $XX }` (cmoc).

---

## 11.1 SWI $1C — Lecture manettes de jeu

Source : Guide du MO5 p.260.

**Entrée :** registre A = numéro de manette (0 ou 1)

**Retour :**
- registre B = position (0–8) selon table cardinale
- bit C (retenue) du registre CC = 1 si bouton fire enfoncé

| Valeur B | Direction |
|----------|-----------|
| 0 | Centre (aucune direction) |
| 1 | Nord (haut) |
| 2 | Nord-Est |
| 3 | Est (droite) |
| 4 | Sud-Est |
| 5 | Sud (bas) |
| 6 | Sud-Ouest |
| 7 | Ouest (gauche) |
| 8 | Nord-Ouest |

> Note SDK : le SDK `mo5_joystick` lit directement les registres PIA (plus rapide, edge detection).
> Ce SWI est utile si on veut utiliser la routine moniteur sans SDK.

---

## 11.2 SWI $1E — Génération de musique

Source : Guide du MO5 p.262-263.

**Entrée :** registre B = code de note (voir table ci-dessous)

**Registres RAM à initialiser avant l'appel :**

| Registre | Adresse | Contenu |
|----------|---------|---------|
| `TEMPO` | `$2039–$203A` | Tempo (1=rapide, 5=standard, 255=très lent) |
| `DUREE` | `$203B–$203C` | Durée de la note (voir table durées) |
| `TIMBRE` | `$203D` | Timbre/attaque (0=continu, 200=piqué) |
| `OCTAVE` | `$203E–$203F` | Octave (1=grave, 4=LA440, 5=aigu) |

### Table des codes de notes (registre B)

| Note | Code hex | Note | Code hex |
|------|----------|------|----------|
| SILENCE | `$00` | SOL | `$08` |
| DO | `$01` | SOL# | `$09` |
| DO# | `$02` | LA | `$0A` |
| RE | `$03` | LA# | `$0B` |
| RE# | `$04` | SI | `$0C` |
| MI | `$05` | UT (DO+1 oct) | `$0D` |
| FA | `$06` | — | — |
| FA# | `$07` | — | — |

### Table des durées (registre DUREE)

| Note | Valeur |
|------|--------|
| RONDE | 96 |
| BLANCHE pointée | 72 |
| BLANCHE | 48 |
| NOIRE pointée | 36 |
| NOIRE (standard) | 24 |
| CROCHE pointée | 18 |
| CROCHE | 12 |
| DOUBLE CROCHE pointée | 9 |
| DOUBLE CROCHE | 6 |
| TRIPLE CROCHE pointée | 5 |
| TRIPLE CROCHE | 3 |
| NOIRE en triolet | 16 |
| CROCHE en triolet | 8 |
| DOUBLE CROCHE en triolet | 4 |

### Octaves disponibles

5 octaves (1 à 5). Octave 4 = LA 440 Hz (référence standard).

### Appel depuis C (cmoc)

```c
/* Jouer un DO noire à l'octave 4 */
static void swi_play_note(unsigned char note, unsigned char duree,
                           unsigned char octave, unsigned char tempo)
{
    *((unsigned char*)0x2039) = 0;
    *((unsigned char*)0x203A) = tempo;
    *((unsigned char*)0x203B) = 0;
    *((unsigned char*)0x203C) = duree;
    *((unsigned char*)0x203D) = 0;     /* timbre continu */
    *((unsigned char*)0x203E) = 0;
    *((unsigned char*)0x203F) = octave;
    asm {
        ldb note
        swi
        fcb $1E
    }
}
```

> ⚠️ Le SWI musique est **bloquant** — le CPU attend la fin de la note.
> Pour un jeu, préférer `mo5_audio` (buzzer direct) ou une lib non bloquante pilotée par VBL.

---

# 12. ANTI-PATTERNS

```text
❌ Supposer un mapping couleur RGB linéaire
❌ Écrire en vidéo sans sélectionner la bonne banque
❌ Utiliser malloc
❌ Utiliser des divisions à l'exécution (préférer >> ou lookup tables)
❌ Supposer E=1/E=0 comme sur TO7
❌ Accès mémoire dispersés pendant affichage actif
❌ Framebuffer linéaire (pas assez de RAM)
❌ Déclarer des variables après un statement en cmoc (C89)
❌ Initialiser une variable à sa déclaration en cmoc
❌ Déclarer static inline mo5_wait_vbl() (risque d'optimisation registre)
❌ Trop de variables locales dans une fonction (stack overflow silencieux)
❌ Utiliser int là où unsigned char suffit (opérations 16-bit inutiles)
```

---

# 13. TABLEAU DE FIABILITÉ

| Élément | Fiabilité | Source |
|---------|-----------|--------|
| Carte mémoire générale | ÉLEVÉE | Manuel Technique p.60 + Clefs Pour MO5 p.78 + Guide p.273 |
| Mapping pixel vidéo | ÉLEVÉE | Manuel Technique p.8-12 + Clefs p.112 |
| VBL registre `$A7E7` bit 7 (INITN) | ÉLEVÉE | Manuel Technique p.116 |
| Registres gate-array `$A7E4`–`$A7E7` | ÉLEVÉE | Manuel Technique p.116 |
| Budget cycles (estimations) | MOYENNE | Calcul théorique, à mesurer |
| PORTA `$A7C0` bit 0 = banque vidéo | ÉLEVÉE | Manuel Technique p.18 + MAME + Guide p.278 |
| PORTA `$A7C0` bits 1-4 = couleur tour | ÉLEVÉE | Manuel Technique p.18 + Guide p.278 |
| PORTA `$A7C0` bits 6-7 = cassette | ÉLEVÉE | Manuel Technique p.41-42 + Guide p.278 |
| PORTB `$A7C1` bit 0 = son buzzer | ÉLEVÉE | Manuel Technique p.40 + Guide p.278 |
| PORTB `$A7C1` bits 1-7 = clavier | ÉLEVÉE | Manuel Technique p.30-32 + Guide p.278 |
| Bip clavier via `$2019` bit 3 | ÉLEVÉE | Clefs Pour MO5 p.110 et p.118 |
| CRB `$A7C3` CB1=50Hz CB2=incrustation | ÉLEVÉE | Clefs Pour MO5 p.96 |
| PIA extension directions `$A7CC` | ÉLEVÉE | Manuel Technique p.48-49 |
| Boutons fire PORTB `$A7CD` bits 6-7 | ÉLEVÉE | Clefs Pour MO5 p.99 et p.107 |
| Init DAC (séquence DDR) | ÉLEVÉE | Manuel Technique p.48-50 |
| Palette couleurs (codage RVB+demi) | ÉLEVÉE | Manuel Technique p.13 + Clefs p.140 + Guide p.249 |
| Registres moniteur RAM `$2000`–`$20FF` | ÉLEVÉE | Guide du MO5 p.275-277 + Clefs p.91-94 |
| Codes SWI moniteur (table complète) | ÉLEVÉE | Guide du MO5 p.274 + Clefs p.79-80 |
| SWI $1C manettes (table cardinale) | ÉLEVÉE | Guide du MO5 p.260 |
| SWI $1E musique (notes + durées) | ÉLEVÉE | Guide du MO5 p.262-263 |
| Registres musique TEMPO/DUREE/OCTAVE | ÉLEVÉE | Guide du MO5 p.262-263 |
| Contraintes cmoc C89 | ÉLEVÉE | Observé en pratique sur projet |
| Timing bus/gate-array | MOYENNE | Manuel Technique p.22-26 |

---

# 14. SOURCES

| Source | Contenu | Fiabilité |
|--------|---------|-----------|
| Manuel Technique MO5 (Michel Oury, Cedic-Nathan 1985) | Hardware complet : PIA, gate-array, clavier, son, cassette, vidéo. Schémas électroniques. | Référence primaire |
| Guide du MO5 (Deledicq, Cedic-Nathan 1985, 2e éd.) | Moniteur complet : SWI table, registres $2000-$20FF, SWI musique détaillé, SWI manettes, PIA système p.278 | Très élevée |
| Clefs Pour MO5 (Blanchard, PSI 1985) | Carte mémoire, registres PIA bit à bit, SWI, clavier, couleurs | Très élevée |
| MAME `thomson.cpp` (Antoine Miné, BSD-3) | Câblage exact PIA, buzzer | Très élevée |
| Sessions débogage Claude | Corrections V2/V3/V4/V5/V6 | Élevée (vérifiée sources primaires) |
| Docs SDK projet (vbl, cmoc, optim) | VBL, budget cycles, contraintes cmoc | Élevée (observé en pratique) |
