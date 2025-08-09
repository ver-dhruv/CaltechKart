#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asset.h"
#include "asset_cache.h"
#include "background.h"
#include "car.h"
#include "checkpoints.h"
#include "collision.h"
#include "forces.h"
#include "power_up.h"
#include "sdl_wrapper.h"
#include <SDL2/SDL_mixer.h>

typedef enum { MENU, SETTINGS, RACE, PAUSE } game_state_t;
const double SCALE_CONST = 29875.0;
const size_t NUM_MENU_BUTTONS = 4;
const double G = 9.81;
const char *BACKGROUND_PATH = "assets/frogger-background.png";
const char *MINIMAP_PATH = "assets/minimap.png";
const double TRACK_MU = 2;
const double CAR_ELASTICITY = 2.5;
const size_t NUM_CARS = 3;
const double WALL_ELASTICITY = 1;
double AI_PATH_TOLERANCE = 0.2;
double STAR_MULTIPLIER = 1.2;

double EASY_AI_SPEED = 250;
double MED_AI_SPEED = 300;
double HARD_AI_SPEED = 350;
size_t NUM_VILLAINS = 4;

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};
const vector_t SPAWN_POS = {600, 200};
const SDL_Rect CAR_BOX = {.x = 350, .y = 175, .w = 50, .h = 100};
const SDL_Rect CAR_STATS_BOX = {.x = 450, .y = 160, .w = 234, .h = 175};
const SDL_Rect VILLAIN_CHOOSER_BOX = {.x = 50, .y = 100, .w = 300, .h = 111};
const char *VILLAIN_CHOOSER_PATHS[] = {
    "assets/easy_ai.png", "assets/mid_ai.png", "assets/hard_ai.png",
    "assets/ghost_setting.png"};

const SDL_Rect GAME_LOGO = {.x = 335, .y = 10, .w = 330, .h = 150};
const char *MENU_BACKGROUND_PATH = "assets/background.png";
const char *GAME_BACKGROUND_PATH = "assets/new_track.png";
const char *GAME_LOGO_PATH = "assets/caltech_karts_logo.png";
const char *CAR_STAT_PATHS[] = {"assets/f1_car_menu.png",
                                "assets/golf_cart_menu.png",
                                "assets/pickup_menu.png"};
const char *LAP_NO_PATHS[] = {
    "assets/lap_1.png",
    "assets/lap_2.png",
    "assets/lap_3.png",
};
const size_t NO_LAPS = 3;
const SDL_Rect LAP_BOX = {.x = 20, .y = 280, .w = 180, .h = 60};
const double WALL_WIDTH = 50.0;
const double MINIMAP_SCALE = 0.02;
const double MINI_CAR_RADIUS = 50;

const size_t NUM_BOXES = 3;
const double SHROOM_DURATION = 5;
const double STAR_DURATION = 10;
const double REVERSE_DURATION = 10;
const double ITEM_DISTANCE = 50; // how an item should be placed
const double SHELL_ROT_SPEED = 6.0;
const double STUN_ROT_SPEED = 2 * M_PI;

const vector_t START_IN = {408, 200};
const vector_t START_OUT = {640, 200};
const size_t START_OFFSET = 3;
const vector_t AI_START_OFFSET = {-120, 0};
const double MAX_W = M_PI / 32;

const char *WRONG_WAY_IMAGE_PATH = "assets/wrong_way.png";
const char *WRONG_WAY_ARROW_IMAGE_PATH = "assets/wrong_way_arrow.png";
const double WRONG_WAY_WIDTH = 60;
const double WRONG_WAY_HEIGHT = 30;
const double WRONG_WAY_OFFSET = 60;

const char *GAME_FONT_PATH = "assets/RetroMario-Regular.ttf";
const vector_t TIME_POSITION = {50, 250};

const char *GAME_MUSIC = "assets/game_music.wav";
const char *MENU_MUSIC = "assets/menu_music.wav";
const char *STAR_MUSIC = "assets/star_music.wav";

SDL_Rect WINDOW = {
    .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};

void toggle_pause(state_t *state);
void start_race(state_t *state);
void open_settings(state_t *state);
void prev_car(state_t *state);
void next_car(state_t *state);
void next_villain(state_t *state);
void prev_villain(state_t *state);
void restart_game(state_t *state);

typedef struct button_info {
  const char *image_path;
  const char *font_path;
  SDL_Rect image_box;
  SDL_Rect text_box;
  rgb_color_t text_color;
  const char *text;
  button_handler_t handler;
} button_info_t;

button_info_t PAUSE_BUTTON = {.image_path = "assets/pause_button.png",
                              .font_path = NULL,
                              .image_box = (SDL_Rect){900, 25, 75, 75},
                              .text_box = (SDL_Rect){900, 25, 0, 0},
                              .text_color = (rgb_color_t){255, 255, 255},
                              .text = NULL,
                              .handler = (void *)toggle_pause};

