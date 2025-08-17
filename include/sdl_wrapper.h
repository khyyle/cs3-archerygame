#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
  ONE_KEY = 6,
  TWO_KEY = 7,
  THREE_KEY = 8,
} arrow_key_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in
 * seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * A mousepress handler
 * When a mouse button is clicked, the handler is passed
 *
 * @param state the current state of the game
 * @param type the type of mouse event
 * @param mouse_x the x-coordinate of the mouse when clicked
 * @param mouse_y the y-coordinate of the mouse when clicked
 */
typedef void (*mouse_handler_t)(state_t *state, mouse_event_type_t type,
                                double mouse_x, double mouse_y);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(state_t *state);

/**
 * Clears the screen. Should be called before drawing bodies in each frame.
 */
void sdl_clear(void);

/**
 * Draws a body using the color of the body.
 *
 * @param body the body struct to draw
 */
void sdl_draw_body(body_t *body);

/**
 * Loads an image from a file and returns it as an SDL texture.
 *
 * @param image_path the file path to the image
 * @return a pointer to the loaded texture
 */
SDL_Texture *sdl_get_image_texture(const char *image_path);

/**
 * Creates a texture for the given text in the given color
 *
 * @param text the text to render
 * @param color the texture color to render in
 * @param font the font to render the text in
 */
SDL_Texture *sdl_get_text_texture(const char *text, SDL_Color color,
                                  TTF_Font *font);

/**
 * Creates an SDL_Rect with the specified dimensions.
 *
 * @param x the x-coordinate of the rectangle
 * @param y the y-coordinate of the rectangle
 * @param w the width of the rectangle
 * @param h the height of the rectangle
 * @return a pointer to the created rectangle
 */
SDL_Rect *sdl_get_rect(double x, double y, double w, double h);

/**
 * Renders an image to the screen using the specified texture and rectangle.
 *
 * @param image_texture the texture to render
 * @param rect the rectangle defining the position and size of the rendered
 * image
 */
void sdl_render_image(SDL_Texture *image_texture, SDL_Rect *rect);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the bodies in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_body(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called for mouse events
 * Overwrites any existing mouse handler
 * @param handler the function to call with each mouse press
 */
void sdl_on_mouse(mouse_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Finds the bounding box for a given body
 *
 * @param body the body whose bounding box is to be computed
 *
 * @return SDL_Rect representing the body's bounding box
 */

SDL_Rect sdl_get_body_bounding_box(body_t *body);

/**
 * Maps a scene coordinate to a window coordinate
 * @param scene_pos position in scene
 * @param window_center center of window, given by get_window_center
 *
 * @return the mapped vector in world coords
 */
vector_t get_window_position(vector_t scene_pos, vector_t window_center);

/**
 * Check whether an x y pair is within a SDL_Rect
 *
 * @param x x-position in SDL coord system
 * @param y y-position in SDL coord system
 * @param rect region to check
 *
 * @return true if inside rect, false otherwise
 */
bool sdl_in_rect(double x, double y, SDL_Rect rect);

/**
 * Computes the center of the window in pixel coordinates
 * @return a vector pointing to the center
 */
vector_t get_window_center();

/**
 * Returns the renderer object
 */
SDL_Renderer *sdl_get_renderer();

/**
 * Draw and render a dot.
 * @param x x position (SDL coords)
 * @param y y position (SDL coords)
 * @param r radius of the dot to be draw
 * @param color color of the dot
 */
void draw_dot(int x, int y, int r, SDL_Color color);

#endif // #ifndef __SDL_WRAPPER_H__