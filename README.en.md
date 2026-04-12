# MO5 SDK

A lightweight C library for creating programs for the Thomson MO5,
designed to be used with the **CMOC** compiler.

## 📁 Project Structure

- `src/` : Source files (`.c`)
- `include/` : Public headers (`.h`)
- `obj/` : Temporary directory for object files (generated at compilation)
- `lib/` : Contains the final static library `libsdk_mo5.a`
- `docs/` : Markdown documentation for all modules (used by the MO5 RAG server)
- `scripts/` : Python utility scripts

## 🛠️ Prerequisites & Compilation

### Prerequisites

- **CMOC** (version 0.1.93 or higher recommended)
- **lwtools** (for the `lwar` archiver)

### Compile the library

```bash
make
```

### Export the SDK

To generate a `sdk_mo5` folder ready to be distributed (headers + `.a` only):

```bash
make export_sdk
```

> This command is called automatically by `make install` in the project template.

---

## 📦 Library Contents

### Low-level I/O — `mo5_defs.h`

Direct access to the system monitor via SWI interrupts.

| Function | Description |
|---|---|
| `mo5_getchar()` | Blocking read of a keyboard character |
| `mo5_putchar(c)` | Write a character to the screen |
| `mo5_newline()` | Move to the next line |
| `mo5_wait_key(key)` | Wait until a specific key is pressed |
| `mo5_wait_for_key()` | Wait for a key press and return it (uppercase) |

---

### High-level I/O — `mo5_stdio.h`

String and screen I/O functions.

| Function | Description |
|---|---|
| `fgets(buf, max)` | Read a string with echo and Backspace support |
| `fputs(s)` | Write a string |
| `puts(s)` | Write a string followed by a newline |
| `clrscr()` | Clear the screen and reset the cursor |
| `getchar` | Macro mapping to `mo5_getchar()` |

---

### Character classification — `mo5_ctype.h`

Character validation compliant with C standards.

| Function | Description |
|---|---|
| `islower(c)` | Lowercase letter |
| `isupper(c)` | Uppercase letter |
| `isprint(c)` | Printable character (32-126) |
| `ispunct(c)` | Punctuation character |

---

### Utilities — `mo5_utils.h`

| Function | Description |
|---|---|
| `mo5_clamp(val, min, max)` | Clamps a value to a range (`unsigned char`) |

---

### Video & hardware — `mo5_video.h`

Video hardware abstraction: VRAM registers, 16-color palette, screen dimensions, VBL sync.

| Function | Description |
|---|---|
| `mo5_video_init(color)` | Initializes graphics mode (pre-computes `row_offsets`, fills color bank) |
| `mo5_wait_vbl()` | Waits for the next vertical blanking interval (50 Hz PAL) |
| `mo5_clear_screen(color)` | Clears the entire screen |
| `mo5_fill_rect(tx, ty, w, h, color)` | Fills a rectangle |

Color constants: `C_BLACK`, `C_RED`, `C_GREEN`, `C_YELLOW`, `C_BLUE`, `C_MAGENTA`, `C_CYAN`, `C_WHITE`, `C_GRAY`, `C_LIGHT_RED`, `C_LIGHT_GREEN`, `C_LIGHT_YELLOW`, `C_LIGHT_BLUE`, `C_PURPLE`, `C_LIGHT_CYAN`, `C_ORANGE`.

Color macro: `COLOR(bg, fg)`, encodes background and foreground in a single `FFFFBBBB` byte.

---

### Opaque sprites — `mo5_sprite.h`

Direct rendering on a black background (both VRAM banks are overwritten). Use when the background is uniform.

| Function | Description |
|---|---|
| `mo5_draw_sprite(...)` | Draw a sprite (raw coordinates) |
| `mo5_clear_sprite(...)` | Clear a sprite |
| `mo5_move_sprite(...)` | Move a sprite (optimized: only redraws the changed area) |
| `mo5_actor_draw(actor)` | Draw an actor |
| `mo5_actor_clear(actor)` | Clear an actor |
| `mo5_actor_move(actor, x, y)` | Move an actor |

---

### Transparent sprites — `mo5_sprite_bg.h` ⭐ Recommended

Transparent rendering over a colored background. The scenery background color is preserved.

Asset convention: sprite color data background bits must be `0x0`.

| Function | Description |
|---|---|
| `mo5_draw_sprite_bg(...)` | Draw a transparent sprite |
| `mo5_clear_sprite_bg(...)` | Clear a sprite (form bank only) |
| `mo5_move_sprite_bg(...)` | Move a transparent sprite |
| `mo5_actor_draw_bg(actor)` | Draw an actor |
| `mo5_actor_clear_bg(actor)` | Clear an actor |
| `mo5_actor_move_bg(actor, x, y)` | Move an actor, preserving the background |

