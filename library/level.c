#include "level.h"
#include "arrow.h"
#include "asset.h"
#include "camera.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// arena constants
const size_t NUM_ARENA_VERTICES = 60;
const size_t NUM_WALL_VERITCES = 2;
const size_t ARENA_HEIGHT = 100;
const double IMMOVABLE_MASS = INFINITY;
const char *GROUND_INFO = "ground";

const double FOREST_HILL_HEIGHT = 90.0;
const double FOREST_HILL_HALFWIDTH = 180.0;

const double MESA_TOP_HEIGHT = 80.0;
const double MESA_LOW_HEIGHT = 0;
const double MESA_CLIFF_RATIO = 0.45;
const double MESA_CLIFF_WIDTH = 10;

const double MOON_CRATER_DEPTH = 80.0;
const double MOON_CRATER_HALFWIDTH = 200.0;
const double MOON_LIP_HEIGHT = 75.0;
const double MOON_LIP_OFFSET = 260.0;
const double MOON_LIP_HALFWIDTH = 60.0;

const size_t IMPACT_BURST_COUNT = 20;

double forest_height(double x, double w) {
  return ARENA_HEIGHT +
         FOREST_HILL_HEIGHT *
             exp(-pow((x - 0.5 * w) / FOREST_HILL_HALFWIDTH, 2.0));
}

double mesa_height(double x, double w) {
  if (x < MESA_CLIFF_RATIO * w) {
    return ARENA_HEIGHT + MESA_LOW_HEIGHT;
  } else {
    return ARENA_HEIGHT + MESA_TOP_HEIGHT;
  }
}

double moon_height(double x, double w) {
  double y =
      ARENA_HEIGHT -
      (MOON_CRATER_DEPTH * exp(-pow((x - 0.5 * w) / MOON_CRATER_HALFWIDTH, 2)));
  return y + MOON_LIP_HEIGHT * exp(-pow((fabs(x - 0.5 * w) - MOON_LIP_OFFSET) /
                                            MOON_LIP_HALFWIDTH,
                                        2));
}

body_t *make_ground(level_info_t info) {
  list_t *verts = list_init(NUM_ARENA_VERTICES + NUM_WALL_VERITCES, free);
  double dx = info.screen_max.x / (double)(NUM_ARENA_VERTICES - 1);
  double w = info.screen_max.x - info.screen_min.x;
  double cliff_x = MESA_CLIFF_RATIO * w;
  bool cliff_inserted = false;

  for (size_t i = 0; i < NUM_ARENA_VERTICES; i++) {
    double x = i * dx;
    double y;
    switch (info.type) {
    case FOREST:
      y = forest_height(x, w);
      break;
    case MESA:
      if (!cliff_inserted && x >= cliff_x) {
        vector_t *v_low = malloc(sizeof *v_low);
        *v_low = (vector_t){cliff_x, ARENA_HEIGHT + MESA_LOW_HEIGHT};
        list_add(verts, v_low);
        vector_t *v_high = malloc(sizeof *v_high);
        *v_high = (vector_t){cliff_x, ARENA_HEIGHT + MESA_TOP_HEIGHT};
        list_add(verts, v_high);

        cliff_inserted = true;
      }
      y = mesa_height(x, w);
      break;
    case MOON:
      y = moon_height(x, w);
      break;
    default:
      y = ARENA_HEIGHT;
    }

    if (i == 0 || i == NUM_ARENA_VERTICES - 1) {
      y = ARENA_HEIGHT;
    }
    vector_t *v = malloc(sizeof *v);
    *v = (vector_t){x, y};
    list_add(verts, v);
  }
  vector_t *br = malloc(sizeof(vector_t));
  vector_t *bl = malloc(sizeof(vector_t));
  *br = (vector_t){info.screen_max.x, info.screen_min.y};
  *bl = (vector_t){info.screen_min.x, info.screen_min.y};
  list_add(verts, br);
  list_add(verts, bl);
  return body_init_with_info(verts, IMMOVABLE_MASS, info.terrain_color,
                             (void *)GROUND_INFO, NULL);
}

level_t *level_init(level_info_t info) {
  level_t *level = malloc(sizeof(level_t));

  level->info = info;
  level->scene = scene_init();
  level->gravity = info.gravity;
  level->max_wind = info.max_wind;
  level->wind = VEC_ZERO;
  SDL_Rect bg_rect = {info.screen_min.x, info.screen_min.y,
                      info.screen_max.x - info.screen_min.x,
                      info.screen_max.y - info.screen_min.y};
  asset_make_image(info.background_path, bg_rect);

  body_t *ground = make_ground(info);
  scene_add_body(level->scene, ground);

  return level;
}

void level_tick(level_t *level, double dt) {
  size_t n = scene_bodies(level->scene);
  for (size_t i = 0; i < n; i++) {
    body_t *b = scene_get_body(level->scene, i);
    if (body_is_removed(b)) {
      continue;
    }
    vector_t center = body_get_centroid(b);
    if (body_get_mass(b) != IMMOVABLE_MASS &&
        (center.x < level->info.screen_min.x ||
         center.x > level->info.screen_max.x ||
         center.y < level->info.screen_min.y)) {
      body_remove(b);
    } else if (arrow_check_ground_collision(level, b)) {
      arrow_spawn_impact_burst(level, center, IMPACT_BURST_COUNT);
      body_remove(b);
    } else if (particle_check_ground_collision(level, b)) {
      body_remove(b);
    } else if (body_get_mass(b) != IMMOVABLE_MASS) {
      double m = body_get_mass(b);
      body_add_force(b, vec_multiply(m, level->gravity));
      body_add_force(b, vec_multiply(m, level->wind));
      vector_t v = body_get_velocity(b);
      double angle = atan2(v.y, v.x);
      body_set_rotation(b, angle);
    }
  }
  scene_tick(level->scene, dt);
}

double level_ground_height(level_t *level, double x) {
  double w = level->info.screen_max.x;
  switch (level->info.type) {
  case FOREST:
    return forest_height(x, w);
  case MESA:
    return mesa_height(x, w);
  case MOON:
    return moon_height(x, w);
  default:
    return ARENA_HEIGHT;
  }
}

const char *get_ground_info() { return GROUND_INFO; }

void level_destroy(level_t *level) {
  if (!level) {
    return;
  }
  scene_free(level->scene);
  free(level);
}