button_info_t MENU_BUTTON_TEMPLATES[] = {
    {.image_path = "assets/start.png",
     .font_path = NULL,
     .image_box = (SDL_Rect){200, 350, 600, 50},
     .text_box = (SDL_Rect){200, 350, 0, 0},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)start_race},
    {.image_path = "assets/settings.png",
     .font_path = NULL,
     .image_box = (SDL_Rect){200, 400, 600, 50},
     .text_box = (SDL_Rect){200, 400, 0, 0},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)open_settings},
    {.image_path = "assets/right_arrow.png",
     .font_path = NULL,
     .image_box =
         (SDL_Rect){CAR_BOX.x + 20, CAR_BOX.y + CAR_BOX.h + 10, 40, 40},
     .text_box = (SDL_Rect){CAR_BOX.x + 20, CAR_BOX.y + CAR_BOX.h + 20, 40, 40},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)next_car},
    {.image_path = "assets/left_arrow.png",
     .font_path = NULL,
     .image_box =
         (SDL_Rect){CAR_BOX.x - 20, CAR_BOX.y + CAR_BOX.h + 10, 40, 40},
     .text_box = (SDL_Rect){CAR_BOX.x - 20, CAR_BOX.y + CAR_BOX.h + 20, 40, 40},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)prev_car}};

button_info_t HOME_BUTTON = {.image_path = "assets/home.png",
                             .font_path = NULL,
                             .image_box = (SDL_Rect){225, 350, 500, 100},
                             .text_box = (SDL_Rect){200, 100, 0, 0},
                             .text_color = (rgb_color_t){255, 255, 255},
                             .text = NULL,
                             .handler = (void *)restart_game};

const char *INSTRUCTIONS_FILE_PATH = "assets/instructions.png";
SDL_Rect INSTRUCTIONS_BOX_SETTINGS = {600, 100, 300, 150};
SDL_Rect INSTRUCTIONS_BOX_PAUSE = {225, 100, 500, 250};

const size_t NUM_SETTINGS_BUTTONS = 2;

button_info_t SETTINGS_BUTTONS_TEMPLATES[] = {
    {.image_path = "assets/right_arrow.png",
     .font_path = NULL,
     .image_box = (SDL_Rect){150, 250, 40, 40},
     .text_box = (SDL_Rect){150, 250, 0, 0},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)next_villain},
    {.image_path = "assets/left_arrow.png",
     .font_path = NULL,
     .image_box = (SDL_Rect){100, 250, 40, 40},
     .text_box = (SDL_Rect){100, 250, 0, 0},
     .text_color = (rgb_color_t){255, 255, 255},
     .text = NULL,
     .handler = (void *)prev_villain},
};

struct state {
  double time;
  list_t *body_assets;
  list_t *lap_numbers;
  list_t *car_stats;
  list_t *villain_chooser;
  list_t *car_options;
  asset_t *pause_button;
  asset_t *menu_bg;
  asset_t *logo;
  car_type_t car_type;
  villain_type_t villain_type;
  list_t *settings_buttons;
  body_t *car;
  body_t *villain;
  asset_t *wrong_way_arrow;
  scene_t *scene;
  asset_t *bg;
  double villain_speed;
  checkpoint_state_t *checkpoint_state;
  game_state_t game_state;
  list_t *menu_buttons;
  asset_t *mini_map;
  asset_t *wrong_way;
  asset_t *mini_car;
  asset_t *mini_villain;
  list_t *inside_walls;
  list_t *outside_walls;
  asset_t *home_button;
  asset_t *instructions;
  bool *switches;
  list_t *boxes;
  list_t *shells;
  size_t *key_mapping;
  Mix_Chunk *menu_music;
  Mix_Chunk *game_music;
  Mix_Chunk *star_music;
  double best_time;
  double lap_time;
};

asset_t *create_button_from_info(state_t *state, button_info_t info) {
  asset_t *button_image = NULL;
  if (info.image_path != NULL) {
    button_image = asset_make_image(info.image_path, info.image_box);
  }
  asset_t *button_text = NULL;
  if (info.text != NULL) {
    button_text = asset_make_text(info.font_path, info.text_box, info.text,
                                  info.text_color);
  }
  SDL_Rect box;
  if (button_image == NULL) {
    box = info.text_box;
  } else {
    box = info.image_box;
  }
  asset_t *button =
      asset_make_button(box, button_image, button_text, info.handler);
  asset_cache_register_button(button);
  return button;
}

list_t *inside_walls() {
  list_t *points = inside_walls_points(WALL_WIDTH);
  size_t size = list_size(points);
  list_t *inside_walls = list_init(size, NULL);
  for (size_t i = 0; i < size; i++) {
    body_t *body = body_init(list_get(points, i), INFINITY, get_blue());
    list_add(inside_walls, body);
  }
  return inside_walls;
}

