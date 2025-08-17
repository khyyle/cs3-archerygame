#include "crate.h"
#include "asset.h"
#include "camera.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double CRATE_SIZE = 64.0;
const double MARGIN = CRATE_SIZE * 2.0;
const double CRATE_MASS = INFINITY;
const int32_t CRATE_HP = 30;
const char *CRATE_IMG = "assets/crate.png";
const size_t CRATE_NUM_POINTS = 4;
const color_t CRATE_COLOR = {1, 1, 1};
const double LABEL_OFFSET = 12.0;
const char *CRATE_TAG = "crate";
const char *FONT_PATH = "assets/Arial.ttf";
const size_t CRATE_HUD_PX = 20;
const size_t TEXT_WIDTH = 50;

bool crate_is(body_t *b) {
  if (!b) {
    return false;
  }
  color_t color = body_get_color(b);
  return CRATE_COLOR.red == color.red && CRATE_COLOR.green == color.green &&
         CRATE_COLOR.blue == color.blue;
}

body_t *crate_spawn(level_t *level) {
  scene_t *scene = level->scene;
  size_t n = scene_bodies(scene);
  for (size_t i = 0; i < n; i++) {
    if (crate_is(scene_get_body(scene, i))) {
      return NULL;
    }
  }

  double min_x = level->info.screen_min.x + MARGIN;
  double max_x = level->info.screen_max.x - MARGIN;
  double x = rand_double(min_x, max_x);
  double y_0 = level_ground_height(level, x);

  vector_t center = {x, y_0 + CRATE_SIZE * 0.5};
  list_t *verts = list_init(CRATE_NUM_POINTS, free);
  for (size_t i = 0; i < CRATE_NUM_POINTS; i++) {
    double x_side = (i == 1 || i == 2) ? 1 : -1;
    double y_side = (i >= 2) ? 1 : -1;
    vector_t *vertex = malloc(sizeof(vector_t));
    *vertex = (vector_t){center.x + x_side * CRATE_SIZE * .5,
                         center.y + y_side * CRATE_SIZE * .5};
    list_add(verts, vertex);
  }

  crate_info_t *info = malloc(sizeof(crate_info_t));
  *info = (crate_info_t){.tag = CRATE_TAG, .hp = CRATE_HP};

  body_t *crate =
      body_init_with_info(verts, CRATE_MASS, CRATE_COLOR, info, free);
  asset_make_image_with_body(CRATE_IMG, crate);
  scene_add_body(scene, crate);
  return crate;
}

void crate_render_hp(scene_t *scene, camera_t *cam, const char *font_path,
                     const color_t color) {
  size_t n = scene_bodies(scene);
  for (size_t i = 0; i < n; ++i) {
    body_t *b = scene_get_body(scene, i);
    if (!crate_is(b)) {
      continue;
    }

    int32_t hp = ((crate_info_t *)body_get_info(b))->hp;
    char txt[64];
    sprintf(txt, "HP: %d", hp);

    vector_t world = body_get_centroid(b);
    world.y += CRATE_SIZE * 0.5 + LABEL_OFFSET;
    vector_t screen = camera_world_to_screen(cam, world);

    SDL_Rect rect = {(screen.x - TEXT_WIDTH * 0.5),
                     (screen.y - CRATE_HUD_PX * 0.5), TEXT_WIDTH, CRATE_HUD_PX};
    asset_make_text(font_path, rect, txt, color);
    break;
  }
}
