#include "state.h"
#include "arrow.h"
#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "crate.h"
#include "hud.h"
#include "input.h"
#include "shoot.h"
#include "turn_engine.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const size_t FOREST_LEVEL_IDX = 0;
const size_t MESA_LEVEL_IDX = 1;
const size_t MOON_LEVEL_IDX = 2;

const color_t P1_COLOR = {.blue = 1, .green = 0, .red = 0};
const color_t P2_COLOR = {.blue = 0, .green = 0, .red = 1};
const double PLAYER_HALF_PX = 32;
const double ZOOMED = 1.4;
const vector_t PLAYER_HITBOX[4] = {{-PLAYER_HALF_PX, -PLAYER_HALF_PX},
                                   {PLAYER_HALF_PX, -PLAYER_HALF_PX},
                                   {PLAYER_HALF_PX, PLAYER_HALF_PX},
                                   {-PLAYER_HALF_PX, PLAYER_HALF_PX}};
const size_t PLAYER_HITBOX_PTS = 4;
const double PLAYER_MASS = INFINITY;
const double PLAYER_X_POS = 100;
const int32_t PLAYER_HP = 100;

const size_t BUTTON_WIDTH = 200;
const size_t BUTTON_HEIGHT = 150;
const size_t PLAYER_HEALTH_WIDTH = 100;
const size_t WIN_LABEL_W = 100;
const size_t WIN_LABEL_H = 50;
const size_t GAME_OVER_Y_SHIFT = 80;

const color_t START_SCREEN_COLOR = {1.0, 1.0, 1.0};
const char *SCREEN_FONT = "assets/Arial.ttf";
const char *START_BKGD_IMG = "assets/start_bg.png";
const char *SEL_BKGD_IMG = "assets/sel_bg.png";

const char *PLAY_BTN_IMG = "assets/play_btn.png";
const char *CTRL_BTN_IMG = "assets/ctrl_btn.png";
const char *BACK_BTN_IMG = "assets/back_btn.png";

const char *FOREST_BTN_IMG = "assets/forest_button.png";
const char *MESA_BTN_IMG = "assets/mesa_button.png";
const char *MOON_BTN_IMG = "assets/moon_button.png";

const SDL_Rect CTRL1 = {80, 160, 340, 35};
const SDL_Rect CTRL2 = {80, 200, 210, 35};
const SDL_Rect CTRL3 = {80, 240, 760, 35};
const SDL_Rect CTRL4 = {80, 280, 860, 35};
const SDL_Rect CTRL5 = {80, 320, 370, 35};
const SDL_Rect CTRL6 = {80, 360, 860, 35};

const char *CTRL_MSG1 = "Left-click & drag to aim";
const char *CTRL_MSG2 = "Release to shoot";
const char *CTRL_MSG3 =
    "Press the down key to zoom out and see the entire arena";
const char *CTRL_MSG4 =
    "Select between standard, heavy, and multishot arrows using keys 1, 2, 3";
const char *CTRL_MSG5 = "Crates can spawn--shoot them to gain 30 HP";
const char *CTRL_MSG6 = "Last person standing wins!";

const SDL_Rect FULL = {0, 0, 1000, 500};
const SDL_Rect PLAY_BTN = {350, 225, 300, 100};
const SDL_Rect CTRL_BTN = {375, 375, 250, 100};
const SDL_Rect BACK_BTN = {0, 0, 150, 100};

const SDL_Rect FOREST_BTN = {375, 150, 250, 150};
const SDL_Rect MESA_BTN = {50, 150, 250, 150};
const SDL_Rect MOON_BTN = {700, 150, 250, 150};

const char *P1_PATH = "assets/blue_archer.png";
const char *P2_PATH = "assets/red_archer.png";

void push_winner_label(player_id_t winner, vector_t screen_min,
                       vector_t screen_max) {
  const char *msg = (winner == PLAYER_ONE) ? "P1 WINS :D" : "P2 WINS :(";
  color_t color = (winner == PLAYER_ONE) ? P1_COLOR : P2_COLOR;

  size_t mid_x = ((screen_min.x + screen_max.x) * 0.5);
  size_t mid_y = ((screen_min.y + screen_max.y) * 0.5);

  SDL_Rect box = {mid_x - WIN_LABEL_W / 2,
                  (mid_y - WIN_LABEL_H / 2) - GAME_OVER_Y_SHIFT, WIN_LABEL_W,
                  WIN_LABEL_H};
  asset_make_text(SCREEN_FONT, box, msg, color);
}

SDL_Rect get_reset_button_rect(void) {
  vector_t center = get_window_center();
  size_t win_width = center.x * 2;
  size_t win_height = center.y * 2;
  size_t button_width = BUTTON_WIDTH;
  size_t button_height = BUTTON_HEIGHT;
  SDL_Rect rect = {(win_width - button_width) / 2,
                   (win_height - button_height) / 2 + GAME_OVER_Y_SHIFT,
                   button_width, button_height};
  return rect;
}