list_t *outside_walls() {
  list_t *points = outside_walls_points(WALL_WIDTH);
  size_t size = list_size(points);
  list_t *outside_walls = list_init(size, NULL);
  for (size_t i = 0; i < size; i++) {
    body_t *body = body_init(list_get(points, i), INFINITY, get_blue());
    list_add(outside_walls, body);
  }
  return outside_walls;
}

void race_free(state_t *state) {
  // remember to set things to null after freeing
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    body_remove(asset_get_body(list_get(state->body_assets, i)));
  }
  list_free(state->body_assets);
  for (size_t i = 0; i < list_size(state->shells); i++) {
    body_remove(asset_get_body(list_get(state->shells, i)));
  }
  list_free(state->shells);
  for (size_t i = 0; i < list_size(state->boxes); i++) {
    body_remove(asset_get_body(list_get(state->boxes, i)));
  }
  list_free(state->boxes);
  state->bg = NULL;
  state->car = NULL;
  state->villain = NULL;
  body_remove(asset_get_body(state->mini_map));
  asset_destroy(state->mini_map);
  state->mini_map = NULL;
  body_remove(asset_get_body(state->mini_car));
  asset_destroy(state->mini_car);
  state->mini_car = NULL;
  body_remove(asset_get_body(state->mini_villain));
  asset_destroy(state->mini_villain);
  state->mini_villain = NULL;

  return;
}

body_t *background() {
  return body_init(background_list(), INFINITY, get_blue());
}

void use_item(state_t *state) {
  body_t *car = state->car;
  if (car_has_shell(car)) {
    power_up_info_t info = car_get_powerup_state(car);
    body_t *shell = info.shell;
    info.shell = NULL;
    car_set_powerup_state(car, info);
    create_stun_collision(state->scene, car, shell, 1.0);
    size_t size = list_size(state->inside_walls);
    for (size_t i = 0; i < size; i++) {
      create_physics_collision(state->scene, list_get(state->inside_walls, i),
                               shell, WALL_ELASTICITY);
      create_physics_collision(state->scene, list_get(state->outside_walls, i),
                               shell, WALL_ELASTICITY);
    }
    return;
  }
  power_up_info_t info = car_get_powerup_state(car);
  double theta = body_get_rotation(car);
  vector_t direction = {.x = sin(theta), .y = -cos(theta)};
  power_up_type_t power = info.power_up;
  switch (power) {
  case NONE: {
    break;
  }
  case SHROOM: {
    info.fast = fmax(info.fast, SHROOM_DURATION);
    vector_t vel = body_get_velocity(car);
    if (vec_get_length(vel) == 0) {
      vel = direction;
    }
    vel = vec_multiply(get_speed_multiplier() * car_get_top_speed(car) /
                           vec_get_length(vel),
                       vel);
    body_add_impulse(car,
                     vec_multiply(body_get_mass(car),
                                  vec_subtract(vel, body_get_velocity(car))));
    break;
  }
  case STAR: {
    Mix_HaltChannel(0);
    Mix_PlayChannel(0, state->star_music, -1);
    info.immune = STAR_DURATION;
    vector_t vel = body_get_velocity(car);
    if (vec_get_length(vel) == 0) {
      vel = direction;
    }
    vel = vec_multiply(
        STAR_MULTIPLIER * car_get_top_speed(car) / vec_get_length(vel), vel);
    body_add_impulse(car,
                     vec_multiply(body_get_mass(car),
                                  vec_subtract(vel, body_get_velocity(car))));
    break;
  }
  case REVERSE: {
    info.reverse = REVERSE_DURATION;
    printf("CONTROLS SCRAMBLED!\n");
    permute(state->key_mapping, 4);
    break;
  }
  case FAKE: {
    vector_t center = vec_subtract(body_get_centroid(state->car),
                                   vec_multiply(ITEM_DISTANCE, direction));
    asset_t *box = make_box(center);
    list_add(state->body_assets, box);
    scene_add_body(state->scene, asset_get_body(box));
    create_stun_collision(state->scene, state->car, asset_get_body(box),
                          3.0); // constant over 2
    create_stun_collision(state->scene, state->villain, asset_get_body(box),
                          3.0); // constant over 2
    break;
  }
  case SHELL: {
    vector_t center = vec_subtract(body_get_centroid(state->car),
                                   vec_multiply(ITEM_DISTANCE, direction));
    asset_t *shell = make_shell(center, theta, SHELL_ROT_SPEED);
    body_t *body = asset_get_body(shell);
    info.shell = body;
    scene_add_body(state->scene, body);
    size_t size = list_size(state->shells);
    for (ssize_t i = 0; i < size; i++) {
      body_t *curr = asset_get_body(list_get(state->shells, i));
      if (curr == NULL) {
        list_remove(state->shells, i);
        i--;
        size--;
      } else {
        create_shell_collision(state->scene, curr, body);
      }
    }
    list_add(state->shells, shell);
    create_stun_collision(state->scene, state->villain, body,
                          1.0); // constant under 2
    break;
  }
  case BOOST: {
    asset_t *boost = make_boost(state->car);
    body_t *body = asset_get_body(boost);
    scene_add_body(state->scene, body);
    list_add(state->body_assets, boost);
    create_boost_collision(state->scene, state->car, body);
    break;
  }
  default:
    break;
  }
  info.power_up = NONE;
  car_set_powerup_state(car, info);
}

