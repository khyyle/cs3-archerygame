#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "body.h"
#include "level.h"
#include "list.h"
#include "vector.h"
#include <stdbool.h>

/**
 * Represents the status of a collision between two shapes.
 * The shapes are either not colliding, or they are colliding along some axis.
 */
typedef struct {
  /** Whether the two shapes are colliding */
  bool collided;
  /**
   * If the shapes are colliding, the axis they are colliding on.
   * This is a unit vector pointing from the first shape towards the second.
   * Normal impulses are applied along this axis.
   * If collided is false, this value is undefined.
   */
  vector_t axis;
} collision_info_t;

/**
 * Computes the status of the collision between two bodies.
 *
 * @param body1 the first body
 * @param body2 the second body
 * @return whether the shapes are colliding, and if so, the collision axis.
 * The axis should be a unit vector pointing from shape1 towards shape2.
 */
collision_info_t find_collision(body_t *body1, body_t *body2);

/**
 * used to analytically determine collisions with the ground for a certain
 * body type because our levels don't generally satisfy the separating axis
 * theorem.
 *
 * @param level current level, necessary for computing level height
 * @param body body to check collision for
 * @param required_info info string a body must have for its collision to be
 * checked
 *
 * @return true if collision, false if body does not have required_info or no
 * collision
 */
bool alt_check_collision_certain_body(level_t *level, body_t *body,
                                      const char *required_info);

#endif // #ifndef __COLLISION_H__
