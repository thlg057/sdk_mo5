# MO5 Retro-Library

Une bibliothèque C légère pour le développement sur Thomson MO5, conçue pour être utilisée avec le compilateur **CMOC**.

## 📁 Structure du Projet

- `src/` : Contient les fichiers sources (`.c`).
- `include/` : Contient les headers publics (`.h`).
- `obj/` : Dossier temporaire pour les fichiers objets (généré à la compilation).
- `lib/` : Contient la bibliothèque statique finale `libmo5.a`.

## 🛠️ Installation & Compilation

### Prérequis
- **CMOC** (version 0.1.93 ou supérieure recommandée).
- **lwtools** (pour l'archiveur `lwar`).

### Compiler la bibliothèque
Pour générer les fichiers objets et l'archive `.a`, lancez simplement :
```bash
make
```

### Exporter le SDK
Pour générer un dossier sdk_mo5 prêt à être distribué (contenant uniquement les .h et le .a), utilisez :

```bash
make export_sdk
```

## 📦 Contenu de la bibliothèque
La bibliothèque est divisée en trois modules principaux :

1- Entrées/Sorties de base (mo5_defs) :
- Accès direct au moniteur système via interruptions swi.
- Fonctions : mo5_getchar(), mo5_putchar(), mo5_newline().

2- Mini-stdio (mo5_stdio) :
- Fonctions de haut niveau pour l'écran et le clavier.
- Fonctions : fgets() (avec support du Backspace), puts(), clrscr().

2- Types de caractères (mo5_ctype) :
- Validation de caractères conforme aux standards C.
- Fonctions : islower(), isupper(), isprint(), ispunct().

## 📥 Installation du SDK (Headers et Bibliothèque)

Pour utiliser cette bibliothèque dans un autre projet sans avoir à recompiler les fichiers sources (`.c`), vous pouvez automatiser la récupération du SDK pré-compilé (fichiers `.h` et `.a`) via une cible `install` dans votre Makefile.

### Configuration du Makefile

Ajoutez ces variables et cette règle à votre Makefile pour gérer l'installation automatique depuis les Releases GitHub :

```makefile
# Version du SDK à récupérer
SDK_VERSION = v0.1.0
# URL de la release (remplacez par votre lien GitHub réel)
SDK_URL     = https://github.com/thgl057/sdk_mo5/releases/download/$(SDK_VERSION)/sdk_mo5.zip
# Dossier local où installer le SDK
SDK_DIR     = lib/sdk_mo5

install:
	@echo "Installation du SDK MO5 $(SDK_VERSION)..."
	@mkdir -p "$(SDK_DIR)"
	@curl -L "$(SDK_URL)" -o sdk_temp.zip
	@unzip -o sdk_temp.zip -d "$(SDK_DIR)"
	@rm sdk_temp.zip
	@echo "✓ SDK installé avec succès dans $(SDK_DIR)"
```
## 🚀 Utilisation 
Une fois le SDK récupéré, vous pouvez compiler votre programme en liant la bibliothèque :

``` bash
cmoc --thommo main.c -I./sdk_mo5/include ./sdk_mo5/lib/libmo5.a -o mon_programme.k7
```

## 📄 Licence
Ce projet est destiné au retro-computing sur Thomson MO5.