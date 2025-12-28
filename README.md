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

## 📄 License

This project is intended for retro-computing on Thomson MO5.
