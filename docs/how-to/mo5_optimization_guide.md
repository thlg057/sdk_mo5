# MO5 C Optimization Guide — Thomson 6809 / CMOC

> Best practices for writing efficient C code targeting the Thomson MO5 (Motorola 6809, ~1 MHz, CMOC compiler). Every cycle counts.

---

## 1. Prefer `unsigned char` over `int`

The 6809 is a native 8-bit CPU. All `int` operations (16-bit) require multiple instructions.

| Type | Width | Add | Compare | Load/Store |
|---|---|---|---|---|
| `unsigned char` | 8-bit | 1 instruction | 1 instruction | 1 instruction |
| `int` | 16-bit | 2–4 instructions | 2–4 instructions | 2 instructions |

```c
// ❌ int loop counter — 16-bit increment every iteration
for (int i = 0; i < 32; i++) { ... }

// ✅ unsigned char — 8-bit increment
for (unsigned char i = 0; i < 32; i++) { ... }
```

```c
// ❌ function returning int for a boolean
int isprint(char c) { return (c >= 32 && c <= 126) ? TRUE : FALSE; }

// ✅ unsigned char — 0 or 1 fits in 8 bits
unsigned char isprint(char c) { return (c >= 32 && c <= 126) ? 1 : 0; }
```

**Exception:** use `unsigned int` when a value exceeds 255 (e.g. VRAM offsets: max = 199 × 40 = 7960).

---

## 2. Avoid Multiplication and Division

The 6809 has no native 16-bit multiply instruction (only 8×8→16 via `MUL`). Division does not exist in hardware at all. Both operations in C compile to expensive software routines.

### Replace with pointer increment

```c
// ❌ multiply per pixel — recomputes i * width_bytes on every inner iteration
for (unsigned char i = 0; i < height; i++) {
    for (unsigned char j = 0; j < width_bytes; j++) {
        VRAM[offset + j] = color_data[i * width_bytes + j];  // ← multiplication!
    }
}

// ✅ pointer increment — no multiplication at all
unsigned char *src = color_data;
for (unsigned char i = 0; i < height; i++) {
    unsigned int offset = row_offsets[y + i] + x;
    for (unsigned char j = 0; j < width_bytes; j++) {
        VRAM[offset + j] = *src++;  // ← pure increment
    }
}
```

### Replace with a pre-computed lookup table

```c
// ❌ multiply on every scanline access
unsigned int offset = y * SCREEN_WIDTH_BYTES + x;

// ✅ pre-computed table in mo5_video_init()
unsigned int row_offsets[SCREEN_HEIGHT];   // computed once: row_offsets[y] = y * 40
unsigned int offset = row_offsets[y] + x;  // single addition at runtime
```

### Replace multiply by power of 2 with shift

```c
// ❌ multiply
unsigned int size = height * 4;

// ✅ left shift (×4 = <<2)
unsigned int size = height << 2;
```

---

## 3. Use Pointer Increment Instead of Array Indexing

Array indexing `a[i]` computes a base address + offset every time. A running pointer just increments.

```c
// ❌ index recomputed each iteration
void fputs(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        putchar(str[i]);
        i++;
    }
}

// ✅ pointer walks forward directly
void fputs(const char *str) {
    while (*str)
        putchar(*str++);
}
```

```c
// ❌ two separate indexed accesses per pixel
VRAM[offset + j] = (VRAM[offset + j] & 0x0F) | color_data[i * width_bytes + j];

// ✅ one pre-incremented pointer, no index arithmetic
unsigned char *src = color_data;
VRAM[offset + j] = (VRAM[offset + j] & 0x0F) | (*src++ & 0xF0);
```

---

## 4. Count Down in Loops

Comparing against zero is free on the 6809 (the Z flag is set automatically after decrement). Comparing against a non-zero constant requires a `CMP` instruction.

```c
// ❌ compare against non-zero constant each iteration
for (unsigned char i = 0; i < height; i++) { ... }

// ✅ decrement toward zero — no explicit compare needed
for (unsigned char i = height; i != 0; i--) { ... }
```

**Note:** only applicable when loop body does not depend on `i` as an index. If you need `i` as a row index, keep the ascending form with `row_offsets`.

---

## 5. Minimize VRAM Bank Switches

Switching VRAM banks (writing to `*PRC`) is a memory-mapped I/O operation. Each switch costs cycles. Batch all writes to the same bank together.

