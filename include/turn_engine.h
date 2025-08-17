#ifndef TURN_ENGINE_H
#define TURN_ENGINE_H

#include "arrow.h"
#include "camera.h"
#include "input.h"
#include "level.h"
#include <stdint.h>

typedef enum { CAM_PLAYER, CAM_ARROW } cam_mode_t;

typedef enum { PLAYER_ONE, PLAYER_TWO } player_id_t;

typedef struct turn_engine {
  level_t *level;
  camera_t *cam;
  cam_mode_t cam_mode;
  double user_zoom;
  double turn_len;
  double timer;
  player_id_t active;
  size_t p_body_idx[2];
  body_t *tracked_arrow;
  bool cpu_pending;
  size_t cpu_sample_idx;
  double cpu_best_err;
  double cpu_best_angle;
  double cpu_best_speed;
  arrow_variant_t equipped_arrow;
  double burst_animation_time;
} turn_engine_t;

/**
 * initializes the turn engine
 *
 * @param level the current level
 * @param cam camera object
 * @param turn_len_sec maximum amount of time a player can take before shooting
 * @param p1_body_idx index of the main player's body
 * @param p2_body_dx index of AI's body
 *
 * @return the initialized turn engine
 */
turn_engine_t *turn_engine_init(level_t *level, camera_t *cam,
                                double turn_len_sec, size_t p1_body_idx,
                                size_t p2_body_idx);

/**
 * updates the turn engine, checking if an arrow has been shot and
 * subsequently removed
 * @param eng current turn engine object
 * @param dt frame time increment
 */
void turn_engine_update(turn_engine_t *eng, double dt);

/**
 * registers a new arrow object. call upon an arrow being shot
 * @param eng engine to register the arrow within
 * @param arrow the arrow to register
 */
void turn_engine_register_arrow(turn_engine_t *eng, body_t *arrow);

/**
 * Frees a turn engine. Note: turn_engine_t does NOT take
 * ownership of level_t or camera_t
 * @param eng the turn engine to free
 */
void turn_engine_destroy(turn_engine_t *eng);

/**
 * Key handler for turn engine to be passed to sdl_wrapper
 * @param key key pressed
 * @param type one of KEY_PRESSED or KEY_RELEASED
 * @param held_time how long key has been down
 * @param aux auxilliary value, casted to turn_engine_t
 */
void turn_engine_on_key(char key, key_event_type_t type, double held_time,
                        void *aux);

/**
 * Find whether arrow is or isn't in flight
 * @param eng turn engine handler
 *
 * @return true if an arrow is found in flight, otherwise false.
 */
bool turn_engine_arrow_in_flight(turn_engine_t *eng);

/**
 * Get a random double between min and max
 * @param min lower bound
 * @param max upper bound
 *
 * @return double between the specified bounds
 */
double rand_double(double min, double max);

/**
 * retrieve a player's hp
 * @param eng turn engine handler
 * @param id PLAYER_ONE or PLAYER_TWO
 *
 * @return the player's hp
 */
int32_t eng_get_player_hp(turn_engine_t *eng, player_id_t id);

/**
 * retrieve a player's position
 * @param eng turn engine handler
 * @param id PLAYER_ONE or PLAYER_TWO
 *
 * @return a vector pointing to the player's centroid
 */
vector_t eng_get_player_pos(turn_engine_t *eng, player_id_t id);

/**
 * get the associated numeric value with respect to
 * arrow_variant_t
 *
 * @param eng turn engine handler
 *
 * @return number associated with arrow variant used
 */
size_t eng_get_equipped_index(turn_engine_t *eng);

#endif // TURN_ENGINE_H
