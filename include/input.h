#ifndef INPUT_H
#define INPUT_H
/**
 * This module is here to avoid compilation errors as
 * multiple files (state, turn_engine, etc.) rely on these definitions.
 */

/**
 * All the screens in our game. Typedef'd here
 * because of issues with circular imports
 */
typedef enum { SCREEN_START, SCREEN_PLAY, SCREEN_GAME_OVER } screen_t;

/**
 * The possible types of key events.
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * The possible types of mouse events.
 */
typedef enum {
  MOUSE_PRESSED,
  MOUSE_RELEASED,
  MOUSE_DRAGGED
} mouse_event_type_t;

#endif // INPUT_H