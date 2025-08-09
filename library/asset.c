#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
  bool is_rotated;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new != NULL);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  image_asset_t *image = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
  image->body = NULL;
  image->texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  image->is_rotated = false;
  return (asset_t *)image;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  image_asset_t *image =
      (image_asset_t *)asset_init(ASSET_IMAGE, sdl_get_bounding_box(body));
  image->body = body;
  image->texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  image->is_rotated = false;
  return (asset_t *)image;
}

asset_t *asset_make_rotatable_image_with_body(const char *filepath,
                                              body_t *body) {
  image_asset_t *image =
      (image_asset_t *)asset_make_image_with_body(filepath, body);
  image->is_rotated = true;
  return (asset_t *)image;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  text_asset_t *asset = (text_asset_t *)asset_init(ASSET_FONT, bounding_box);
  asset->text = text;
  asset->color = color;
  asset->font = asset_cache_obj_get_or_create(ASSET_FONT, filepath);
  return (asset_t *)asset;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  assert(image_asset == NULL || image_asset->type == ASSET_IMAGE);
  assert(text_asset == NULL || text_asset->type == ASSET_FONT);
  button_asset_t *asset =
      (button_asset_t *)asset_init(ASSET_BUTTON, bounding_box);
  asset->text_asset = (text_asset_t *)text_asset;
  asset->image_asset = (image_asset_t *)image_asset;
  asset->handler = handler;
  asset->is_rendered = false;
  return (asset_t *)asset;
}

bool is_contained(double x, double y, SDL_Rect box) {
  return (bool)((x >= box.x) && (x <= box.x + box.w) && (y >= box.y) &&
                (y <= box.y + box.h));
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  button_asset_t *asset = (button_asset_t *)button;
  if (asset->is_rendered && is_contained(x, y, button->bounding_box)) {
    asset->handler(state);
    asset->is_rendered = false;
  }
}

void asset_rotate(asset_t *asset, double angle) { ; }

void asset_render(asset_t *asset) {
  // add a case for button assets.
  vector_t loc = {.x = asset->bounding_box.x, .y = asset->bounding_box.y};
  vector_t size = {.x = asset->bounding_box.w, .y = asset->bounding_box.h};
  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *image = (image_asset_t *)asset;
    if (image->body != NULL) {
      if (image->is_rotated) {
        SDL_Rect bounding_box =
            sdl_update_bounding_box_body(image->body, asset->bounding_box);
        sdl_display_image_with_angle(image->texture, bounding_box,
                                     body_get_rotation(image->body),
                                     body_get_centroid(image->body));
      } else {
        SDL_Rect bounding_box = sdl_get_bounding_box(image->body);
        loc = (vector_t){.x = bounding_box.x, .y = bounding_box.y};
        size = (vector_t){.x = bounding_box.w, .y = bounding_box.h};
        sdl_display_image(image->texture, loc, size);
      }
    } else {
      sdl_display_image(image->texture, loc, size);
    }
    break;
  }
  case ASSET_FONT: {
    text_asset_t *text = (text_asset_t *)asset;
    SDL_Color color = {
        .r = text->color.r, .g = text->color.g, .b = text->color.b, .a = 255};
    sdl_display_message(text->font, text->text, loc, color);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *button = (button_asset_t *)asset;
    if (button->image_asset != NULL) {
      asset_render((asset_t *)button->image_asset);
    }
    if (button->text_asset != NULL) {
      asset_render((asset_t *)button->text_asset);
    }
    button->is_rendered = true;
    break;
  }
  default: {
    abort();
  }
  }
}

body_t *asset_get_body(asset_t *asset) {
  if (asset->type != ASSET_IMAGE) {
    return NULL;
  }
  image_asset_t *image = (image_asset_t *)asset;
  return image->body;
}

void asset_set_body(asset_t *asset, body_t *body) {
  if (asset->type == ASSET_IMAGE) {
    image_asset_t *image = (image_asset_t *)asset;
    image->body = body;
  }
}

void asset_destroy(asset_t *asset) { free(asset); }
