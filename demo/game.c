#include "asset_cache.h"
#include "level.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};
const size_t TURN_LEN = 45;
const size_t NUM_LEVEL_OPTIONS = 3;

const color_t FOREST_COLOR = {0, 0.251, 0.051};
const vector_t EARTH_GRAVITY = {0, -500};
const double FOREST_MAX_WIND = 125;

const color_t MESA_COLOR = {0.82, 0.42, 0};
const double MESA_MAX_WIND = 400;

const color_t MOON_COLOR = {0.49, 0.49, 0.486};
const vector_t MOON_GRAVITY = {0, -100};
const double MOON_MAX_WIND = 0;

const level_info_t LEVELS[] = {
    (level_info_t){.background_path = "assets/forest.png",
                   .gravity = EARTH_GRAVITY,
                   .max_wind = FOREST_MAX_WIND,
                   .terrain_color = FOREST_COLOR,
                   .turn_len = TURN_LEN,
                   .screen_min = MIN,
                   .screen_max = MAX,
                   .type = FOREST},
    (level_info_t){.background_path = "assets/mesa.png",
                   .gravity = EARTH_GRAVITY,
                   .max_wind = MESA_MAX_WIND,
                   .terrain_color = MESA_COLOR,
                   .turn_len = TURN_LEN,
                   .screen_min = MIN,
                   .screen_max = MAX,
                   .type = MESA},
    (level_info_t){.background_path = "assets/moon.png",
                   .gravity = MOON_GRAVITY,
                   .max_wind = MOON_MAX_WIND,
                   .terrain_color = MOON_COLOR,
                   .turn_len = TURN_LEN,
                   .screen_min = MIN,
                   .screen_max = MAX,
                   .type = MOON}

};

state_t *emscripten_init() {
  sdl_init(MIN, MAX);
  asset_cache_init();
  state_t *state = state_init(LEVELS, 3);

  sdl_on_key((key_handler_t)turn_engine_on_key);
  sdl_on_mouse((mouse_handler_t)state_mouse_handler);
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  state_tick(state, dt);
  return sdl_is_done(state);
}

void emscripten_free(state_t *state) {
  state_free(state);
  asset_cache_destroy();
}