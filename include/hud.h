#ifndef __HUD_H__
#define __HUD_H__
#include "turn_engine.h"

/**
 * Draw and update all text based HUD. To be called each frame
 * in state_tick
 *
 * @param eng turn engine handler
 */
void hud_draw(turn_engine_t *eng);

#endif // #ifndef __HUD_H__