```c
// ❌ bank switch inside the inner loop — one switch per pixel
for (unsigned char i = 0; i < height; i++) {
    for (unsigned char j = 0; j < width_bytes; j++) {
        *PRC &= ~0x01;          // bank switch
        VRAM[offset + j] = color;
        *PRC |= 0x01;           // bank switch
        VRAM[offset + j] = form;
    }
}

// ✅ one switch per pass — all color pixels then all form pixels
*PRC &= ~0x01;
for (unsigned char i = 0; i < height; i++) {
    unsigned int offset = row_offsets[y + i] + x;
    for (unsigned char j = 0; j < width_bytes; j++)
        VRAM[offset + j] = *color_src++;
}

*PRC |= 0x01;
for (unsigned char i = 0; i < height; i++) {
    unsigned int offset = row_offsets[y + i] + x;
    for (unsigned char j = 0; j < width_bytes; j++)
        VRAM[offset + j] = *form_src++;
}
```

**Exception:** when conditional logic requires checking both banks together (e.g. transparency test), a per-pixel switch may be unavoidable — but keep it explicit and document it.

---

## 6. Use Bitwise Operations Instead of Arithmetic

Bit operations compile to single 6809 instructions (`ANDA`, `ORA`, `EORA`, `LSRA`...).

```c
// ❌ arithmetic
unsigned char fg = (color >> 4) * 16;
unsigned char bg = color % 16;

// ✅ bitwise
unsigned char fg = color & 0xF0;
unsigned char bg = color & 0x0F;
```

```c
// ❌ multiply by 2
value = value * 2;

// ✅ left shift
value = value << 1;

// ❌ divide by 2
value = value / 2;

// ✅ right shift
value = value >> 1;
```

---

## 7. Avoid Signed Types Unless Necessary

Signed arithmetic requires sign-extension and more complex comparisons on the 6809. Use `unsigned char` by default; reserve `signed char` only when a value must go negative (e.g. a delta).

```c
// ❌ signed by default
int dx = new_x - old_x;   // 16-bit signed subtraction

// ✅ compute delta as signed char — sufficient for ±127
signed char dx = (signed char)(new_x - old_x);
unsigned char adx = dx < 0 ? -dx : dx;   // absolute value in unsigned char
```

---

## 8. Pass Small Values as `unsigned char`, Not `int`

Function parameters passed as `int` are 16-bit even if the value fits in 8 bits. The caller and callee both pay the cost.

```c
// ❌ 16-bit parameters on an 8-bit CPU
void mo5_draw_sprite(int tx, int ty, int width_bytes, int height);

// ✅ 8-bit parameters — fits in a single register
void mo5_draw_sprite(unsigned char tx,         unsigned char ty,
                     unsigned char width_bytes, unsigned char height);
```

---

## 9. Guard Against `unsigned char` Underflow

There are no negative `unsigned char` values. Subtracting past zero wraps around to 255. Always test before subtracting.

```c
// ❌ underflow: 0 - 4 = 252 (wraps around)
pos.y -= SPEED;

// ✅ test before subtracting
if (pos.y >= SPEED)
    pos.y -= SPEED;
else
    pos.y = 0;
```

The same applies to upper bounds — test before adding if overflow is possible:

```c
// ❌ overflow: 253 + 4 = 1 (wraps around)
pos.y += SPEED;

// ✅
if (pos.y + SPEED <= max_y)
    pos.y += SPEED;
else
    pos.y = max_y;
```

---

## 10. Declare Loop-Invariant Values Outside the Loop

The CMOC compiler may not hoist invariant values automatically. Compute them once.

```c
// ❌ max_x recomputed every iteration
while (1) {
    unsigned char max_x = SCREEN_WIDTH_BYTES - actor->sprite->width_bytes;
    ...
}

// ✅ compute once before the loop
unsigned char max_x = SCREEN_WIDTH_BYTES - actor->sprite->width_bytes;
unsigned char max_y = SCREEN_HEIGHT      - actor->sprite->height;
while (1) { ... }
```

---

## 11. Use `static` for Local Helpers

`static` functions are not exported — the compiler/linker can inline or optimize them more aggressively, and they do not pollute the link namespace.

```c
// ✅ internal helper — not visible outside the translation unit
static void dr_save(MO5_Actor_DR *actor) { ... }
static void dr_draw(MO5_Actor_DR *actor) { ... }
```

---

## 12. Skip Transparent Pixels Explicitly

Skipping work entirely is faster than doing cheap work. If a sprite group is transparent (fg = 0), do not write to VRAM at all.

```c
// ❌ always writes to VRAM, even for transparent groups
VRAM[offset + j] = (VRAM[offset + j] & 0x0F) | (*src & 0xF0);

// ✅ skip transparent groups — no VRAM access, no bank switch
unsigned char fg = *color_src & 0xF0;
if (fg) {
    *PRC &= ~0x01;
    VRAM[offset + j] = (VRAM[offset + j] & 0x0F) | fg;
    *PRC |= 0x01;
    VRAM[offset + j] |= *form_src;
}
color_src++;
form_src++;
```

