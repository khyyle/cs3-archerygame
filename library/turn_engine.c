#include "turn_engine.h"
#include "arrow.h"
#include "crate.h"
#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double CAM_ZOOM = 1.4;
const double CAM_NORMAL = 1.0;
const double CAM_OFFSET_Y = 70;

const double DT = 0.05;        // time step for arrow integral approximation
const double SIM_TIME = 4.0;   // time interval to integrate arrow path over
const size_t SAMPLES = 50;     // 50 sample monte carlo simulation
const double SHOT_SCALE = 5.0; // shot power multiplier
const double MIN_SPEED = SHOT_SCALE * 60.0;
const double MAX_SPEED = SHOT_SCALE * 200.0;
const double MIN_ANGLE = 10.0 * M_PI / 180.0;
const double MAX_ANGLE = 80 * M_PI / 180.0;
const double BATCH_SIZE = 10; // how many AI samples per frame
const double MIN_AI_TURN_TIME = 5;

const double CRATE_SPAWN_CHANCE = 0.30;

void put_camera_on_p1(turn_engine_t *eng) {
  camera_t *cam = eng->cam;

  double half_w = (cam->screen_max.x - cam->screen_min.x) * 0.5 / cam->zoom;
  double half_h = (cam->screen_max.y - cam->screen_min.y) * 0.5 / cam->zoom;

  double cam_x = cam->screen_min.x + half_w;
  body_t *p = scene_get_body(eng->level->scene, eng->p_body_idx[PLAYER_ONE]);
  double player_y = body_get_centroid(p).y;
  double cam_y =
      fmax(player_y, cam->screen_min.y + half_h) + CAM_OFFSET_Y * CAM_ZOOM;
  camera_set_center(cam, (vector_t){cam_x, cam_y});
}

bool scene_contains_body(scene_t *scene, const body_t *body) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    if (scene_get_body(scene, i) == body) {
      return true;
    }
  }
  return false;
}

double rand_double(double min, double max) {
  double rand_unit = (double)rand() / ((double)RAND_MAX + 1.0);
  return min + (max - min) * rand_unit;
}

vector_t rand_wind(turn_engine_t *eng) {
  if (eng->level->max_wind == 0) {
    return (vector_t){0, 0};
  }
  double strength = rand_double(0.0, eng->level->max_wind);
  double theta = rand_double(0.0, 2.0 * M_PI);
  return (vector_t){.x = strength * cos(theta), .y = strength * sin(theta)};
}

void start_cpu_search(turn_engine_t *eng) {
  eng->cpu_pending = true;
  eng->cpu_sample_idx = 0;
  eng->cpu_best_err = __DBL_MAX__;
}

void enter_player_mode(turn_engine_t *eng) {
  eng->cam_mode = CAM_PLAYER;
  if (eng->active == PLAYER_ONE) {
    camera_set_zoom(eng->cam, CAM_ZOOM);
    put_camera_on_p1(eng);
  } else {
    camera_set_zoom(eng->cam, CAM_NORMAL);
    put_camera_on_p1(eng);
    start_cpu_search(eng);
  }
  put_camera_on_p1(eng);
}

void enter_arrow_mode(turn_engine_t *eng) {
  eng->cam_mode = CAM_ARROW;
  camera_set_zoom(eng->cam, CAM_NORMAL);
  if (eng->active == PLAYER_ONE) {
    vector_t arrow_pos = body_get_centroid(eng->tracked_arrow);
    camera_set_center(eng->cam, arrow_pos);
  }
}

void sync_zoom(turn_engine_t *eng) {
  if (eng->cam_mode != CAM_PLAYER || eng->active == PLAYER_TWO) {
    camera_set_zoom(eng->cam, CAM_NORMAL);
    return;
  }
  double target = eng->user_zoom;
  camera_set_zoom(eng->cam, target);
  put_camera_on_p1(eng);
}

double try_shot(turn_engine_t *eng, double angle, double speed) {
  vector_t accel = vec_add(eng->level->gravity, eng->level->wind);

  body_t *p2 = scene_get_body(eng->level->scene, eng->p_body_idx[PLAYER_TWO]);
  vector_t pos = body_get_centroid(p2);
  vector_t vel = {speed * cos(angle), speed * sin(angle)};

  body_t *target =
      scene_get_body(eng->level->scene, eng->p_body_idx[PLAYER_ONE]);
  vector_t target_pos = body_get_centroid(target);

  for (double t = 0; t < SIM_TIME; t += DT) {
    vel = vec_add(vel, vec_multiply(DT, accel));
    pos = vec_add(pos, vec_multiply(DT, vel));

    if (pos.y - 3.0 <= level_ground_height(eng->level, pos.x)) {
      break;
    }
  }
  vector_t diff = vec_subtract(pos, target_pos);
  return vec_dot(diff, diff);
}

