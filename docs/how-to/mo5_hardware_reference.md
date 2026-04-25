# MO5 Hardware Reference — Documentation Technique Complète

> Version fusionnée : V2.1 (ChatGPT/corrections Claude) + doc PIA (Claude/MAME)  
> Cible : IA + développeurs C bas niveau (CMOC)  
> Sources : Manuel Technique MO5 (Michel Oury / Cedic-Nathan), MAME `thomson.cpp` (Antoine Miné, BSD-3), sessions de débogage

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

# 2. MÉMOIRE VIDÉO

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

# 3. MODÈLE COULEUR

## Réalité hardware

- 4 bits par pixel : RVB + intensité
- Conversion via circuit analogique (PROM)
- Le mapping n'est **pas linéaire RGB standard**

## Palette approximative (fiabilité MOYENNE — à vérifier sur vrai hardware)

| Code | Couleur |
|------|---------|
| 0 | Noir |
| 1 | Rouge |
| 2 | Vert |
| 3 | Jaune |
| 4 | Bleu |
| 5 | Magenta |
| 6 | Cyan |
| 7 | Blanc |
| 8 | Gris |
| 9 | Rose |
| 10 | Vert clair |
| 11 | Jaune clair |
| 12 | Bleu clair |
| 13 | Rose clair |
| 14 | Cyan clair |
| 15 | Orange |

> ⚠️ Ces valeurs sont indicatives. Ne pas supposer un mapping RGB standard.  
> Utiliser des tables prédéfinies testées sur vrai hardware ou émulateur.

---

# 4. PIA — MODÈLE COMPLET

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

Géré par `mo5_sys_porta_out` dans MAME.

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0 | Sortie | **Sélection banque vidéo** : 0 = FORME, 1 = COULEUR |
| 1 | Sortie | Signal cassette (écriture données) |
| 2 | Entrée | Signal cassette (lecture données) |
| 3 | Entrée | Bouton crayon optique |
| 4 | — | Moteur cassette (géré via CA2, voir CRA) |
| 5-7 | — | Non documentés |

> ⚠️ Le **bit 0** est critique — modifier les autres bits sans masque peut
> corrompre la sélection de banque vidéo.

### $A7C1 — PORTB (ORB / DDRB) ★ BUZZER

**C'est ici que se trouve le buzzer système.**

Source MAME confirmée :
```cpp
m_pia_sys->writepb_handler().set("buzzer", FUNC(dac_bit_interface::data_w));
```

| Bit | Direction | Fonction |
|-----|-----------|----------|
| 0 | Sortie | **Son buzzer 1-bit** (bip clavier et sons ROM) |
| 1-7 | Entrée | Lignes matrice clavier |

**Couper le bip clavier (cmoc, sans volatile) :**
```c
*((unsigned char*)0xA7C1) = 0x00;
```

> ⚠️ Cela coupe **tout** le son buzzer système.  
> Pour ne masquer que le bit son en préservant les autres :
> ```c
> unsigned char val = *((unsigned char*)0xA7C1);
> val &= ~0x01;
> *((unsigned char*)0xA7C1) = val;
> ```
> Condition préalable : CRB bit 2 = 1 (accès ORB actif).  
> C'est normalement déjà le cas après l'init du moniteur.

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

CB2 → câblé en `nop` sur MO5 (non utilisé).

IRQ B → **interruption principale du 6809**.

> ⚠️ Différence TO7/MO5 : c'est **IRQB** (pas IRQA) qui génère l'IRQ principale.  
> Commentaire MAME : `// WARNING: differs from TO7 !`

---

## PIA Extension Jeux/Musique — $A7CC à $A7CF

Extension **optionnelle** sur MO5 de base.  
**Intégrée de série** sur : MO5E, MO6, MO5NR.  
Source : Manuel Technique p. 48-51.

```text
$A7CC  PORTA  (et DDRA)   → manettes de jeu
$A7CD  PORTB  (et DDRB)   → DAC son 6 bits
$A7CE  CRA
$A7CF  CRB
```

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

# 5. MINI SDK C (CMOC)

```c
/* Accès mémoire de base */
#define PEEK(addr)      (*(unsigned char*)(addr))
#define POKE(addr, val) (*(unsigned char*)(addr) = (val))

/* Vidéo */
#define SCREEN          ((unsigned char*)0x0000)
#define PIA_PORTA       (*(unsigned char*)0xA7C0)

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

```c
void mo5_mute_beep(void)
{
    unsigned char val = *((unsigned char*)0xA7C1);
    val &= ~0x01;
    *((unsigned char*)0xA7C1) = val;
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

# 6. PATTERNS MO5-SPÉCIFIQUES

## 6.1 Minimiser les bank switches

```text
❌ FORM → COLOR → FORM → COLOR  (un pixel à la fois)
✔  FORM (ligne entière) → COLOR (ligne entière)
```

## 6.2 Dirty rectangles (quasi-obligatoire)

```text
Ne mettre à jour que les zones modifiées.
Performance clé sur MO5 vu l'arbitrage mémoire.
```

## 6.3 Sprite masqué

```c
dst = (dst & mask) | sprite;   /* propre mais plus lent */
dst ^= sprite;                  /* rapide mais effacement destructif */
```

## 6.4 Double buffering

```text
⚠️ RAM limitée (32 Ko user) → pas de vrai double buffer possible.
Solution : buffer partiel + dirty rectangles.
```

## 6.5 Accès groupés

```text
Regrouper les écritures mémoire en rafales courtes.
Éviter les accès dispersés pendant l'affichage actif.
Privilégier les gros transferts pendant le VBL.
```

---

# 7. ANTI-PATTERNS

```text
❌ Supposer un mapping couleur RGB linéaire
❌ Écrire en vidéo sans sélectionner la bonne banque
❌ Utiliser malloc
❌ Utiliser des divisions à l'exécution
❌ Supposer E=1/E=0 comme sur TO7
❌ Accès mémoire dispersés pendant affichage actif
❌ Framebuffer linéaire (pas assez de RAM)
```

---

# 8. TABLEAU DE FIABILITÉ

| Élément | Fiabilité | Source |
|---------|-----------|--------|
| Mapping pixel vidéo | ÉLEVÉE | Manuel technique |
| PIA système ($A7C0-$A7C3) | ÉLEVÉE | Manuel + MAME + EPI |
| Buzzer sur $A7C1 bit 0 | ÉLEVÉE | MAME `thomson.cpp` confirmé |
| PIA extension ($A7CC-$A7CF) | ÉLEVÉE | Manuel technique p.48-51 |
| Init DAC (séquence DDR) | ÉLEVÉE | Manuel technique p.48 |
| Palette couleurs | MOYENNE | Approximation, à tester |
| Timing précis bus/gate-array | FAIBLE | Manuel incomplet sur ce point |

---

# 9. SOURCES

| Source | Contenu | Fiabilité |
|--------|---------|-----------|
| Manuel Technique MO5 (Cedic-Nathan) | Hardware, PIA, DAC, vidéo | Référence primaire |
| MAME `thomson.cpp` (Antoine Miné) | Câblage exact PIA, buzzer | Très élevée |
| Bulletin EPI n°44 | Adresse PIA système `$A7C0` | Élevée |
| regards.sur.sciences.free.fr | Bit 0 `$A7C0` = banque vidéo | Élevée |
| Sessions débogage Claude | Buzzer `$A7C1`, corrections V2 | Élevée (vérifiée MAME) |
