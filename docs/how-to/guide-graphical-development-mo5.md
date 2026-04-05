# Developing a Graphical Game or Application for the Thomson MO5

## Overview

This guide covers everything needed to develop a graphical game or application for the Thomson MO5 in C, using the **sdk_mo5** library. It covers the hardware constraints, the SDK setup, sprite creation from PNG assets, the rendering APIs, and text display in graphics mode.

- SDK repository: https://github.com/thlg057/sdk_mo5.git
- Project template: https://github.com/thlg057/mo5_template

---

## Part 1 — Setting up the SDK

The build system is pre-configured. Install the SDK with:

```bash
make install
```

This clones and builds the SDK, then exports the following into `tools/`:

```
tools/
├── include/      ← Header files to use in your project
├── lib/          ← Compiled static library (libsdk_mo5.a)
├── scripts/      ← Utility scripts (png2mo5.py, fd2sd.py, ...)
└── docs/         ← Documentation for all headers
```

The Makefile is pre-configured to pass the correct `-I` and linker flags automatically. You only need to declare your source files:

```makefile
PROJ_SRC := src/main.c src/game.c    # All your .c files
PROJ_HDR := include/game.h           # All your .h files — REQUIRED for make to detect changes
```

> **Important:** always list your `.h` files in `PROJ_HDR`. If a header changes but is not listed, `make` will not detect the change and will not recompile — leading to silent bugs.

---

## Part 2 — MO5 Graphics Hardware

Understanding the hardware constraints is essential before writing any graphical code.

### Screen dimensions

- Resolution: **320 × 200 pixels**
- 16 available colors (fixed palette, see below)

### The two-bank video memory model

The MO5 splits video memory into two banks, each 8000 bytes (40 bytes × 200 rows):

| Bank   | Role | Content |
|--------|------|---------|
| **Form bank** | Bitmap | 1 bit per pixel — `1` = foreground color, `0` = background color |
| **Color bank** | Attributes | 1 byte per 8-pixel group — format `FFFFBBBB` (foreground in bits 4–7, background in bits 0–3) |

**The fundamental constraint:** within any group of 8 horizontal pixels, only **two colors** can coexist — one foreground and one background. This is a hard hardware limit.

### Coordinate system

Horizontal positions are expressed in **bytes**, not pixels. One byte = 8 pixels.

- X range: 0–39 (bytes)
- Y range: 0–199 (pixel rows)

### The 16-color palette

```c
#define C_BLACK        0
#define C_RED          1
#define C_GREEN        2
#define C_YELLOW       3
#define C_BLUE         4
#define C_MAGENTA      5
#define C_CYAN         6
#define C_WHITE        7
#define C_GRAY         8
#define C_LIGHT_RED    9
#define C_LIGHT_GREEN  10
#define C_LIGHT_YELLOW 11
#define C_LIGHT_BLUE   12
#define C_PURPLE       13
#define C_LIGHT_CYAN   14
#define C_ORANGE       15
```

To combine foreground and background into a single color byte, use the `COLOR` macro:

```c
COLOR(bg, fg)   // e.g. COLOR(C_BLACK, C_WHITE)
```

---

## Part 3 — Program Structure