void step_cpu_search(turn_engine_t *eng) {
  if (!eng->cpu_pending) {
    return;
  }
  if (eng->equipped_arrow != ARROW_STANDARD) {
    eng->equipped_arrow = ARROW_STANDARD;
  }

  for (size_t k = 0; k < BATCH_SIZE && eng->cpu_sample_idx < SAMPLES; k++) {
    double ang = rand_double(MIN_ANGLE, MAX_ANGLE);
    ang = M_PI - ang;
    double speed = rand_double(MIN_SPEED, MAX_SPEED);
    double err = try_shot(eng, ang, speed);
    if (err < eng->cpu_best_err) {
      eng->cpu_best_err = err;
      eng->cpu_best_angle = ang;
      eng->cpu_best_speed = speed;
    }
    eng->cpu_sample_idx++;
  }

  if (eng->cpu_sample_idx >= SAMPLES &&
      eng->timer <= eng->turn_len - MIN_AI_TURN_TIME) {
    body_t *shooter =
        scene_get_body(eng->level->scene, eng->p_body_idx[PLAYER_TWO]);
    vector_t vel = {eng->cpu_best_speed * cos(eng->cpu_best_angle),
                    eng->cpu_best_speed * sin(eng->cpu_best_angle)};
    body_t *arrow =
        arrow_spawn(eng->level->scene, shooter, vel, eng->equipped_arrow);
    turn_engine_register_arrow(eng, arrow);
    eng->cpu_pending = false;
  }
}

turn_engine_t *turn_engine_init(level_t *level, camera_t *cam, double turn_len,
                                size_t p1_idx, size_t p2_idx) {
  turn_engine_t *eng = calloc(1, sizeof(turn_engine_t));
  eng->level = level;
  eng->cam = cam;
  eng->turn_len = turn_len;
  eng->p_body_idx[0] = p1_idx;
  eng->p_body_idx[1] = p2_idx;
  eng->active = PLAYER_ONE;
  eng->timer = turn_len;
  eng->tracked_arrow = NULL;
  eng->user_zoom = CAM_ZOOM;
  eng->cam_mode = CAM_PLAYER;
  eng->cpu_pending = false;
  eng->equipped_arrow = ARROW_STANDARD;
  eng->burst_animation_time = 0;
  camera_set_zoom(cam, CAM_ZOOM);
  put_camera_on_p1(eng);
  eng->level->wind = rand_wind(eng);
  return eng;
}

void turn_engine_register_arrow(turn_engine_t *eng, body_t *arrow) {
  eng->burst_animation_time = 0.0;
  eng->tracked_arrow = arrow;
  arrow_clear_particles();
  enter_arrow_mode(eng);
}

void turn_engine_update(turn_engine_t *eng, double dt) {
  eng->timer -= dt;
  if (eng->burst_animation_time > 0.0) {
    eng->burst_animation_time -= dt;
    if (eng->burst_animation_time <= 0.0) {
      enter_player_mode(eng);
    }
    return;
  }

  if (eng->cpu_pending && !eng->tracked_arrow) {
    step_cpu_search(eng);
  }

  if (eng->tracked_arrow &&
      !scene_contains_body(eng->level->scene, eng->tracked_arrow)) {
    eng->tracked_arrow = NULL;
    eng->burst_animation_time = 3.0;
    eng->timer = 0.0;
  }

  if (eng->timer <= 0.0) {
    eng->active = eng->active == PLAYER_ONE ? PLAYER_TWO : PLAYER_ONE;
    eng->timer = eng->turn_len;
    arrow_clear_particles();
    eng->tracked_arrow = NULL;
    eng->level->wind = rand_wind(eng);
    enter_player_mode(eng);
    if (rand_double(0, 1) < CRATE_SPAWN_CHANCE) {
      crate_spawn(eng->level);
    }
  }
}

void turn_engine_on_key(char key, key_event_type_t type, double held_time,
                        void *aux) {
  state_t *state = aux;
  if (type == KEY_PRESSED) {
    switch (key) {
    case ONE_KEY:
      state->eng->equipped_arrow = ARROW_STANDARD;
      break;
    case TWO_KEY:
      state->eng->equipped_arrow = ARROW_HEAVY;
      break;
    case THREE_KEY:
      state->eng->equipped_arrow = ARROW_MULTI;
      break;
    case DOWN_ARROW:
      state->eng->user_zoom = CAM_NORMAL;
      sync_zoom(state->eng);
    default:
      break;
    }
  } else if (type == KEY_RELEASED) {
    switch (key) {
    case DOWN_ARROW:
      state->eng->user_zoom = CAM_ZOOM;
      sync_zoom(state->eng);
      break;
    default:
      break;
    }
  }
}

bool turn_engine_arrow_in_flight(turn_engine_t *eng) {
  return eng->tracked_arrow != NULL;
}

void turn_engine_destroy(turn_engine_t *eng) {
  level_destroy(eng->level);
  camera_destroy(eng->cam);
  free(eng);
}

int32_t eng_get_player_hp(turn_engine_t *eng, player_id_t id) {
  body_t *b = scene_get_body(eng->level->scene, eng->p_body_idx[id]);
  assert(b);
  return *(int32_t *)body_get_info(b);
}

vector_t eng_get_player_pos(turn_engine_t *eng, player_id_t id) {
  body_t *b = scene_get_body(eng->level->scene, eng->p_body_idx[id]);
  assert(b);
  return body_get_centroid(b);
}

size_t eng_get_equipped_index(turn_engine_t *eng) {
  switch (eng->equipped_arrow) {
  case ARROW_STANDARD:
    return 0;
  case ARROW_HEAVY:
    return 1;
  case ARROW_MULTI:
    return 2;
  default:
    exit(1);
  }
}