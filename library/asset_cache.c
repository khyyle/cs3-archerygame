#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  if (entry->type == ASSET_IMAGE) {
    SDL_DestroyTexture(entry->obj);
  } else if (entry->type == ASSET_TEXT) {
    TTF_CloseFont(entry->obj);
  }
  free(entry);
}

static entry_t *find_existing_filepath(const char *filepath) {
  size_t n = list_size(ASSET_CACHE);
  for (size_t i = 0; i < n; i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (strcmp(entry->filepath, filepath) == 0) {
      return entry;
    }
  }
  return NULL;
}

void asset_cache_init() {
  TTF_Init();
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() {
  TTF_Quit();
  list_free(ASSET_CACHE);
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *existing_entry = find_existing_filepath(filepath);
  if (existing_entry != NULL) {
    return existing_entry->obj;
  }

  entry_t *new = malloc(sizeof(entry_t));
  if (ty == ASSET_IMAGE) {
    SDL_Texture *texture = sdl_get_image_texture(filepath);
    new->type = ty;
    new->filepath = filepath;
    new->obj = texture;
  } else if (ty == ASSET_TEXT) {
    TTF_Font *font = TTF_OpenFont(filepath, FONT_SIZE);
    new->type = ty;
    new->filepath = filepath;
    new->obj = font;
  }
  list_add(ASSET_CACHE, new);
  return new->obj;
}
