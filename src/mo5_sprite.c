#include "mo5_sprite.h"
#include <cmoc.h>

// Définition du tableau des offsets de lignes (initialisé dans mo5_init_graphic_mode)
unsigned int row_offsets[SCREEN_HEIGHT];

void mo5_init_graphic_mode(unsigned char color) {
    unsigned int i;
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        row_offsets[i] = i * SCREEN_WIDTH_BYTES;
    }
    
    *PRC = 0x00;
    *VIDEO_REG |= 0x01;
    *PRC &= ~0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++) {
        VRAM[i] = color;
    }
    
    *PRC |= 0x01;
    for (i = 0; i < SCREEN_SIZE_BYTES; i++) {
        VRAM[i] = 0x00; 
    }    
}

void mo5_draw_sprite(int tx, int ty, unsigned char *form_data, unsigned char *color_data, int width_bytes, int height) {
    unsigned int offset;
    int i, j;
    
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        
        // 1. Écrire les COULEURS (banque attributs)
        *PRC &= ~0x01;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = color_data[i * width_bytes + j];
        }
        
        // 2. Écrire les FORMES (banque bitmap)
        *PRC |= 0x01;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = form_data[i * width_bytes + j];
        }
    }
}

void mo5_clear_sprite(int tx, int ty, int width_bytes, int height) {
    unsigned int offset;
    int i, j;
    
    *PRC |= 0x01;  // Banque FORME seulement
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = 0x00;
        }
    }
}