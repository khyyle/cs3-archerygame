#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct camera {
  vector_t screen_min, screen_max;
  vector_t center;
  double zoom;
} camera_t;

/**
 * @param screen_min  bottom left world coords of the window
 * @param screen_max  top right world coords of the window
 *
 * @return a newly initialized camera
 */
camera_t *camera_init(vector_t screen_min, vector_t screen_max);

/**
 * Move camera center (world coords). Prevents camera from showing
 * anything outside of window bounds
 * @param cam the camera whose center to set
 * @param the center in world coords to set to
 */
void camera_set_center(camera_t *cam, vector_t center);

/**
 * Adjusts the camera zoom. 1.0 for normal, <1.0 for zoom out,
 * >1.0 for zoom in.
 * @param cam camera object
 * @param zoom zoom level
 */
void camera_set_zoom(camera_t *cam, double zoom);

/**
 * Apply pan and zoom to the SDL_Renderer.
 * Must be called before any asset_render() or sdl_render_scene().
 * @param cam the cam to apply the adjusted settings to
 */
void camera_apply(const camera_t *cam);

/**
 * Restore renderer state (scale=1, full viewport).
 * @param cam the camera to destroy
 */
void camera_reset(const camera_t *cam);

/**
 * Maps SDL screen coords to camera based coords
 *
 * @param cam current camera
 * @param screen_pt the point on the screen to map
 *
 * @return screen_pt mapped to camera world coords
 */
vector_t camera_screen_to_world(camera_t *cam, vector_t screen_pt);

/**
 * Maps camera coords to SDL screen coords
 * @param cam current camera controller
 * @param world position in world coord
 *
 * @return world in SDL screen coords
 */
vector_t camera_world_to_screen(camera_t *cam, vector_t world);

/**
 * Frees the camera object fully
 * @param cam the camera to destroy
 */
void camera_destroy(camera_t *cam);

#endif // CAMERA_H
