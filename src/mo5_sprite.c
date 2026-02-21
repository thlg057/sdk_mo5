#include "mo5_sprite.h"   // inclut mo5_video.h transitivement

// ============================================================================
// API BAS NIVEAU
// ============================================================================

/**
 * Dessine un sprite à (tx, ty).
 * Optimisé : 1 seul switch PRC pour toutes les couleurs,
 *            1 seul switch PRC pour toutes les formes.
 * Moins de toggles PRC = moins de cycles perdus.
 */
void mo5_draw_sprite(int tx, int ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height) {
    unsigned int offset;
    int i, j;

    // Passe 1 : écrire TOUTES les couleurs (1 seul switch PRC)
    *PRC &= ~0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = color_data[i * width_bytes + j];
        }
    }

    // Passe 2 : écrire TOUTES les formes (1 seul switch PRC)
    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = form_data[i * width_bytes + j];
        }
    }
}

/**
 * Efface un sprite à (tx, ty).
 * Optimisé : 1 seul switch PRC par passe.
 */
void mo5_clear_sprite(int tx, int ty, int width_bytes, int height) {
    unsigned int offset;
    int i, j;

    // Passe 1 : effacer TOUTES les couleurs (1 seul switch PRC)
    *PRC &= ~0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = 0x00;
        }
    }

    // Passe 2 : effacer TOUTES les formes (1 seul switch PRC)
    *PRC |= 0x01;
    for (i = 0; i < height; i++) {
        offset = row_offsets[ty + i] + tx;
        for (j = 0; j < width_bytes; j++) {
            VRAM[offset + j] = 0x00;
        }
    }
}

/**
 * Déplace un sprite de (old_tx, old_ty) vers (new_tx, new_ty).
 *
 * Principe : ne clearer que la zone hors recouvrement entre l'ancienne
 * et la nouvelle position, puis dessiner le nouveau sprite en entier.
 *
 * Gain typique : ~25% d'écritures VRAM pour un déplacement de 1 octet
 * ou 8 lignes (cas standard sprite 16×16).
 *
 * Fallback automatique sur clear+draw si aucun recouvrement
 * (déplacement supérieur à la taille du sprite).
 */
void mo5_move_sprite(int old_tx, int old_ty,
                     int new_tx, int new_ty,
                     unsigned char *form_data, unsigned char *color_data,
                     int width_bytes, int height) {
    int dx = new_tx - old_tx;
    int dy = new_ty - old_ty;
    unsigned int offset;
    int i, j;

    // Taille de la zone de recouvrement
    int overlap_w = width_bytes - (dx < 0 ? -dx : dx);
    int overlap_h = height      - (dy < 0 ? -dy : dy);

    // Pas de recouvrement : fallback clear + draw classique
    if (overlap_w <= 0 || overlap_h <= 0) {
        mo5_clear_sprite(old_tx, old_ty, width_bytes, height);
        mo5_draw_sprite(new_tx, new_ty, form_data, color_data, width_bytes, height);
        return;
    }

    // -------------------------------------------------------------------------
    // PASSE COULEUR
    // -------------------------------------------------------------------------
    *PRC &= ~0x01;

    // Clear colonne hors overlap (à gauche si on va à droite, à droite sinon)
    if (dx != 0) {
        int clear_col = (dx > 0) ? old_tx : (old_tx + width_bytes + dx);
        int clear_w   = (dx < 0) ? -dx : dx;
        for (i = 0; i < height; i++) {
            offset = row_offsets[old_ty + i] + clear_col;
            for (j = 0; j < clear_w; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    // Clear lignes hors overlap (en haut si on va en bas, en bas sinon)
    if (dy != 0) {
        int clear_row = (dy > 0) ? old_ty : (old_ty + height + dy);
        int clear_h   = (dy < 0) ? -dy : dy;
        for (i = 0; i < clear_h; i++) {
            offset = row_offsets[clear_row + i] + old_tx;
            for (j = 0; j < width_bytes; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    // Dessiner les couleurs du nouveau sprite
    for (i = 0; i < height; i++) {
        offset = row_offsets[new_ty + i] + new_tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = color_data[i * width_bytes + j];
    }

    // -------------------------------------------------------------------------
    // PASSE FORME (même logique, PRC différent)
    // -------------------------------------------------------------------------
    *PRC |= 0x01;

    if (dx != 0) {
        int clear_col = (dx > 0) ? old_tx : (old_tx + width_bytes + dx);
        int clear_w   = (dx < 0) ? -dx : dx;
        for (i = 0; i < height; i++) {
            offset = row_offsets[old_ty + i] + clear_col;
            for (j = 0; j < clear_w; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    if (dy != 0) {
        int clear_row = (dy > 0) ? old_ty : (old_ty + height + dy);
        int clear_h   = (dy < 0) ? -dy : dy;
        for (i = 0; i < clear_h; i++) {
            offset = row_offsets[clear_row + i] + old_tx;
            for (j = 0; j < width_bytes; j++)
                VRAM[offset + j] = 0x00;
        }
    }

    for (i = 0; i < height; i++) {
        offset = row_offsets[new_ty + i] + new_tx;
        for (j = 0; j < width_bytes; j++)
            VRAM[offset + j] = form_data[i * width_bytes + j];
    }
}

// ============================================================================
// API ACTOR
// ============================================================================

void mo5_actor_draw(const MO5_Actor *actor) {
    mo5_draw_sprite(
        actor->pos.x, actor->pos.y,
        actor->sprite->form,  actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

void mo5_actor_clear(const MO5_Actor *actor) {
    mo5_clear_sprite(
        actor->pos.x, actor->pos.y,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

/**
 * Déplace l'acteur vers (new_x, new_y).
 * - No-op si la position n'a pas changé
 * - Met à jour old_pos avant de modifier pos
 * - Utilise mo5_move_sprite (rendu différentiel optimisé)
 */
void mo5_actor_move(MO5_Actor *actor, int new_x, int new_y) {
    if (actor->pos.x == new_x && actor->pos.y == new_y)
        return;

    actor->old_pos = actor->pos;
    actor->pos.x   = new_x;
    actor->pos.y   = new_y;

    mo5_move_sprite(
        actor->old_pos.x, actor->old_pos.y,
        actor->pos.x,     actor->pos.y,
        actor->sprite->form,  actor->sprite->color,
        actor->sprite->width_bytes, actor->sprite->height
    );
}

/**
 * Limite la position de l'acteur aux bords de l'écran.
 * Tient compte de la taille du sprite pour ne pas déborder.
 */
void mo5_actor_clamp(MO5_Actor *actor) {
    int max_x = SCREEN_WIDTH_BYTES - actor->sprite->width_bytes;
    int max_y = SCREEN_HEIGHT      - actor->sprite->height;

    if (actor->pos.x < 0)      actor->pos.x = 0;
    if (actor->pos.x > max_x)  actor->pos.x = max_x;
    if (actor->pos.y < 0)      actor->pos.y = 0;
    if (actor->pos.y > max_y)  actor->pos.y = max_y;
}
