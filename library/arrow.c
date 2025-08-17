#include "arrow.h"
#include "asset.h"
#include "collision.h"
#include "crate.h"
#include "forces.h"
#include "list.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const color_t ARROW_COLOR = {0.78, 0.773, 0.341};
const size_t ARROW_CAPACITY = 8;
const double DAMAGE_SCALE = 0.03;
const double MAX_DAMAGE = 50;

const size_t ARROW_VERTEX_NUMBER = 5;
const char *ARROW_INFO = "arrow";
const char *BURST_PARTICLE_INFO = "particle";

const size_t CRATE_HEAL = 30;
const size_t SHOOTER_HP = 100;

const double PARTICLE_LIFETIME = 0.8;
const double PARTICLE_SIZE = 3.0; // for arrow trail
const double PARTICLE_SPEED = 20.0;
const color_t TRAIL_COLOR = {255, 100, 100};

const size_t BURST_PARTICLE_PTS = 10; // for impact effect
const double BURST_PARTICLE_RADIUS = 5;
const double BURST_PARTICLE_MASS = 0.5;
const color_t BURST_COLOR = {0.47, 0.17, 0.137};
const double BURST_SPEED = 100;
const double EPSILON =
    1.0; // finite difference dx value for finding normal vector

enum { MAX_PARTICLES = 300 };

/**
 * array containing particles to be rendered
 * across different frames
 */
particle_t particles[MAX_PARTICLES];

/**
 * The length of the particle_t array, particles.
 */
size_t particle_count = 0;

const struct {
  double ARROW_MASS;
  double SHAFT_LEN;
  double SHAFT_W;
  double TIP_LEN;
  double VEL_MUL;
} ARROW_SPECS[] = {
    {1.0, 24.0, 6.0, 6.0, 1.00},  /* standard */
    {2.5, 24.0, 10.0, 6.0, 0.60}, /* heavy */
    {0.3, 18.0, 6.0, 5.0, 1.2}    /* multishot */
};

typedef struct {
  body_t *shooter;
} arrow_aux_t;

void arrow_clear_particles() { particle_count = 0; }

size_t arrow_get_particle_count() { return particle_count; }

body_t *make_particle(vector_t centroid) {
  double d_theta = M_PI / BURST_PARTICLE_PTS;
  double theta = 0;
  list_t *vertices = list_init(BURST_PARTICLE_PTS, free);
  for (size_t i = 0; i < BURST_PARTICLE_PTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    *point = (vector_t){centroid.x + BURST_PARTICLE_RADIUS * cos(theta),
                        centroid.y + BURST_PARTICLE_RADIUS * sin(theta)};
    list_add(vertices, point);
    theta += d_theta;
  }
  body_t *particle =
      body_init_with_info(vertices, BURST_PARTICLE_MASS, BURST_COLOR,
                          (void *)BURST_PARTICLE_INFO, NULL);
  return particle;
}

void arrow_spawn_impact_burst(level_t *level, vector_t pos, size_t n) {
  double y1 = level_ground_height(level, pos.x - EPSILON);
  double y2 = level_ground_height(level, pos.x + EPSILON);
  vector_t normal = {-(y2 - y1), 2 * EPSILON};
  normal = vec_multiply(1.0 / vec_get_length(normal), normal);
  for (size_t i = 0; i < n && particle_count < MAX_PARTICLES; i++) {
    double theta = rand_double(-M_PI / 2, M_PI / 2);
    vector_t dir = {normal.x * cos(theta) - normal.y * sin(theta),
                    normal.x * sin(theta) + normal.y * cos(theta)};
    vector_t vel = vec_multiply(rand_double(0.2, 1) * BURST_SPEED, dir);
    body_t *particle = make_particle(pos);
    body_set_velocity(particle, vel);
    scene_add_body(level->scene, particle);
  }
}

void arrow_add_particle_trail(body_t *arrow, arrow_variant_t variant) {
  if (particle_count >= MAX_PARTICLES)
    return;

  vector_t arrow_pos = body_get_centroid(arrow);
  vector_t arrow_vel = body_get_velocity(arrow);

  vector_t dir = vec_multiply(-1 / vec_get_length(arrow_vel), arrow_vel);

  vector_t offset = vec_multiply(ARROW_SPECS[variant].SHAFT_LEN, dir);
  vector_t pos = vec_add(arrow_pos, offset);

  vector_t random_vel = {.x = rand_double(-PARTICLE_SPEED, PARTICLE_SPEED),
                         .y = rand_double(-PARTICLE_SPEED, PARTICLE_SPEED)};
  particles[particle_count] = (particle_t){
      .position = pos,
      .velocity = vec_add(vec_multiply(0.3, arrow_vel), random_vel),
      .lifetime = PARTICLE_LIFETIME,
      .max_lifetime = PARTICLE_LIFETIME,
      .size = PARTICLE_SIZE,
      .color = TRAIL_COLOR};
  particle_count++;
}