/* KEY HANDLER */
void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  if (state->game_state != RACE) {
    return;
  }
  body_t *car = state->car;
  vector_t vel = body_get_velocity(car);
  power_up_info_t car_powerups = car_get_powerup_state(state->car);
  if (car_powerups.stun > 0) {
    return;
  }
  double multiplier = 1;
  if (car_powerups.fast > 0) {
    multiplier = get_speed_multiplier();
  } else if (car_powerups.immune > 0) {
    multiplier = STAR_MULTIPLIER;
  }
  double speed = vec_get_length(vel);
  double theta = body_get_rotation(car);
  vector_t direction = {.x = sin(theta), .y = -cos(theta)};
  double top_speed = multiplier * car_get_top_speed(car);
  double turining_radius =
      vec_get_length(vel) * vec_get_length(vel) / (car_get_friction(car) * G);
  if (key < 5 && car_get_powerup_state(state->car).reverse > 0) {
    key = state->key_mapping[key - 1] + 1;
  }
  if (type == KEY_PRESSED) {
    switch (key) {
    case UP_ARROW: {
      if (speed <= top_speed) {
        vector_t acceleration =
            vec_multiply(car_get_acceleration(car) * held_time, direction);
        vel = vec_add(vel, acceleration);
      } else {
        vel = vec_multiply(top_speed / speed, vel);
      }
      break;
    }
    case DOWN_ARROW: {
      if (speed <= top_speed) {
        vector_t acceleration =
            vec_multiply(car_get_acceleration(car) * held_time, direction);
        vel = vec_subtract(vel, acceleration);
      } else {
        vel = vec_multiply(top_speed / speed, vel);
      }
      break;
    }
    case LEFT_ARROW: {
      if (vel.x == 0 && vel.y == 0) {
        break;
      }
      double dtheta =
          sqrt(car_get_friction(car) * G / turining_radius) * held_time;
      if (dtheta > MAX_W) {
        dtheta = MAX_W;
      }
      body_set_rotation(car, body_get_rotation(car) + dtheta);
      break;
    }
    case RIGHT_ARROW: {
      if (vel.x == 0 && vel.y == 0) {
        break;
      }
      double dtheta =
          sqrt(car_get_friction(car) * G / turining_radius) * held_time;
      if (dtheta > MAX_W) {
        dtheta = MAX_W;
      }
      body_set_rotation(car, body_get_rotation(car) - dtheta);
      break;
    }
    case SPACE_BAR: {
      use_item(state);
      break;
    }
    case R: {
      if (car_has_shell(car)) {
        flip_shell(car_get_powerup_state(car).shell);
      }
      break;
    }
    default:
      break;
    }
  }
  body_set_velocity(car, vel);
}

void handle_checkpoint_state(state_t *state, double dt) {
  checkpoint_state_t *checkpoint_state = car_get_checkpoint_state(state->car);
  if (get_wrong_way(state->car, checkpoint_state)) {
    asset_render(state->wrong_way);
    asset_render(state->wrong_way_arrow);
    set_wrong_way_time(checkpoint_state,
                       get_wrong_way_time(checkpoint_state) + dt);
  } else {
    set_wrong_way_time(checkpoint_state, 0);
  }
  if (get_lap_over(checkpoint_state)) {
    reset_checkpoint_state(checkpoint_state);
    car_set_laps_done(state->car, car_get_laps_done(state->car) + 1);
    if (state->best_time == 0.0 || state->lap_time <= state->best_time) {
      state->best_time = state->lap_time;
    }
    state->lap_time = 0.0;
  } else {
    state->lap_time += dt;
  }
  if (get_lap_over(car_get_checkpoint_state(state->villain))) {
    reset_checkpoint_state(car_get_checkpoint_state(state->villain));
    car_set_laps_done(state->villain, car_get_laps_done(state->villain) + 1);
  }
}

