#ifndef MO5_DEFS_H
#define MO5_DEFS_H

typedef enum {
    FALSE = 0,
    TRUE  = 1
} Boolean;

/* Limites et tailles */
#define MO5_MAX_NAME_LENGTH     30
#define MO5_BUFFER_SIZE         32

/* Codes clavier / écran */
#define MO5_CLEAR_SCREEN        12
#define MO5_BACKSPACE_CHAR      8
#define MO5_ENTER_CHAR          13
#define MO5_SPACE_CHAR          32
#define MO5_LINE_FEED           10

/**
 * @brief Reads a single character from the input.
 * @return The character read.
 * @note Blocking call: waits until a character is available.
 */
char mo5_getchar(void); 

/**
 * @brief Writes a single character to the output.
 * @param c Character to write.
 */
void mo5_putchar(char c); 

/**
 * @brief Outputs a newline sequence to the display.
 * @note Advances the cursor to the next line according to the MO5 display rules.
 */
void mo5_newline(void);

#endif