---

### Form-only sprites — `mo5_sprite_form.h`

Fastest rendering mode: only the form bank is written. The color bank is initialized once and never touched again. Use when all sprite pixels share the same foreground color.

| Function | Description |
|---|---|
| `mo5_draw_sprite_form(...)` | Draw (form only) |
| `mo5_clear_sprite_form(...)` | Clear |
| `mo5_move_sprite_form(...)` | Move |
| `mo5_actor_draw_form(actor)` | Draw an actor |
| `mo5_actor_clear_form(actor)` | Clear an actor |
| `mo5_actor_move_form(actor, x, y)` | Move an actor |

---

### Dirty Rectangle — `mo5_actor_dr.h`

Advanced sprite engine: saves the VRAM area before drawing and restores it at the start of each frame. Enables arbitrarily colored backgrounds, correct sprite layering, and pixel-perfect transparency.

Required per-frame sequence:
1. `mo5_actor_dr_restore()` on all sprites (reverse draw order)
2. Game logic + `mo5_actor_dr_move()`
3. `mo5_actor_dr_save_draw()` on all sprites (normal draw order)

| Function | Description |
|---|---|
| `mo5_actor_dr_init(actor, sprite, x, y)` | Initialize the actor and perform the first draw |
| `mo5_actor_dr_restore(actor)` | Restore the saved VRAM |
| `mo5_actor_dr_save_draw(actor)` | Save VRAM then draw |
| `mo5_actor_dr_move(actor, x, y)` | Update position |

---

### Text rendering in graphics mode — `mo5_font6.h` / `mo5_font8.h`

Arcade fonts for displaying text without overwriting the background scenery.

| Module | Size | Lines on screen |
|---|---|---|
| `mo5_font6.h` | 8×6 pixels | 33 lines |
| `mo5_font8.h` | 8×8 pixels | 25 lines |

| Function | Description |
|---|---|
| `mo5_font6_puts(tx, ty, s, fg)` | Display a string in 6px font |
| `mo5_font6_clear(tx, ty, len)` | Clear a text area |
| `mo5_font8_puts(tx, ty, s, fg)` | Display a string in 8px font |
| `mo5_font8_clear(tx, ty, len)` | Clear a text area |

Coordinates: `tx` in bytes (0-39), `ty` in pixels (0-199).

---

### Shared types — `mo5_sprite_types.h`

Included automatically by the sprite modules. Do not include directly.

Defines `MO5_Position`, `MO5_Sprite`, `MO5_Actor` and `mo5_actor_clamp()`.

---

## 🐍 Python Scripts

### `png2mo5.py`

Converts a PNG image to C structures for the MO5.
Checks and encodes the hardware constraint (2 colors per 8-pixel block).

```bash
make convert IMG=./assets/player.png
# generates include/assets/player.h
```

### `makefd.py`

Generates a bootable `.fd` floppy disk image for the Thomson MO5 from
a `.BIN` binary.

The Thomson bootloader (`BOOTMO.BIN`) is **embedded directly in the
script**, no need to clone or compile the external
[BootFloppyDisk](https://github.com/OlivierP-To8/BootFloppyDisk) project.
The toolchain is fully self-contained with no third-party repo dependency.

```bash
python3 scripts/makefd.py output.fd program.BIN
```

> This script is called automatically by `make` in the project template.

### `fd2sd.py`

Converts a `.fd` floppy disk image to `.sd` format compatible with SDDrive.

```bash
python3 scripts/fd2sd.py input.fd output.sd
```

---

## 🤖 AI Assistant Integration (MCP)

The `docs/` folder contains Markdown documentation for all modules.
They are indexed in the MO5 RAG server, your AI agent can search them in natural language.

👉 [retrocomputing-ai.cloud](https://retrocomputing-ai.cloud)  
👉 [npmjs.com/@thlg057/mo5-rag-mcp](https://www.npmjs.com/package/@thlg057/mo5-rag-mcp)

---

## 🔗 The full ecosystem

- 📦 **Project template** : https://github.com/thlg057/mo5_template
- 🎮 **Space Invaders tutorial** : https://github.com/thlg057/mo5-space-invaders-tutorial
- 🤖 **MCP server** : https://www.npmjs.com/package/@thlg057/mo5-rag-mcp
- 🌐 **Knowledge base** : https://retrocomputing-ai.cloud
- 📖 **Blog** : https://thlg057.github.io/mo5-blog/

---

## 📄 License

MIT, Copyright (c) 2026 Thierry Le Got