void create_mini_map(state_t *state) {
  list_t *map = background_list();
  vector_t vec = vec_subtract(*(vector_t *)list_get(map, 3),
                              *(vector_t *)list_get(map, 1));
  list_free(map);
  vec = vec_multiply(MINIMAP_SCALE, vec);
  vector_t center = vec_multiply(0.5, vec);
  center = (vector_t){.x = center.x, .y = MAX.y - center.y};
  list_t *points = make_rectangle(center, vec.x, vec.y);
  body_t *body = body_init(points, INFINITY, get_blue());
  state->mini_map = asset_make_image_with_body(MINIMAP_PATH, body);
  state->mini_car = make_mini_car(state->car_type);
  state->mini_villain = make_mini_villain(state->villain_type);
}

void update_mini_map(state_t *state) {
  vector_t bg_center = body_get_centroid(asset_get_body(state->bg));
  vector_t car_center = body_get_centroid(state->car);
  vector_t car_pos = vec_subtract(car_center, bg_center);
  car_pos = vec_add(body_get_centroid(asset_get_body(state->mini_map)),
                    vec_multiply(MINIMAP_SCALE, car_pos));
  body_set_centroid(asset_get_body(state->mini_car), car_pos);
  body_set_rotation(asset_get_body(state->mini_car),
                    body_get_rotation(state->car));
  vector_t villain_center = body_get_centroid(state->villain);
  vector_t villain_pos = vec_subtract(villain_center, bg_center);
  villain_pos = vec_add(body_get_centroid(asset_get_body(state->mini_map)),
                        vec_multiply(MINIMAP_SCALE, villain_pos));
  body_set_centroid(asset_get_body(state->mini_villain), villain_pos);
  body_set_rotation(asset_get_body(state->mini_villain),
                    body_get_rotation(state->villain) + M_PI);
}

void update_arrow(state_t *state) {
  double theta = body_get_rotation(state->car);
  vector_t direction = {.x = sin(theta), .y = -cos(theta)};
  direction = vec_multiply(-WRONG_WAY_OFFSET, direction);
  body_t *arrow_body = asset_get_body(state->wrong_way_arrow);
  body_set_centroid(arrow_body,
                    vec_add(direction, body_get_centroid(state->car)));
  vector_t right_way = get_right_way_from_position(
      car_get_checkpoint_state(state->car), state->car);
  body_set_rotation(arrow_body, -atan2(right_way.y, right_way.x));
}

void create_boxes(state_t *state) {
  state->boxes = list_init(2, (free_func_t)asset_destroy);
  list_t *inside = get_inside_out();
  list_t *outside = get_outside_in();
  size_t size = list_size(inside);
  for (size_t i = 0; i < size; i += 2) {
    vector_t in = *(vector_t *)list_get(inside, i);
    vector_t out = *(vector_t *)list_get(outside, i);
    for (size_t j = 1; j <= NUM_BOXES; j++) {
      vector_t center =
          vec_add(vec_multiply(j, in), vec_multiply(NUM_BOXES + 1 - j, out));
      center = vec_multiply(1.0 / ((double)NUM_BOXES + 1.0), center);
      asset_t *box = make_box(center);
      body_t *body = asset_get_body(box);
      scene_add_body(state->scene, body);
      create_box_collision(state->scene, state->car, body, state->switches);
      create_box_collision(state->scene, state->villain, body, state->switches);
      list_add(state->boxes, box);
    }
  }
  list_free(inside);
  list_free(outside);
}

void menu_free(state_t *state) {
  list_free(state->car_options);
  list_free(state->car_stats);
  list_free(state->menu_buttons);
  asset_destroy(state->menu_bg);
  asset_destroy(state->logo);
  state->car_options = NULL;
  state->logo = NULL;
  state->car_stats = NULL;
  state->menu_buttons = NULL;
  state->menu_bg = NULL;
}

void next_villain(state_t *state) {
  state->villain_type = (state->villain_type + 1) % NUM_VILLAINS;
}
void prev_villain(state_t *state) {
  state->villain_type = (state->villain_type + 3) % NUM_VILLAINS;
}

double get_villain_speed(state_t *state, villain_type_t villain_type) {
  switch (villain_type) {
  case EASY_AI:
    return EASY_AI_SPEED;
  case MEDIUM_AI:
    return MED_AI_SPEED;
  case HARD_AI:
    return HARD_AI_SPEED;
  case GHOST:
    return SCALE_CONST / (state->best_time);
  }
}

