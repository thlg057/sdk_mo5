# MO5 Retro-Library

A lightweight C library for development on Thomson MO5, designed to be
used with the **CMOC** compiler.

## 📁 Project Structure

-   `src/` : Contains source files (`.c`).
-   `include/` : Contains public headers (`.h`).
-   `obj/` : Temporary directory for object files (generated at
    compilation).
-   `lib/` : Contains the final static library `libsdk_mo5.a`.

## 🛠️ Installation & Compilation

### Prerequisites

-   **CMOC** (version 0.1.93 or higher recommended).
-   **lwtools** (for the `lwar` archiver).

### Compile the library

To generate the object files and the `.a` archive, simply run:

    make

### Export the SDK

To generate a folder sdk_mo5 ready to be distributed (containing only
the .h and the .a), use:

    make export_sdk

## 📦 Library Contents

The library is divided into three main modules:

1- Basic Input/Output (mo5_defs): - Direct access to the system monitor
via swi interrupts. - Functions: mo5_getchar(), mo5_putchar(),
mo5_newline().

2- Mini-stdio (mo5_stdio): - High-level functions for screen and
keyboard. - Functions: fgets() (with Backspace support), puts(),
clrscr().

2- Character types (mo5_ctype): - Character validation compliant with C
standards. - Functions: islower(), isupper(), isprint(), ispunct().

## 📥 SDK Installation (Headers and Library)

To use this library in another project without having to recompile the
source files (.c), you can automate retrieval of the precompiled SDK
(files .h and .a) via an install target in your Makefile.

### Makefile Configuration

Add these variables and this rule to your Makefile to manage automatic
installation from GitHub Releases:

    # SDK version to retrieve
    SDK_VERSION = v0.1.0
    # Release URL (replace with your real GitHub link)
    SDK_URL     = https://github.com/thgl057/sdk_mo5/releases/download/$(SDK_VERSION)/sdk_mo5.zip
    # Local folder where to install the SDK
    SDK_DIR     = lib/sdk_mo5

    install:
        @echo "Installing MO5 SDK $(SDK_VERSION)..."
        @mkdir -p "$(SDK_DIR)"
        @curl -L "$(SDK_URL)" -o sdk_temp.zip
        @unzip -o sdk_temp.zip -d "$(SDK_DIR)"
        @rm sdk_temp.zip
        @echo "✓ SDK installed successfully in $(SDK_DIR)"

## 🚀 Usage

Once the SDK is retrieved, you can compile your program by linking the
library:

    cmoc --thommo main.c -I./sdk_mo5/include ./sdk_mo5/lib/libsdk_mo5.a -o mon_programme.k7

## 📄 License

This project is intended for retro-computing on Thomson MO5.
