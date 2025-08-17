#ifndef __CRATE_H__
#define __CRATE_H__

#include "camera.h"
#include "level.h"
#include <stdint.h>

typedef struct {
  const char *tag;
  int32_t hp;
} crate_info_t;

/**
 * Identify if something is a crate body
 */
bool crate_is(body_t *b);

/**
 * @param crate crate of interest
 * @return the hp of the crate
 */
int32_t crate_get_hp(body_t *crate);

/**
 * Spawn a crate at a random position in a level
 * @param level current level object
 *
 * @returns crate body to be added to the level
 */
body_t *crate_spawn(level_t *level);

/**
 * Render the hp label for a crate
 * @param scene current scene containing all bodies
 * @param cam camera object
 * @param font_path file of font to render in
 * @param color color for the text to be rendered in
 */
void crate_render_hp(scene_t *scene, camera_t *cam, const char *font_path,
                     const color_t color);

#endif // #ifndef __CRATE_H__