void start_race(state_t *state) {
  Mix_HaltChannel(0);
  Mix_PlayChannel(0, state->game_music, -1);
  state->time = 0;
  state->game_state = RACE;
  state->key_mapping = malloc(4 * sizeof(size_t));
  for (size_t i = 0; i < 4; i++) {
    state->key_mapping[i] = i;
  }
  menu_free(state);

  state->body_assets = list_init(2, (free_func_t)asset_destroy);
  body_t *bg_body = background();
  scene_add_body(state->scene, bg_body);
  state->bg = asset_make_image_with_body(GAME_BACKGROUND_PATH, bg_body);
  list_add(state->body_assets, state->bg);
  state->lap_numbers = list_init(NO_LAPS, (free_func_t)asset_destroy);
  for (size_t i = 0; i < NO_LAPS; i++) {
    list_add(state->lap_numbers, asset_make_image(LAP_NO_PATHS[i], LAP_BOX));
  }
  state->pause_button = create_button_from_info(state, PAUSE_BUTTON);

  body_t *car = make_car(state->car_type);
  body_set_centroid(car, SPAWN_POS);
  state->car = car;
  list_add(state->body_assets, make_car_image(car));
  body_set_rotation(car, M_PI);
  scene_add_body(state->scene, car);
  create_drag(state->scene, TRACK_MU, car);

  body_t *villain = make_car((state->car_type + 1) % NUM_CARS);
  body_set_centroid(villain, vec_add(AI_START_OFFSET, SPAWN_POS));
  state->villain = villain;
  list_add(state->body_assets, make_car_image(villain));
  body_set_rotation(villain, M_PI);
  scene_add_body(state->scene, villain);
  create_drag(state->scene, TRACK_MU, villain);
  state->villain_speed = get_villain_speed(state, state->villain_type);
  change_top_speed(villain, state->villain_speed);

  body_t *arrow_body =
      body_init(make_rectangle(VEC_ZERO, WRONG_WAY_WIDTH, WRONG_WAY_HEIGHT),
                INFINITY, get_blue());
  state->wrong_way_arrow = asset_make_rotatable_image_with_body(
      WRONG_WAY_ARROW_IMAGE_PATH, arrow_body);

  if (state->villain_type != GHOST) {
    create_car_collision(state->scene, state->car, villain);
  }

  list_t *checkpoints = make_checkpoints(get_inside_out(), get_outside_in(),
                                         START_IN, START_OUT, START_OFFSET);
  checkpoint_state_t *checkpoint_state_car = checkpoint_state_init(checkpoints);
  checkpoint_state_t *checkpoint_state_villain =
      checkpoint_state_init(checkpoints);

  car_set_checkpoint_state(state->car, checkpoint_state_car);
  car_set_checkpoint_state(state->villain, checkpoint_state_villain);

  state->wrong_way = asset_make_image(WRONG_WAY_IMAGE_PATH, GAME_LOGO);

  for (size_t i = 0; i < list_size(checkpoints); i++) {
    scene_add_body(state->scene, list_get(checkpoints, i));
    create_collision(state->scene, state->car, list_get(checkpoints, i),
                     checkpoint_collision, checkpoint_state_car, 0);
    create_collision(state->scene, state->villain, list_get(checkpoints, i),
                     checkpoint_collision, checkpoint_state_villain, 0);
  }

  state->outside_walls = outside_walls();
  state->inside_walls = inside_walls();
  size_t len_outside_walls = list_size(state->outside_walls);
  size_t len_inside_walls = list_size(state->inside_walls);
  for (size_t i = 0; i < len_outside_walls; i++) {
    scene_add_body(state->scene, list_get(state->outside_walls, i));
    create_physics_collision(state->scene, state->car,
                             list_get(state->outside_walls, i),
                             WALL_ELASTICITY);
    create_physics_collision(state->scene, state->villain,
                             list_get(state->outside_walls, i),
                             WALL_ELASTICITY);
  }
  for (size_t i = 0; i < len_inside_walls; i++) {
    scene_add_body(state->scene, list_get(state->inside_walls, i));
    create_physics_collision(state->scene, state->car,
                             list_get(state->inside_walls, i), WALL_ELASTICITY);
    create_physics_collision(state->scene, state->villain,
                             list_get(state->inside_walls, i), WALL_ELASTICITY);
  }
  state->switches = malloc(6 * sizeof(bool));
  for (size_t i = 0; i < 6; i++)
    state->switches[i] = true;
  create_boxes(state);
  state->shells = list_init(2, (free_func_t)asset_destroy);
  create_mini_map(state);
  sdl_on_key((key_handler_t)on_key);
}

void next_car(state_t *state) {
  state->car_type = ((state->car_type + 1) % NUM_CARS);
}
void prev_car(state_t *state) {
  state->car_type = ((state->car_type + 2) % NUM_CARS);
}

void create_menu_buttons(state_t *state) {
  for (size_t i = 0; i < NUM_MENU_BUTTONS; i++) {
    button_info_t info = MENU_BUTTON_TEMPLATES[i];
    asset_t *button = create_button_from_info(state, info);
    list_add(state->menu_buttons, button);
  }

  state->best_time = 170;
  state->lap_time = 0.0;
}

