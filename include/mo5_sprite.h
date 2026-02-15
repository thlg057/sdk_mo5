#ifndef MO5_SPRITE_H
#define MO5_SPRITE_H

#define PRC       ((unsigned char *)0xA7C0)
#define VIDEO_REG ((unsigned char *)0xA7E7)
#define VRAM      ((unsigned char *)0x0000)

// Palette MO5
#define C_BLACK 0
#define C_RED 1
#define C_GREEN 2
#define C_YELLOW 3
#define C_BLUE 4
#define C_MAGENTA 5
#define C_CYAN 6
#define C_WHITE 7
#define C_GRAY 8
#define C_LIGHT_RED 9
#define C_LIGHT_GREEN 10
#define C_LIGHT_YELLOW 11
#define C_LIGHT_BLUE 12
#define C_PURPLE 13
#define C_LIGHT_CYAN 14
#define C_ORANGE 15

// Macro couleur : FORME en haut, FOND en bas
#define COLOR(bg, fg) (unsigned char)((bg & 0x0F) | ((fg & 0x0F) << 4))

// Dimensions en 320 x 200
#define SCREEN_WIDTH_BYTES 40
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE_BYTES (SCREEN_WIDTH_BYTES * SCREEN_HEIGHT)

// Déclaration externe du tableau des offsets de lignes
extern unsigned int row_offsets[SCREEN_HEIGHT];

void mo5_init_graphic_mode(unsigned char color);
void mo5_draw_sprite(int tx, int ty, unsigned char *form_data, unsigned char *color_data, int width_bytes, int height);
void mo5_clear_sprite(int tx, int ty, int width_bytes, int height);

#endif