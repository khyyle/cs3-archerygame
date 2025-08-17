#include "hud.h"
#include "asset.h"
#include "camera.h"
#include "crate.h"
#include "turn_engine.h"
#include <SDL2/SDL.h>
#include <stdio.h>

const char *HUD_FONT_PATH = "assets/Arial.ttf";
const size_t HUD_FONT_PX = 20;
const color_t HUD_COLOR = {1, 1, 1};
const size_t HUD_OFS = 4;
const size_t NUM_PLAYERS = 2;
const size_t HP_HUD_OFFSET = 48;
const size_t EQUIP_OFFSET_Y = 32;
const char *VARIANT_STR[] = {"standard", "heavy", "multishot"};
const char *WIND_STRENGTH[] = {"STRONG++", "STRONG", "MEDIUM", "LOW", "NONE"};
const size_t WIDTH = 100;

const double WIND_THRESH_STRONG = 200.0;
const double WIND_THRESH_HIGH = 100.0;
const double WIND_THRESH_MEDIUM = 50.0;

void make_char_hp_label(turn_engine_t *eng, player_id_t id) {
  char msg[64];
  size_t hp = eng_get_player_hp(eng, id);
  vector_t pos = eng_get_player_pos(eng, id);
  pos.y += HP_HUD_OFFSET;
  vector_t scr = camera_world_to_screen(eng->cam, pos);
  sprintf(msg, "P%u HP: %zu", id + 1, hp);
  SDL_Rect rect = {(scr.x - WIDTH / 2), (scr.y - HUD_FONT_PX / 2), WIDTH,
                   HUD_FONT_PX};
  asset_make_text(HUD_FONT_PATH, rect, msg, HUD_COLOR);
}

void hud_draw(turn_engine_t *eng) {
  list_t *assets = asset_get_asset_list();
  size_t i = 0;
  while (i < list_size(assets)) {
    asset_t *a = list_get(assets, i);
    if (asset_get_type(a) == ASSET_TEXT) {
      list_remove(assets, i);
      continue;
    }
    i++;
  }

  SDL_Rect rect;
  char time[64];
  sprintf(time, "TIME %.1f", eng->timer);
  rect = (SDL_Rect){HUD_OFS, HUD_OFS, WIDTH, HUD_FONT_PX};
  asset_make_text(HUD_FONT_PATH, rect, time, HUD_COLOR);

  char wind_mag_msg[64];
  vector_t wind = eng->level->wind;
  double wind_mag = vec_get_length(wind);
  const char *msg;
  if (wind_mag >= WIND_THRESH_STRONG) {
    msg = WIND_STRENGTH[0];
  } else if (wind_mag >= WIND_THRESH_HIGH) {
    msg = WIND_STRENGTH[1];
  } else if (wind_mag >= WIND_THRESH_MEDIUM) {
    msg = WIND_STRENGTH[2];
  } else if (wind_mag > 0) {
    msg = WIND_STRENGTH[3];
  } else if (wind_mag == 0) {
    msg = WIND_STRENGTH[4];
  }
  sprintf(wind_mag_msg, "WIND STRENGTH: %s", msg);
  rect = (SDL_Rect){HUD_OFS, 2 * HUD_OFS + HUD_FONT_PX, WIDTH, HUD_FONT_PX};
  asset_make_text(HUD_FONT_PATH, rect, wind_mag_msg, HUD_COLOR);

  char wind_dir_msg[64];
  double wind_dir = atan2(wind.y, wind.x) * 180 / M_PI;
  if (wind_dir < 0) {
    wind_dir = -wind_dir + 180;
  }
  sprintf(wind_dir_msg, "WIND DIRECTION: %.1f deg", wind_dir);
  rect = (SDL_Rect){HUD_OFS, 3 * HUD_OFS + 2 * HUD_FONT_PX, WIDTH, HUD_FONT_PX};
  asset_make_text(HUD_FONT_PATH, rect, wind_dir_msg, HUD_COLOR);

  char equipped_msg[128];
  sprintf(equipped_msg, "EQUIPPED: %s",
          VARIANT_STR[eng_get_equipped_index(eng)]);
  rect = (SDL_Rect){HUD_OFS, (eng->cam->screen_max.y - EQUIP_OFFSET_Y), WIDTH,
                    HUD_FONT_PX};
  asset_make_text(HUD_FONT_PATH, rect, equipped_msg, HUD_COLOR);

  crate_render_hp(eng->level->scene, eng->cam, HUD_FONT_PATH, HUD_COLOR);
  make_char_hp_label(eng, PLAYER_ONE);
  make_char_hp_label(eng, PLAYER_TWO);
}