void fix_villain_path(body_t *car) {
  double theta = body_get_rotation(car);
  vector_t direction = {.x = sin(theta), .y = -cos(theta)};
  vector_t right_way =
      get_right_way_from_position(car_get_checkpoint_state(car), car);
  double dot_prod = vec_dot(direction, right_way);
  if (abs(dot_prod > AI_PATH_TOLERANCE)) {
    theta = theta + atan2(right_way.y, right_way.x) -
            atan2(direction.y, direction.x);
    body_set_rotation(car, theta);
  }
  vector_t new_dir = {sin(theta), -cos(theta)};
  vector_t velocity = vec_multiply(car_get_top_speed(car), new_dir);
  body_set_velocity(car, velocity);
}

void show_menu(state_t *state) {
  asset_render(state->menu_bg);
  asset_render(state->logo);
  for (size_t i = 0; i < NUM_MENU_BUTTONS; i++) {
    asset_render(list_get(state->menu_buttons, i));
  }
  asset_render(list_get(state->car_options, state->car_type));
  asset_render(list_get(state->car_stats, state->car_type));
}

asset_t *make_time_asset(double time) {
  size_t seconds = (size_t)floor(time);
  size_t minutes = seconds / 60;
  size_t milliseconds = (size_t)(1000.0 * (time - seconds));
  seconds = seconds % 60;
  char *aux = malloc(5 * sizeof(char));
  if (seconds < 10) { // 1 digit
    sprintf(aux, ":0%zu.", seconds);
  } else {
    sprintf(aux, ":%zu.", seconds);
  }
  char *text = malloc(12 * sizeof(char)); // give more than enough space
  sprintf(text, "%zu%s%zu", minutes, aux, milliseconds);
  free(aux);
  SDL_Rect rect = {
      .x = TIME_POSITION.x, .y = TIME_POSITION.y, .w = 100, .h = 100};
  asset_t *asset = asset_make_text(GAME_FONT_PATH, rect, text, get_blue());
  return asset;
}

/* EMSCRIPTEN FUNCTIONS */
state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  // Initialie mixer
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048);
  Mix_AllocateChannels(1);
  state_t *state = malloc(sizeof(state_t));
  state->villain_type = MEDIUM_AI;
  state->scene = scene_init();
  state->game_state = MENU;
  srand(time(NULL));
  restart_game(state);
  return state;
}

void toggle_pause(state_t *state) {
  if (state->game_state == RACE) {
    state->game_state = PAUSE;
    state->home_button = create_button_from_info(state, HOME_BUTTON);
    state->instructions =
        asset_make_image(INSTRUCTIONS_FILE_PATH, INSTRUCTIONS_BOX_PAUSE);
  } else {
    state->game_state = RACE;
    asset_destroy(state->home_button);
    asset_destroy(state->instructions);
    state->home_button = NULL;
    state->instructions = NULL;
  }
}

void create_settings_buttons(state_t *state) {
  for (size_t i = 0; i < NUM_SETTINGS_BUTTONS; i++) {
    button_info_t info = SETTINGS_BUTTONS_TEMPLATES[i];
    asset_t *button = create_button_from_info(state, info);
    list_add(state->settings_buttons, button);
  }
}

void open_settings(state_t *state) {
  menu_free(state);
  state->menu_bg = asset_make_image(MENU_BACKGROUND_PATH, WINDOW);
  state->home_button = create_button_from_info(state, HOME_BUTTON);
  state->villain_chooser = list_init(NUM_VILLAINS, (free_func_t)asset_destroy);
  state->game_state = SETTINGS;
  for (size_t i = 0; i < NUM_VILLAINS; i++) {
    asset_t *villain_choice =
        asset_make_image(VILLAIN_CHOOSER_PATHS[i], VILLAIN_CHOOSER_BOX);
    list_add(state->villain_chooser, villain_choice);
  }
  state->settings_buttons =
      list_init(NUM_SETTINGS_BUTTONS, (free_func_t)asset_destroy);
  create_settings_buttons(state);
  state->instructions =
      asset_make_image(INSTRUCTIONS_FILE_PATH, INSTRUCTIONS_BOX_SETTINGS);
}

void settings_free(state_t *state) {
  list_free(state->settings_buttons);
  asset_destroy(state->menu_bg);
  asset_destroy(state->instructions);
  asset_destroy(state->home_button);
}

void show_settings(state_t *state) {
  asset_render(state->menu_bg);
  asset_render(list_get(state->villain_chooser, state->villain_type));
  for (size_t i = 0; i < 2; i++) {
    asset_render(list_get(state->settings_buttons, i));
  }
  asset_render(state->instructions);
  asset_render(state->home_button);
}

