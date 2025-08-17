#include "shoot.h"
#include "arrow.h"
#include "camera.h"
#include "sdl_wrapper.h"
#include "turn_engine.h"
#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double SHOT_POWER = 5.0;
const double MAX_DRAG_DIST = 200;
const double NORMAL_ZOOM = 1.0;
const double MIN_DRAG = 4.0;

const double ZOOM = 1.4;
const double ZOOM_THRESHOLD = 1e-3;
const vector_t TRAJ_OFFSET = {-20, -110};
const double PREVIEW_DT = 0.10;
const SDL_Color PREVIEW_COLOR = {255, 255, 255, 255};

enum { PREVIEW_DOTS = 4 };
/**
 * Where the mouse button went down
 */
static vector_t start_world = {0, 0};

static bool is_dragging = false;

/**
 * Array to hold the four points that
 * will preview our shot
 */
static vector_t preview_pts[PREVIEW_DOTS];

/**
 * to keep track of how many preview dots
 * should be rendered and dynamically add
 * points to preview_pts
 */
static size_t preview_cnt = 0;

vector_t unit_dir(vector_t from, vector_t to) {
  vector_t d = vec_subtract(to, from);
  double len = vec_get_length(d);
  return len ? vec_multiply(1.0 / len, d) : (vector_t){1, 0};
}

void shoot_begin(turn_engine_t *eng, double mouse_x, double mouse_y) {
  if (is_dragging || eng->active != PLAYER_ONE ||
      turn_engine_arrow_in_flight(eng) || eng->cam->zoom < NORMAL_ZOOM) {
    return;
  }
  start_world = camera_screen_to_world(eng->cam, (vector_t){mouse_x, mouse_y});
  is_dragging = true;
}

void shoot_drag(turn_engine_t *eng, double mouse_x, double mouse_y) {
  if (!is_dragging) {
    return;
  }
  vector_t release_w =
      camera_screen_to_world(eng->cam, (vector_t){mouse_x, mouse_y});
  vector_t dir = unit_dir(release_w, start_world);
  double drag = vec_get_length(vec_subtract(release_w, start_world));

  if (drag * eng->cam->zoom < MIN_DRAG) {
    preview_cnt = 0;
    return;
  }
  if (drag > MAX_DRAG_DIST) {
    drag = MAX_DRAG_DIST;
  }

  vector_t accel = vec_add(eng->level->gravity, eng->level->wind);

  body_t *shooter =
      scene_get_body(eng->level->scene, eng->p_body_idx[PLAYER_ONE]);
  arrow_variant_t variant = eng->equipped_arrow;
  vector_t pos = vec_add(body_get_centroid(shooter),
                         vec_multiply(arrow_front_offset(variant), dir));

  preview_cnt = 0;
  double vel_scale = arrow_vel_scale(variant);
  vector_t vel = vec_multiply(drag * SHOT_POWER * vel_scale, dir);
  for (size_t i = 0; i < PREVIEW_DOTS; i++) {
    vel = vec_add(vel, vec_multiply(PREVIEW_DT, accel));
    pos = vec_add(pos, vec_multiply(PREVIEW_DT, vel));
    preview_pts[preview_cnt] = pos;
    preview_cnt++;
  }
}

void shoot_render_preview(camera_t *cam) {
  SDL_Renderer *ren = sdl_get_renderer();
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

  for (size_t i = 0; i < preview_cnt; i++) {
    vector_t scr = camera_world_to_screen(cam, preview_pts[i]);
    if (fabs(cam->zoom - ZOOM) < ZOOM_THRESHOLD) {
      scr = vec_add(scr, TRAJ_OFFSET);
    }
    draw_dot(scr.x, scr.y, PREVIEW_DOTS, PREVIEW_COLOR);
  }
}

void shoot_end(turn_engine_t *eng, body_t *shooter, double mouse_x,
               double mouse_y) {
  if (!is_dragging)
    return;
  is_dragging = false;
  vector_t release_world =
      camera_screen_to_world(eng->cam, (vector_t){mouse_x, mouse_y});

  vector_t dir = unit_dir(release_world, start_world);

  double dist = vec_get_length(vec_subtract(release_world, start_world));
  if (dist * eng->cam->zoom < MIN_DRAG) {
    return;
  }
  if (dist > MAX_DRAG_DIST) {
    dist = MAX_DRAG_DIST;
  }
  vector_t vel = vec_multiply(dist * SHOT_POWER, dir);

  arrow_variant_t v = eng->equipped_arrow;
  if (v == ARROW_MULTI) {
    double offset = 7.0 * M_PI / 180;
    const double angles[3] = {-offset, 0, offset};
    for (size_t i = 0; i < 3; i++) {
      vector_t velocity = vec_rotate(vel, angles[i]);
      body_t *arr = arrow_spawn(eng->level->scene, shooter, velocity, v);
      if (i == 1) {
        turn_engine_register_arrow(eng, arr);
      }
    }
  } else {
    body_t *arr = arrow_spawn(eng->level->scene, shooter, vel, v);
    turn_engine_register_arrow(eng, arr);
  }
}