---

## 13. Declare Local Variables at the Very Top of the Function — Never Initialize at Declaration

CMOC targets C89/C90. In C89, **all local variable declarations must appear at the top of a block, before any instruction**. Declaring a variable after an instruction, or initializing it at the point of declaration, can cause random crashes at runtime — no compiler error is raised.

**[OBSERVÉ]** On this project, declaring a variable mid-function or combining declaration and initialization produced unpredictable behavior: sometimes working, sometimes crashing at startup or mid-game.

```c
// ❌ declaration after an instruction — undefined behavior with CMOC
void my_function(void) {
    unsigned char i;
    i = 0;
    unsigned char x;   // ← declared after an instruction: forbidden in C89
    x = i + 1;
}

// ❌ initialization at declaration — may crash with CMOC
void my_function(void) {
    unsigned char i = 0;    // ← may cause random crashes
    unsigned char x = 42;   // ← same risk
}

// ✅ correct: declare all variables first, then initialize separately
void my_function(void) {
    unsigned char i;
    unsigned char x;
    i = 0;      // initialization on a separate line
    x = 42;
}
```

**Rule**: always declare all local variables at the very top of the function body, then initialize them on separate lines below.

---

## 14. Limit Local Variables per Function — Promote to `static` Global if Needed

**[CMOC]** CMOC allocates local variables on the stack via `LEAS -N,S` at function entry. CMOC performs a runtime stack check at function entry: if the remaining stack space is insufficient for the requested frame, **the program stops immediately** — before executing any instruction in the function.

**[OBSERVÉ]** In `game_loop()`, the following variables already filled the available stack:

```c
const unsigned char max_x;
char          key;
unsigned char i, new_x, score, live;
unsigned char enemies_tick, bullets_tick;
```

Adding a single extra variable (`unsigned char result`) blocked the program at startup, before `while(1)` was ever reached.

The symptom is always the same: **the program freezes at the entry point of the function**, before any visible instruction executes.

**Rule**: when a function approaches the stack limit, promote its local variables to **`static` global variables**. `static` variables live in BSS/DATA — their stack cost is zero.

```c
// ❌ too many local variables — stack overflow at function entry
void game_loop(void) {
    unsigned char score;
    unsigned char live;
    unsigned char new_x;
    unsigned char enemies_tick;
    unsigned char bullets_tick;
    unsigned char result;         // ← one too many: crashes before while(1)
    unsigned char score_updated;
    unsigned char live_updated;
    char          key;
    unsigned char i;
    // ...
}

// ✅ promoted to static globals — zero stack cost
static unsigned char gl_score;
static unsigned char gl_live;
static unsigned char gl_new_x;
static unsigned char gl_enemies_tick;
static unsigned char gl_bullets_tick;
static unsigned char gl_result;
static unsigned char gl_score_updated;
static unsigned char gl_live_updated;
static char          gl_key;
static unsigned char gl_i;

void game_loop(void) {
    // frame is nearly empty — stack check passes
    // ...
}
```

Use a prefix (`gl_` for "game loop") to distinguish these promoted variables from true game-wide globals.

**Also applies to called functions**: each function call pushes a return address (2 bytes) plus the callee's own frame. A chain `game_loop → game_check_collisions → game_display_score` (with a `char buf[4]` in `game_display_score`) can overflow the stack even when each function looks reasonable in isolation. Prefer returning flags to the caller rather than calling display functions from inside logic functions.

---

## Summary Table

| Rule | Avoid | Prefer |
|---|---|---|
| Integer width | `int` (16-bit) | `unsigned char` (8-bit) |
| Multiplication | `i * width` | pointer increment / lookup table |
| Division | `n / 2` | `n >> 1` |
| Array indexing | `data[i * w + j]` | `*ptr++` |
| Loop direction | `i < n` (compare) | `i != 0` (count down, Z flag) |
| Bank switches | one per pixel | one per pass |
| Signed types | `int dx` | `signed char dx` when needed |
| Parameters | `int tx, int ty` | `unsigned char tx, unsigned char ty` |
| Underflow | `pos.y -= speed` blindly | guard with `if (pos.y >= speed)` |
| Loop invariants | compute inside loop | hoist before loop |
| Transparency | write 0 to VRAM | skip with `if (fg)` |
| Variable declarations | mid-function or init at declaration | all at top of function, init separately |
| Too many local variables | many locals → stack overflow | promote to `static` global (`gl_` prefix) |

