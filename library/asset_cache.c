#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
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
  switch (entry->type) {
  case ASSET_IMAGE: {
    SDL_DestroyTexture(entry->obj);
    break;
  }
  case ASSET_FONT: {
    TTF_CloseFont(entry->obj);
    break;
  }
  case ASSET_BUTTON: {
    asset_destroy(entry->obj);
    break;
  }
  default: {
    abort();
  }
  }
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->filepath == NULL) {
      continue;
    } else if (strcmp(entry->filepath, filepath) == 0) {
      assert(ty == entry->type);
      return entry->obj;
    }
  }
  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry != NULL);
  entry->type = ty;
  entry->filepath = filepath;
  switch (ty) {
  case ASSET_IMAGE: {
    entry->obj = sdl_load_image(filepath);
    break;
  }
  case ASSET_FONT: {
    entry->obj = sdl_load_font(filepath, FONT_SIZE);
    break;
  }
  default: {
    assert(false && "Attempted to pass invalid type to Asset");
  }
  }
  list_add(ASSET_CACHE, entry);
  return entry->obj;
}

void asset_cache_register_button(asset_t *button) {
  assert(button != NULL);
  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry != NULL);
  entry->type = ASSET_BUTTON;
  entry->filepath = NULL;
  entry->obj = button;
  list_add(ASSET_CACHE, entry);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->type == ASSET_BUTTON) {
      asset_on_button_click(entry->obj, state, x, y);
    }
  }
}