void restart_game(state_t *state) {
  if (state->game_state == RACE || state->game_state == PAUSE) {
    race_free(state);
    state->game_state = MENU;
  } else if (state->game_state == SETTINGS) {
    settings_free(state);
    state->game_state = MENU;
  }
  state->menu_bg = asset_make_image(MENU_BACKGROUND_PATH, WINDOW);
  state->car_options = list_init(NUM_CARS, (free_func_t)asset_destroy);
  state->car_stats = list_init(NUM_CARS, (free_func_t)asset_destroy);
  state->car_type = F1;
  state->logo = asset_make_image(GAME_LOGO_PATH, GAME_LOGO);

  // Load audio files
  state->menu_music = Mix_LoadWAV(MENU_MUSIC);
  state->game_music = Mix_LoadWAV(GAME_MUSIC);
  state->star_music = Mix_LoadWAV(STAR_MUSIC);
  Mix_PlayChannel(0, state->menu_music, -1);
  Mix_Volume(-1, MIX_MAX_VOLUME);

  for (size_t i = 0; i < NUM_CARS; i++) {
    asset_t *car_img = asset_make_image(get_car_img_path(i), CAR_BOX);
    list_add(state->car_options, car_img);
    asset_t *car_stat = asset_make_image(CAR_STAT_PATHS[i], CAR_STATS_BOX);
    list_add(state->car_stats, car_stat);
  }

  state->menu_buttons = list_init(NUM_MENU_BUTTONS, (free_func_t)asset_destroy);
  create_menu_buttons(state);
}

void show_race(state_t *state, double dt) {
  state->time += dt;
  power_up_info_t info = car_get_powerup_state(state->car);
  if (info.stun > 0) {
    body_set_rotation(state->car,
                      body_get_rotation(state->car) + dt * STUN_ROT_SPEED);
  }
  info.fast -= dt;
  info.immune -= dt;
  info.reverse -= dt;
  info.stun -= dt;
  if (info.immune <= 0 && info.immune > -dt) {
    Mix_HaltChannel(0);
    Mix_PlayChannel(0, state->game_music, -1);
  }
  power_up_info_t villain_info = car_get_powerup_state(state->villain);
  if (villain_info.stun > 0) {
    body_set_rotation(state->villain,
                      body_get_rotation(state->villain) + dt * STUN_ROT_SPEED);
  }
  villain_info.fast -= dt;
  villain_info.immune -= dt;
  villain_info.reverse -= dt;
  villain_info.stun -= dt;
  car_set_powerup_state(state->car, info);
  scene_tick(state->scene, dt);
  scene_center_body(state->scene, state->car, SPAWN_POS);
  update_shell(state->car, dt);
  update_mini_map(state);
  update_arrow(state);
  fix_villain_path(state->villain);
  size_t size = list_size(state->body_assets);
  for (ssize_t i = 0; i < size; i++) {
    asset_t *curr = list_get(state->body_assets, i);
    body_t *body = asset_get_body(curr);
    if (body == NULL) {
      list_remove(state->body_assets, i);
      i--;
      size--;
    } else {
      asset_render(curr);
    }
  }
  size = list_size(state->boxes);
  for (size_t i = 0; i < size; i++) {
    asset_t *box = list_get(state->boxes, i);
    if (update_box(box, dt)) {
      asset_render(box);
    }
  }
  size = list_size(state->shells);
  for (ssize_t i = 0; i < size; i++) {
    asset_t *curr = list_get(state->shells, i);
    body_t *body = asset_get_body(curr);
    if (body == NULL) {
      list_remove(state->shells, i);
      i--;
      size--;
    } else {
      asset_render(curr);
    }
  }
  car_respawn(state->car);
  handle_checkpoint_state(state, dt);
  asset_render(state->mini_map);
  asset_render(state->mini_car);
  asset_render(state->mini_villain);
  asset_t *time_asset = make_time_asset(state->time);
  asset_render(time_asset);
  asset_destroy(time_asset);
  power_up_type_t power = car_get_powerup_state(state->car).power_up;
  if (power > 0) {
    asset_t *item = item_asset(power);
    asset_render(item);
    asset_destroy(item);
  }
  asset_render(state->pause_button);
  if (car_get_laps_done(state->car) == NO_LAPS) {
    printf("Race Over!\n");
    printf("Your best time was %f \n", state->best_time);
    restart_game(state);
    return;
  }
  asset_render(list_get(state->lap_numbers, car_get_laps_done(state->car)));
}

void pause_game(state_t *state) {
  show_race(state, 0);
  asset_render(state->home_button);
  asset_render(state->instructions);
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  sdl_clear();
  switch (state->game_state) {
  case MENU:
    show_menu(state);
    break;
  case RACE:
    show_race(state, dt);
    break;
  case SETTINGS:
    show_settings(state);
    break;
  case PAUSE:
    pause_game(state);
    break;
  default:
    assert(false && "invalid state");
  }
  sdl_show();

  return false;
}

void emscripten_free(state_t *state) {
  Mix_FreeChunk(state->menu_music);
  Mix_FreeChunk(state->game_music);
  Mix_FreeChunk(state->star_music);
  Mix_Quit();
  list_free(state->body_assets);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}
