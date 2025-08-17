#include "camera.h"
#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

const double ZOOM_NORMAL = 1.0;

void clamp_center(camera_t *cam) {
  double half_w = (cam->screen_max.x - cam->screen_min.x) * 0.5 / cam->zoom;
  double half_h = (cam->screen_max.y - cam->screen_min.y) * 0.5 / cam->zoom;

  if (cam->center.x < cam->screen_min.x + half_w) {
    cam->center.x = cam->screen_min.x + half_w;
  }
  if (cam->center.x > cam->screen_max.x - half_w) {
    cam->center.x = cam->screen_max.x - half_w;
  }
  if (cam->center.y < cam->screen_min.y + half_h) {
    cam->center.y = cam->screen_min.y + half_h;
  }
  if (cam->center.y > cam->screen_max.y - half_h) {
    cam->center.y = cam->screen_max.y - half_h;
  }
}

camera_t *camera_init(vector_t screen_min, vector_t screen_max) {
  camera_t *c = malloc(sizeof(camera_t));
  c->screen_min = screen_min;
  c->screen_max = screen_max;
  c->zoom = ZOOM_NORMAL;

  c->center.x = (screen_min.x + screen_max.x) * 0.5;
  c->center.y = (screen_min.y + screen_max.y) * 0.5;
  return c;
}

void camera_set_center(camera_t *cam, vector_t center) {
  cam->center = center;
  clamp_center(cam);
}

void camera_set_zoom(camera_t *cam, double zoom) {
  cam->zoom = zoom;
  clamp_center(cam);
}

void camera_apply(const camera_t *cam) {
  SDL_Renderer *renderer = sdl_get_renderer();

  SDL_RenderSetScale(renderer, cam->zoom, cam->zoom);

  int32_t win_w = cam->screen_max.x - cam->screen_min.x;
  int32_t win_h = cam->screen_max.y - cam->screen_min.y;

  int32_t ofs_x = ((win_w * 0.5) - (cam->center.x * cam->zoom));
  int32_t ofs_y = ((win_h * 0.5) - (cam->center.y * cam->zoom));

  SDL_Rect vp = {ofs_x, ofs_y, win_w, win_h};
  SDL_RenderSetViewport(renderer, &vp);
}

void camera_reset(const camera_t *cam) {
  SDL_Renderer *renderer = sdl_get_renderer();
  SDL_RenderSetScale(renderer, 1.0, 1.0);
  SDL_RenderSetViewport(renderer,
                        &(SDL_Rect){cam->screen_min.x, cam->screen_min.y,
                                    (cam->screen_max.x - cam->screen_min.x),
                                    (cam->screen_max.y - cam->screen_min.y)});
}

vector_t camera_screen_to_world(camera_t *cam, vector_t screen_pt) {
  vector_t win_center = get_window_center();

  return (vector_t){cam->center.x + (screen_pt.x - win_center.x) * cam->zoom,
                    cam->center.y - (screen_pt.y - win_center.y) * cam->zoom};
}

vector_t camera_world_to_screen(camera_t *cam, vector_t world) {
  vector_t win_c = get_window_center();
  double z = cam->zoom;
  return (vector_t){.x = win_c.x + (world.x - cam->center.x) * z,
                    .y = win_c.y - (world.y - cam->center.y) * z};
}

void camera_destroy(camera_t *cam) { free(cam); }
