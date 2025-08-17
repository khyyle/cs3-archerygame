#ifndef ARROW_H
#define ARROW_H

#include "body.h"
#include "camera.h"
#include "level.h"
#include "scene.h"
#include "vector.h"

typedef enum {
  ARROW_STANDARD = 0,
  ARROW_HEAVY = 1,
  ARROW_MULTI = 2,
  ARROW_NVARIANTS
} arrow_variant_t;

typedef struct {
  vector_t position;
  vector_t velocity;
  double lifetime;
  double max_lifetime;
  double size;
  color_t color;
} particle_t;

/**
 * collision handler for an arrow, to be registered between the arrow and
 * any body that satisfies the separating axis theorem
 *
 * @param arrow arrow undergoing the collision
 * @param target target that could be hit by an arrow
 * @param axis axis along which the collision happens
 * @param aux auxilliary value; in our case, will be used to subtract hp
 * @param unused was force constant, not in use.
 */
void arrow_collision_handler(body_t *arrow, body_t *target, vector_t axis,
                             void *aux, double unused);

/**
 * Allocates a new arrow object and registers collisions for that arrow.
 * Only creates collisions between the opposing player; other objects either
 * shouldnt collide with this arrow or do not satsify SAT.
 *
 * @param scene the scene the arrow is being added to
 * @param shooter body the shot originates from
 * @param start_vel initial velocity
 * @param variant arrow variant (standard, heavy, multishot)
 *
 * @return a new arrow body, added to a scene
 */
body_t *arrow_spawn(scene_t *scene, body_t *shooter, vector_t start_vel,
                    arrow_variant_t variant);

/**
 * Registers a particle trail for a given arrow
 * @param arrow the arrow to register a particle trail for
 * @param variant type of the arrow (standard, heavy, multishot)
 */
void arrow_add_particle_trail(body_t *arrow, arrow_variant_t variant);

/**
 * Gets rid of any particles that have been coming
 * off the arrow for longer than PARTICLE_LIFETIME,
 * handles movement too.
 *
 * @param level the level to update particles for
 * @param dt timestep
 * @param variant type of the arrow (standard, heavy, multishot)
 */
void arrow_update_particles(level_t *level, double dt, arrow_variant_t variant);

/**
 * Spawn "dust" burst when arrow hits the ground or a body
 * @param level the level to spawn particles in
 * @param pos impact position
 * @param n number of particles in burst
 */
void arrow_spawn_impact_burst(level_t *level, vector_t pos, size_t n);

/**
 * To be called in state.c. Renders all partilces with
 * an alpha value proportional to how long they have been on screen
 */
void arrow_render_particles();

/**
 * Allows public access of ARROW_SPECS[]
 * @param variant type of arrow (standard, heavy, multishot)
 */
double arrow_front_offset(arrow_variant_t variant);

/**
 * Get rid of all onscreen particle effects. To be called
 * in turn_engine.c
 */
void arrow_clear_particles();

/**
 * @param level level to check for
 * @param body body to check
 *
 * @return false if not collided or not a particle, true if collided
 */
bool particle_check_ground_collision(level_t *level, body_t *body);

/**
 * @param level level to check for
 * @param body body to check
 *
 * @return false if not collided or not a arrow, true if collided
 */
bool arrow_check_ground_collision(level_t *level, body_t *body);

/**
 * Returns the amount of trail / impact particles to be rendered
 */
size_t arrow_get_particle_count();

/**
 * Expose velocity scale argument for rendering shot
 * preview
 *
 * @param variant arrow type
 *
 * @return the VEL_SCALE component
 */
double arrow_vel_scale(arrow_variant_t variant);

#endif // ARROW_H