void render_play_screen(state_t *state, list_t *assets, double dt) {
  sdl_clear();
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *a = list_get(assets, i);
    if (asset_get_type(a) == ASSET_IMAGE && asset_get_body(a) == NULL) {
      asset_render(a);
    }
  }

  camera_apply(state->cam);
  sdl_render_scene(state->level->scene);
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *a = list_get(assets, i);
    if (asset_get_type(a) == ASSET_IMAGE && asset_get_body(a) != NULL) {
      asset_render(a);
    }
  }

  camera_reset(state->cam);
  if (state->eng->active == PLAYER_ONE) {
    shoot_render_preview(state->cam);
  }
  hud_draw(state->eng);
  for (size_t i = 0; i < list_size(assets); i++) {
    asset_t *a = list_get(assets, i);
    if (asset_get_type(a) == ASSET_TEXT) {
      asset_render(a);
    }
  }
  arrow_render_particles();
  sdl_show();
}

void push_start_screen_assets(state_t *state) {
  asset_reset_asset_list();
  asset_make_image(START_BKGD_IMG, FULL);
  asset_make_image(PLAY_BTN_IMG, PLAY_BTN);
  asset_make_image(CTRL_BTN_IMG, CTRL_BTN);
}

void push_controls_assets() {
  asset_reset_asset_list();
  asset_make_image(SEL_BKGD_IMG, FULL);
  asset_make_text(SCREEN_FONT, CTRL1, CTRL_MSG1, START_SCREEN_COLOR);
  asset_make_text(SCREEN_FONT, CTRL2, CTRL_MSG2, START_SCREEN_COLOR);
  asset_make_text(SCREEN_FONT, CTRL3, CTRL_MSG3, START_SCREEN_COLOR);
  asset_make_text(SCREEN_FONT, CTRL4, CTRL_MSG4, START_SCREEN_COLOR);
  asset_make_text(SCREEN_FONT, CTRL5, CTRL_MSG5, START_SCREEN_COLOR);
  asset_make_text(SCREEN_FONT, CTRL6, CTRL_MSG6, START_SCREEN_COLOR);
  asset_make_image(BACK_BTN_IMG, BACK_BTN);
}

void push_arena_select_assets() {
  asset_reset_asset_list();
  asset_make_image(SEL_BKGD_IMG, FULL);
  asset_make_image(FOREST_BTN_IMG, FOREST_BTN);
  asset_make_image(MESA_BTN_IMG, MESA_BTN);
  asset_make_image(MOON_BTN_IMG, MOON_BTN);
  asset_make_image(BACK_BTN_IMG, BACK_BTN);
}

void push_gameover_assets() {
  asset_reset_asset_list();
  vector_t center = get_window_center();
  size_t win_width = center.x * 2;
  size_t win_height = center.y * 2;
  SDL_Rect screen = {0, 0, win_width, win_height};
  asset_make_image("assets/game_over.png", screen);
  SDL_Rect reset_box = get_reset_button_rect();
  asset_make_image("assets/main_menu.png", reset_box);
}

body_t *make_player(vector_t pos, double mass, color_t color,
                    const char *img_path) {
  list_t *vertices = list_init(PLAYER_HITBOX_PTS, free);
  for (size_t i = 0; i < PLAYER_HITBOX_PTS; i++) {
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t){pos.x + PLAYER_HITBOX[i].x, pos.y + PLAYER_HITBOX[i].y};
    list_add(vertices, v);
  }
  int32_t *hp = malloc(sizeof(int32_t));
  *hp = PLAYER_HP;
  body_t *body = body_init_with_info(vertices, mass, color, hp, free);
  asset_make_image_with_body(img_path, body);
  return body;
}

void push_play_assets(state_t *state, size_t level_idx) {
  asset_reset_asset_list();
  const level_info_t info = state->level_info[level_idx];
  state->level = level_init(info);
  state->cam = camera_init(info.screen_min, info.screen_max);

  double p1_x = info.screen_min.x + PLAYER_X_POS;
  double p2_x = info.screen_max.x - PLAYER_X_POS;
  double p1_y = level_ground_height(state->level, p1_x);
  double p2_y = level_ground_height(state->level, p2_x);
  vector_t p1_pos = {p1_x, p1_y + PLAYER_HALF_PX};
  vector_t p2_pos = {p2_x, p2_y + PLAYER_HALF_PX};
  body_t *p2 = make_player(p2_pos, PLAYER_MASS, P2_COLOR, P2_PATH);
  body_t *p1 = make_player(p1_pos, PLAYER_MASS, P1_COLOR, P1_PATH);
  size_t p1_idx = scene_bodies(state->level->scene);
  size_t p2_idx = p1_idx + 1;

  scene_add_body(state->level->scene, p1);
  scene_add_body(state->level->scene, p2);

  state->eng =
      turn_engine_init(state->level, state->cam, info.turn_len, p1_idx, p2_idx);
  state->screen = SCREEN_PLAY;
  state->overlay = OVERLAY_NONE;
}

