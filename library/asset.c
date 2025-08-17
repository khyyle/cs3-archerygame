#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

static list_t *ASSET_LIST = NULL;
const size_t INIT_CAPACITY = 5;

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

static SDL_Color to_sdl_color(color_t c) {
  SDL_Color color = {c.red * 255.0f, c.green * 255.0f, c.blue * 255.0f};
  return color;
}

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  // This is a fancy way of malloc'ing space for an `image_asset_t` if `ty` is
  // ASSET_IMAGE, and `text_asset_t` otherwise.
  if (ASSET_LIST == NULL) {
    ASSET_LIST = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
  }
  asset_t *new =
      malloc(ty == ASSET_IMAGE ? sizeof(image_asset_t) : sizeof(text_asset_t));
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

void asset_make_image_with_body(const char *filepath, body_t *body) {
  SDL_Rect bounding_box = {.x = 0, .y = 0, .w = 0, .h = 0};
  asset_t *base = asset_init(ASSET_IMAGE, bounding_box);
  image_asset_t *img = (image_asset_t *)base;

  img->texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  img->body = body;
  list_add(ASSET_LIST, base);
}

void asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  asset_t *base = asset_init(ASSET_IMAGE, bounding_box);
  SDL_Texture *img = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);

  image_asset_t *img_entry = (image_asset_t *)base;
  img_entry->texture = img;
  img_entry->body = NULL;
  list_add(ASSET_LIST, base);
}

void asset_make_text(const char *filepath, SDL_Rect bounding_box,
                     const char *text, color_t color) {
  asset_t *base = asset_init(ASSET_TEXT, bounding_box);
  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_TEXT, filepath);

  text_asset_t *text_entry = (text_asset_t *)base;
  text_entry->font = font;
  text_entry->text = strdup(text);
  text_entry->color = color;

  int w, h;
  TTF_SizeText(font, text, &w, &h);
  base->bounding_box.w = w;
  base->bounding_box.h = h;
  list_add(ASSET_LIST, base);
}

void asset_reset_asset_list() {
  if (ASSET_LIST != NULL) {
    list_free(ASSET_LIST);
  }
  ASSET_LIST = list_init(INIT_CAPACITY, (free_func_t)asset_destroy);
}

list_t *asset_get_asset_list() { return ASSET_LIST; }

void asset_remove_body(body_t *body) {
  size_t i = list_size(ASSET_LIST);
  while (i > 0) {
    i--;
    asset_t *asset = list_get(ASSET_LIST, i);
    if (asset->type == ASSET_IMAGE) {
      image_asset_t *img = (image_asset_t *)asset;
      if (img->body == body) {
        list_remove(ASSET_LIST, i);
        asset_destroy(asset);
      }
    }
  }
}

void asset_render(asset_t *asset) {
  if (asset->type == ASSET_IMAGE) {
    image_asset_t *img = (image_asset_t *)asset;
    sdl_render_image(img->texture, &img->base.bounding_box);
    if (img->body) {
      img->base.bounding_box = sdl_get_body_bounding_box(img->body);
    }
  } else if (asset->type == ASSET_TEXT) {
    text_asset_t *text = (text_asset_t *)asset;
    SDL_Color color = to_sdl_color(text->color);
    SDL_Texture *text_texture =
        sdl_get_text_texture(text->text, color, text->font);
    sdl_render_image(text_texture, &text->base.bounding_box);
  }
}
asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

body_t *asset_get_body(asset_t *asset) {
  if (asset == NULL || asset->type != ASSET_IMAGE) {
    return NULL;
  }
  image_asset_t *img = (image_asset_t *)asset;
  return img->body;
}

void asset_destroy(asset_t *asset) {
  if (asset->type == ASSET_TEXT) {
    text_asset_t *t = (text_asset_t *)asset;
    free((void *)t->text);
  }
  free(asset);
}