A MO5 program must run inside an infinite loop. When execution exits the loop, the program terminates (similar to Arduino's `loop()` function). There is no operating system to return to in graphics mode.

### Minimal graphical program

```c
#include <mo5_video.h>

int main(void) {
    /* Initialize graphics mode: black background, white foreground */
    mo5_video_init(COLOR(C_BLACK, C_WHITE));

    while (1) {
        mo5_wait_vbl();   /* Sync to 50 Hz vertical blanking */

        /* Update game logic here */
        /* Draw sprites here     */
    }

    return 0;
}
```

### Initialization

`mo5_video_init(color)` must be called once at startup. It:
- Pre-computes the `row_offsets[200]` lookup table (avoids per-scanline multiplication)
- Enables bitmap graphics mode
- Fills the color bank with the given color
- Clears the form bank (all zeros)

### The main loop and VBL synchronization

`mo5_wait_vbl()` blocks until the next vertical blanking interval (50 Hz PAL). Calling it at the start of each loop iteration:
- Guarantees approximately **20 000 CPU cycles** of budget per frame
- Prevents screen tearing by ensuring drawing happens outside the active scan period

```c
while (1) {
    mo5_wait_vbl();       /* Wait for next frame — ~20 000 cycles available */
    update_logic();
    draw();
}
```

---

## Part 4 — Creating Sprites from PNG Images

### PNG constraints

Your source image must respect the MO5 hardware constraints:

- Use only the 16 MO5 palette colors (see above)
- Background color must be **black** (for transparent rendering with `mo5_sprite_bg`)
- Within any horizontal group of 8 pixels, no more than 2 colors
- Width must be a **multiple of 8 pixels**

### Converting a PNG to a C sprite header

```bash
make convert IMG=assets/hero.png
```

This generates `include/assets/hero.h` containing:

```c
#define SPRITE_HERO_WIDTH_BYTES  4
#define SPRITE_HERO_HEIGHT       16

unsigned char sprite_hero_form[64]  = { /* bitmap data */ };
unsigned char sprite_hero_color[64] = { /* color attribute data */ };

#define SPRITE_HERO_INIT \
    { sprite_hero_form, sprite_hero_color, \
      SPRITE_HERO_WIDTH_BYTES, SPRITE_HERO_HEIGHT }
```

### Script options

| Option | Description |
|--------|-------------|
| `--name <path>` | Output path and sprite name (without extension) |
| `--bg-color <0-15>` | Background color index (default: 0 = black) |
| `--quiet` | Suppress verbose output |

### How the conversion works

The script processes the image 8 pixels at a time, left to right, top to bottom:

1. For each 8-pixel block, it identifies the dominant foreground color (closest MO5 palette match)
2. It writes a **form byte**: each bit is `1` if the pixel is closer to the foreground color, `0` otherwise
3. It writes a **color byte** in `FFFFBBBB` format: foreground index in bits 4–7, background in bits 0–3
4. With `--transparent`, the background bits are forced to `0x0` — required for the `mo5_sprite_bg` rendering mode

If the image width is not a multiple of 8, it is truncated to the nearest lower multiple.

---

## Part 5 — Rendering Sprites

The SDK provides two sprite engines with different trade-offs.

### mo5_sprite_bg — Transparent rendering (recommended)

`mo5_sprite_bg.h` renders sprites over a colored background, preserving the background color behind each sprite. This is the standard mode for games with a visible scenery.

Draw operation per byte:
```
color bank: (VRAM & 0x0F) | (sprite_color & 0xF0)
            → writes only the foreground color nibble
            → preserves the background color of the scenery
form bank:  |= form_data
```

**Asset requirement:** the color data must have background bits set to `0x0` — use `--transparent` when converting with `png2mo5.py`.

```c
#include "mo5_sprite_bg.h"
#include "assets/hero.h"

MO5_Actor hero;
hero.sprite.form_data    = sprite_hero_form;
hero.sprite.color_data   = sprite_hero_color;
hero.sprite.width_bytes  = SPRITE_HERO_WIDTH_BYTES;
hero.sprite.height       = SPRITE_HERO_HEIGHT;
hero.pos.x = 10;
hero.pos.y = 50;

/* In the main loop: */
mo5_actor_move_bg(&hero, new_x, new_y);   /* Clear old position, draw at new position */
```

Available functions:

```c
/* Low-level (raw byte coordinates) */
mo5_draw_sprite_bg(tx, ty, form_data, color_data, width_bytes, height);
mo5_clear_sprite_bg(tx, ty, width_bytes, height);
mo5_move_sprite_bg(old_tx, old_ty, new_tx, new_ty, form_data, color_data, width_bytes, height);

/* Actor API (game level) */
mo5_actor_draw_bg(const MO5_Actor *actor);
mo5_actor_clear_bg(const MO5_Actor *actor);
mo5_actor_move_bg(MO5_Actor *actor, new_x, new_y);  /* No-op if position unchanged */
```

### mo5_sprite_form — Form-only rendering (optimized)

`mo5_sprite_form.h` only writes to the form bank. The color bank is set once at initialization and never touched. This is significantly faster than `mo5_sprite_bg` but restricts all sprite pixels to the same foreground color, set globally by `mo5_video_init`.

Use this mode for UI elements, bullets, or any sprite that shares a single uniform color with the background.

```c
/* Low-level */
mo5_draw_sprite_form(tx, ty, form_data, width_bytes, height);
mo5_clear_sprite_form(tx, ty, width_bytes, height);
mo5_move_sprite_form(old_tx, old_ty, new_tx, new_ty, form_data, width_bytes, height);

/* Actor API */
mo5_actor_draw_form(const MO5_Actor *actor);
mo5_actor_clear_form(const MO5_Actor *actor);
mo5_actor_move_form(MO5_Actor *actor, new_x, new_y);
```

### Choosing the right mode

| Criteria | mo5_sprite_bg | mo5_sprite_form |
|----------|--------------|-----------------|
| Background preserved | ✓ Yes | ✗ No |
| Multi-color sprites | ✓ Yes | ✗ No (form color only) |
| Speed | Slower | **Faster** |
| Color bank written | Yes (fg nibble only) | **Never** |
| Best for | Characters, enemies, scenery | Bullets, HUD, single-color elements |

---

## Part 6 — Screen Utilities

```c
/* Fill entire screen with a color */
mo5_clear_screen(COLOR(C_BLACK, C_WHITE));

/* Fill a rectangle */
mo5_fill_rect(tx, ty, width_bytes, height, COLOR(C_BLUE, C_WHITE));
```

`mo5_fill_rect` writes to both banks (color and form), making it suitable for initializing background areas before drawing sprites with `mo5_sprite_bg`.

---

## Part 7 — Displaying Text in Graphics Mode

The SDK provides two bitmap fonts for rendering text directly on the graphics screen, without switching to text mode. Both preserve the background scenery — only the form bank is written.

### Available fonts

| Header | Character size | Lines on screen | Best for |
|--------|---------------|-----------------|----------|
| `mo5_font8.h` | 8 × 8 pixels | 25 lines | Titles, large HUD labels |
| `mo5_font6.h` | 8 × 6 pixels | 33 lines | Scores, dense UI, status bars |

Both fonts use byte-aligned horizontal coordinates (1 unit = 8 pixels), consistent with the rest of the SDK.

### mo5_font8 — 8×8 arcade font

```c
#include <mo5_font8.h>

/* Draw a string */
mo5_font8_puts(2, 16, "SCORE 000100", C_YELLOW);

/* Erase a text zone (n characters) */
mo5_font8_clear(2, 16, 12);
```

### mo5_font6 — 8×6 compact font

```c
#include <mo5_font6.h>

/* Draw a string */
mo5_font6_puts(2, 12, "SCORE 000100", C_YELLOW);

/* Erase a text zone (n characters) */
mo5_font6_clear(2, 12, 12);
```

### Function signatures

Both fonts share the same API:

```c
/* Draw a null-terminated string */
void mo5_font8_puts(unsigned char tx, unsigned char ty,
                    const char *s, unsigned char fg_color);

/* Erase len characters (clear the form bank pixels) */
void mo5_font8_clear(unsigned char tx, unsigned char ty, unsigned char len);
```

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `tx` | Horizontal position in bytes (0–39, 1 unit = 8 pixels) |
| `ty` | Vertical position in pixels (0–199) |
| `s` | Null-terminated string |
| `fg_color` | Foreground color (0–15, use `C_xxx` constants) |
| `len` | Number of characters to erase |

### Behaviour notes

- Automatic line wrap when `tx >= 40`
- Background is fully preserved — only form pixels are written, the color bank is not touched
- `mo5_font8_clear` / `mo5_font6_clear` erases exactly `len` character cells (useful for updating a score without redrawing the whole screen)
- Choose `mo5_font6` when vertical space is limited or when you need more than 25 lines of text

---

## References

- sdk_mo5 repository: https://github.com/thlg057/sdk_mo5
- Project template: https://github.com/thlg057/mo5_template
- Related guides: *Guide d'optimisation C pour 6809*, *SDK Graphique : Gestion des Sprites*, *Synchronisation Verticale (VBL)*
