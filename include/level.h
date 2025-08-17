#ifndef LEVEL_H
#define LEVEL_H

#include "list.h"
#include "scene.h"
#include "vector.h"
#include <stddef.h>

typedef enum { FOREST, MESA, MOON, NONE } level_type_t;

typedef struct {
  const char *background_path;
  vector_t gravity;
  double max_wind;
  color_t terrain_color;
  vector_t screen_min;
  vector_t screen_max;
  size_t turn_len;
  level_type_t type;
} level_info_t;

typedef struct level {
  level_info_t info;
  scene_t *scene;
  vector_t gravity;
  vector_t wind;
  double max_wind;
} level_t;

/**
 * initialize a level
 * @param info the level_info_t describing a level
 * @return the intialized level
 */
level_t *level_init(const level_info_t info);

/**
 * updates bodies managed by a level via the physics engine
 * @param level the level to update
 * @param dt the timestep to apply
 */
void level_tick(level_t *level, double dt);

/**
 * get the height of the ground at a given x point
 * @param level the level whose ground to check
 * @param x_world x position in world coords
 *
 * @return y value of the height at that point in normal coord system
 */
double level_ground_height(level_t *level, double x_world);

/**
 * frees all assets for a given level
 * @param level the level to free
 */
void level_destroy(level_t *level);

/**
 * Exposes the ground's info string for effective comparison
 */
const char *get_ground_info();

#endif // LEVEL_H