void arrow_update_particles(level_t *level, double dt,
                            arrow_variant_t variant) {
  scene_t *scene = level->scene;
  size_t num_bodies = scene_bodies(scene);
  for (size_t i = 0; i < num_bodies; i++) {
    body_t *b = scene_get_body(scene, i);
    if (strcmp(body_get_info(b), ARROW_INFO) == 0) {
      arrow_add_particle_trail(b, variant);
    }
  }
  for (size_t i = 0; i < particle_count;) {
    particles[i].lifetime -= dt;
    particles[i].position =
        vec_add(particles[i].position, vec_multiply(dt, particles[i].velocity));

    if (particles[i].lifetime <= 0) {
      particles[i] = particles[particle_count - 1];
      particle_count--;
    } else {
      i++;
    }
  }
}

void arrow_render_particles() {
  SDL_Renderer *ren = sdl_get_renderer();
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
  for (size_t i = 0; i < particle_count; i++) {
    particle_t p = particles[i];

    size_t alpha = (size_t)(255 * (p.lifetime / p.max_lifetime));
    SDL_SetRenderDrawColor(ren, p.color.red, p.color.green, p.color.blue,
                           alpha);

    vector_t screen_pos = get_window_position(p.position, get_window_center());
    SDL_Rect rect = {.x = screen_pos.x - p.size / 2,
                     .y = screen_pos.y - p.size / 2,
                     .w = p.size,
                     .h = p.size};
    SDL_RenderFillRect(ren, &rect);
  }
}

list_t *make_arrow_shape(vector_t pos, double shaft_len, double shaft_w,
                         double tip_len) {
  double hw = shaft_w * 0.5;
  list_t *v = list_init(ARROW_VERTEX_NUMBER, free);
  const vector_t verts[] = {{pos.x - shaft_len, pos.y - hw},
                            {pos.x, pos.y - hw},
                            {pos.x + tip_len, pos.y},
                            {pos.x, pos.y + hw},
                            {pos.x - shaft_len, pos.y + hw}};

  for (size_t i = 0; i < ARROW_VERTEX_NUMBER; ++i) {
    vector_t *p = malloc(sizeof *p);
    *p = verts[i];
    list_add(v, p);
  }
  return v;
};

void arrow_collision_handler(body_t *arrow, body_t *target, vector_t axis,
                             void *aux, double unused) {
  arrow_aux_t *arrow_details = aux;
  if (target == arrow_details->shooter) {
    return;
  }
  if (strcmp(body_get_info(target), get_ground_info()) == 0) {
    return;
  }

  double mass_factor =
      body_get_mass(arrow) / ARROW_SPECS[ARROW_STANDARD].ARROW_MASS;
  size_t damage =
      fmin(MAX_DAMAGE, DAMAGE_SCALE * mass_factor *
                           vec_get_length(body_get_velocity(arrow)));

  if (crate_is(target)) {
    crate_info_t *info = body_get_info(target);
    info->hp -= damage;
    if (info->hp <= 0) {
      asset_remove_body(target);
      body_remove(target);
      int32_t *shooter_hp = body_get_info(arrow_details->shooter);
      *shooter_hp = (*shooter_hp + CRATE_HEAL > SHOOTER_HP)
                        ? SHOOTER_HP
                        : *shooter_hp + CRATE_HEAL;
    }
  }
  int32_t *hp = body_get_info(target);
  if (hp) {
    *hp -= (int32_t)damage;
  }
  body_remove(arrow);
}

body_t *arrow_spawn(scene_t *scene, body_t *shooter, vector_t start_vel,
                    arrow_variant_t variant) {
  start_vel = vec_multiply(ARROW_SPECS[variant].VEL_MUL, start_vel);
  vector_t dir = vec_multiply(1.0 / vec_get_length(start_vel), start_vel);
  vector_t offset = vec_multiply(
      ARROW_SPECS[variant].SHAFT_LEN + ARROW_SPECS[variant].TIP_LEN * 0.5, dir);
  list_t *shape = make_arrow_shape(vec_add(body_get_centroid(shooter), offset),
                                   ARROW_SPECS[variant].SHAFT_LEN,
                                   ARROW_SPECS[variant].SHAFT_W,
                                   ARROW_SPECS[variant].TIP_LEN);
  body_t *arrow = body_init_with_info(shape, ARROW_SPECS[variant].ARROW_MASS,
                                      ARROW_COLOR, (void *)ARROW_INFO, NULL);
  body_set_velocity(arrow, start_vel);
  scene_add_body(scene, arrow);
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *other = scene_get_body(scene, i);
    if (other == arrow || other == shooter ||
        strcmp(body_get_info(other), get_ground_info()) == 0 ||
        strcmp(body_get_info(other), ARROW_INFO) == 0) {
      continue;
    }
    arrow_aux_t *aux = malloc(sizeof(arrow_aux_t));
    assert(aux);
    aux->shooter = shooter;
    create_collision(scene, arrow, other, arrow_collision_handler, aux, 0.0,
                     free);
  }
  return arrow;
}

double arrow_front_offset(arrow_variant_t variant) {
  return ARROW_SPECS[variant].SHAFT_LEN + ARROW_SPECS[variant].TIP_LEN * 0.5;
}

bool particle_check_ground_collision(level_t *level, body_t *body) {
  return alt_check_collision_certain_body(level, body, BURST_PARTICLE_INFO);
}

bool arrow_check_ground_collision(level_t *level, body_t *body) {
  return alt_check_collision_certain_body(level, body, ARROW_INFO);
}

double arrow_vel_scale(arrow_variant_t variant) {
  return ARROW_SPECS[variant].VEL_MUL;
}