#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stdlib.h>

#include "camera.h"
#include "level.h"
#include "math.h"
#include "turn_engine.h"

typedef enum {
  OVERLAY_NONE,
  OVERLAY_START_SCREEN,
  OVERLAY_CONTROLS,
  OVERLAY_ARENA_SELECT
} overlay_t;

/**
 * Stores the demo state
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state {
  screen_t screen;
  level_t *level;
  camera_t *cam;
  turn_engine_t *eng;
  overlay_t overlay;
  size_t level_num;
  level_info_t level_info[];
} state_t;

/**
 * Returns an initialized state variable that handles
 * the engine, level, camera, and any thing that
 * needs to be rendered on the screen
 * @param levels level info of all possible levels
 * @param num_levels number of levels in "levels"
 *
 * @return a fully initialized state_t
 */
state_t *state_init(const level_info_t levels[], size_t num_levels);

/**
 * free state and all of its constituent variables
 * @param state the state struct
 */
void state_free(state_t *state);

/**
 * Run all necessary updates. Renders necessary objects,
 * checks game end, move things as needed, adjust health, etc.abort
 *
 * @param state current state object
 * @param dt timestep for update
 */
void state_tick(state_t *state, double dt);

/**
 * The one fits all mouse handler for all things that
 * can happen in the game. To be passed to sdl_on_mouse.
 *
 * @param state current state object
 * @param type mouse event type
 * @param mouse_x mouse x pos
 * @param mouse_y mouse y pos
 */
void state_mouse_handler(state_t *state, mouse_event_type_t type,
                         double mouse_x, double mouse_y);

/**
 * Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 */
state_t *emscripten_init();

/**
 * Called on each tick of the program
 * Updates the state variables and display as necessary, depending on the time
 * that has passed.
 *
 * @param state pointer to a state object with info about demo
 * @return a boolean representing whether the game/demo is over
 */
bool emscripten_main(state_t *state);

/**
 * Frees anything allocated in the demo
 * Should free everything in state as well as state itself.
 *
 * @param state pointer to a state object with info about demo
 */
void emscripten_free(state_t *state);

#endif // GAME_STATE_H
