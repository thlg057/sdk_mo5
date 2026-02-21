#ifndef MO5_VIDEO_H
#define MO5_VIDEO_H

#include <cmoc.h>

// ============================================================================
// REGISTRES MATÉRIELS
// ============================================================================

#define PRC       ((unsigned char *)0xA7C0)  // Sélection banque VRAM (bit0: 0=couleur, 1=forme)
#define VIDEO_REG ((unsigned char *)0xA7E7)  // Registre mode vidéo
#define VRAM      ((unsigned char *)0x0000)  // Base de la mémoire vidéo

// Registre de status VBL (bit7 = 1 pendant le retour trame)
#define VBL_REG   ((unsigned char *)0xA7E7)
#define VBL_BIT   0x80

// ============================================================================
// PALETTE MO5 (16 couleurs)
// ============================================================================

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

// Construit un octet couleur MO5 au format FFFFBBBB
// bg = couleur de fond (bits 0-3), fg = couleur de forme (bits 4-7)
#define COLOR(bg, fg) (unsigned char)(((bg) & 0x0F) | (((fg) & 0x0F) << 4))

// ============================================================================
// DIMENSIONS ÉCRAN
// ============================================================================

#define SCREEN_WIDTH_BYTES  40    // 320 pixels / 8 pixels par octet
#define SCREEN_HEIGHT       200   // Lignes pixels
#define SCREEN_SIZE_BYTES   (SCREEN_WIDTH_BYTES * SCREEN_HEIGHT)  // 8000 octets

// ============================================================================
// TABLE DES OFFSETS DE LIGNES
// Précalculée dans mo5_video_init() — évite une multiplication à chaque accès ligne.
// Usage : offset = row_offsets[y] + x
// ============================================================================

extern unsigned int row_offsets[SCREEN_HEIGHT];

// ============================================================================
// FONCTIONS
// ============================================================================

/**
 * Initialise le mode graphique MO5.
 * - Précalcule la table row_offsets
 * - Active le mode bitmap
 * - Remplit la banque couleur avec 'color'
 * - Efface la banque forme (tout à 0)
 *
 * Doit être appelée UNE SEULE FOIS au démarrage, avant tout affichage.
 *
 * @param color  Couleur de fond initiale — utiliser COLOR(bg, fg)
 */
void mo5_video_init(unsigned char color);

/**
 * Attend le prochain retour de trame vertical (VBL, 50 Hz en PAL).
 *
 * À appeler en début de boucle principale pour synchroniser le jeu
 * sur la fréquence d'affichage. Garantit 20 000 cycles CPU par frame
 * et évite le tearing.
 *
 * Usage typique :
 *   while (1) {
 *       mo5_wait_vbl();
 *       update_logic();
 *       draw();
 *   }
 */
void mo5_wait_vbl(void);

/**
 * Remplit tout l'écran (banques couleur et forme) avec la couleur donnée.
 *
 * @param color  Octet couleur — utiliser COLOR(bg, fg)
 */
void mo5_clear_screen(unsigned char color);

/**
 * Remplit un rectangle de l'écran avec une couleur uniforme.
 * Écrit dans les deux banques (couleur et forme).
 *
 * @param tx      Position X en octets (1 octet = 8 pixels)
 * @param ty      Position Y en lignes pixels
 * @param w       Largeur en octets
 * @param h       Hauteur en lignes pixels
 * @param color   Octet couleur — utiliser COLOR(bg, fg)
 */
void mo5_fill_rect(int tx, int ty, int w, int h, unsigned char color);

#endif // MO5_VIDEO_H