state_t *state_init(const level_info_t levels[], size_t num_levels) {
  srand(time(0));
  state_t *state =
      calloc(1, sizeof(state_t) + num_levels * sizeof(level_info_t));
  assert(state);

  for (size_t i = 0; i < num_levels; i++) {
    state->level_info[i] = levels[i];
  }
  state->screen = SCREEN_START;
  state->overlay = OVERLAY_NONE;
  push_start_screen_assets(state);

  return state;
}

void state_free(state_t *state) {
  if (!state) {
    return;
  }
  if (state->eng) {
    turn_engine_destroy(state->eng);
  }
  asset_reset_asset_list();
  free(state);
}

void state_mouse_handler(state_t *state, mouse_event_type_t type,
                         double mouse_x, double mouse_y) {
  switch (state->screen) {
  case SCREEN_START:
    if (type != MOUSE_RELEASED) {
      return;
    }

    if (state->overlay == OVERLAY_CONTROLS) {
      if (sdl_in_rect(mouse_x, mouse_y, BACK_BTN)) {
        state->overlay = OVERLAY_NONE;
        push_start_screen_assets(state);
      }
      return;
    }

    if (state->overlay == OVERLAY_ARENA_SELECT) {
      if (sdl_in_rect(mouse_x, mouse_y, BACK_BTN)) {
        state->overlay = OVERLAY_NONE;
        push_start_screen_assets(state);
        return;
      }
      if (sdl_in_rect(mouse_x, mouse_y, FOREST_BTN)) {
        push_play_assets(state, FOREST);
      } else if (sdl_in_rect(mouse_x, mouse_y, MESA_BTN)) {
        push_play_assets(state, MESA);
      } else if (sdl_in_rect(mouse_x, mouse_y, MOON_BTN)) {
        push_play_assets(state, MOON);
      }
      return;
    }

    if (sdl_in_rect(mouse_x, mouse_y, PLAY_BTN)) {
      state->overlay = OVERLAY_ARENA_SELECT;
      push_arena_select_assets();
      return;
    }
    if (sdl_in_rect(mouse_x, mouse_y, CTRL_BTN)) {
      state->overlay = OVERLAY_CONTROLS;
      push_controls_assets();
      return;
    }
    break;
  case SCREEN_PLAY:
    if (!state->level || state->cam->zoom != ZOOMED) {
      return;
    }
    switch (type) {
      player_id_t active;
      body_t *shooter;
    case MOUSE_PRESSED:
      shoot_begin(state->eng, mouse_x, mouse_y);
      break;
    case MOUSE_DRAGGED:
      shoot_drag(state->eng, mouse_x, mouse_y);
      break;
    case MOUSE_RELEASED:
      active = state->eng->active;
      shooter =
          scene_get_body(state->level->scene, state->eng->p_body_idx[active]);
      shoot_end(state->eng, shooter, mouse_x, mouse_y);
      break;
    }
  case SCREEN_GAME_OVER:
    if (type == MOUSE_RELEASED) {
      SDL_Rect rect = get_reset_button_rect();
      if (sdl_in_rect(mouse_x, mouse_y, rect)) {
        asset_reset_asset_list();
        push_start_screen_assets(state);
        state->screen = SCREEN_START;
      }
    }
    break;
  }
}

void state_tick(state_t *state, double dt) {
  switch (state->screen) {
  case SCREEN_START:
    sdl_clear();
    for (size_t i = 0; i < list_size(asset_get_asset_list()); i++) {
      asset_render(list_get(asset_get_asset_list(), i));
    }
    sdl_show();
    break;
  case SCREEN_GAME_OVER:
    sdl_clear();
    for (size_t i = 0; i < list_size(asset_get_asset_list()); i++) {
      asset_render(list_get(asset_get_asset_list(), i));
    }
    sdl_show();
    break;
  case SCREEN_PLAY:
    if (state->level) {
      list_t *assets = asset_get_asset_list();
      level_tick(state->level, dt);
      turn_engine_update(state->eng, dt);
      arrow_update_particles(state->level, dt, state->eng->equipped_arrow);

      render_play_screen(state, assets, dt);
    }

    body_t *p1 =
        scene_get_body(state->level->scene, state->eng->p_body_idx[PLAYER_ONE]);
    body_t *p2 =
        scene_get_body(state->level->scene, state->eng->p_body_idx[PLAYER_TWO]);

    int32_t hp1 = p1 ? *(int32_t *)body_get_info(p1) : 0;
    int32_t hp2 = p2 ? *(int32_t *)body_get_info(p2) : 0;

    if (hp1 <= 0 || hp2 <= 0) {
      player_id_t winner = (hp2 <= 0 && hp1 > 0) ? PLAYER_ONE : PLAYER_TWO;
      push_gameover_assets();
      turn_engine_destroy(state->eng);
      vector_t min = state->level_info[FOREST_LEVEL_IDX].screen_min;
      vector_t max = state->level_info[FOREST_LEVEL_IDX].screen_max;
      push_winner_label(winner, min, max);
      state->screen = SCREEN_GAME_OVER;
    }
    break;
  }
}