#ifndef SHOOT_H
#define SHOOT_H

#include "arrow.h"
#include "level.h"
#include "turn_engine.h"

/**
 * Call on SDL_MOUSEBUTTONDOWN. Gets reference position for arrow
 * initial velocity computation.
 * @param eng current turn engine
 * @param mouse_x mouse x position in the window
 * @param mouse_y mouse y position in the window
 */
void shoot_begin(turn_engine_t *eng, double mouse_x, double mouse_y);

/**
 * Call on SDL_MOUSEMOTION. Handles drawing aiming visuals
 * @param eng current turn engine
 * @param mouse_x mouse x position in the window
 * @param mouse_y mouse y position in the window
 */
void shoot_drag(turn_engine_t *eng, double mouse_x, double mouse_y);

/**
 * To be called on SDL_MOUSEBUTTONUP. Handles firing the arrow
 * @param eng current turn engine
 * @param shooter body the arrow is to be shot from
 * @param mouse_x mouse x position in the window
 * @param mouse_y mouse y position in the window
 */
void shoot_end(turn_engine_t *eng, body_t *shooter, double mouse_x,
               double mouse_y);

/**
 * Render prospective shot path
 * @param cam current camera
 */
void shoot_render_preview(camera_t *cam);

#endif