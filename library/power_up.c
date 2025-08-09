#include "power_up.h"
#include "asset.h"
#include "body.h"
#include "car.h"
#include "forces.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
// 120 by 60
const double BOX_SIZE = 30.0;
const double SHELL_SIZE = 20.0;
const char *BOX_PATH = "assets/box.png";
const char *SHELL_PATH = "assets/shell.png";
const char *SHROOM_PATH = "assets/mushroom.png";
const char *STAR_PATH = "assets/star.png";
const char *BOOST_PATH = "assets/boost_pad.png";
const char *SCRAMBLE_PATH = "assets/reverse_controls.png";
const SDL_Rect ITEM_BOUNDING_BOX = {900, 400, 100, 100};
const double SHELL_DISTANCE = 50;
const double SHELL_MASS = 60; // any non-zero finite value works
const double BOX_COOLDOWN = 10.0;
const size_t POSSIBLE_ITEMS = 6;
const double STUN_COOLDOWN = 2.0;
const double STUN_DURATION = 3.0;
const double BOOST_DURATION = 3.0;
const double BOOST_SIZE = 60.0;
double SPEED_MULTIPLIER = 1.5;
const double CAR_EL = 0.8;

struct shell_item_info {
  vector_t *vector;
  asset_t *asset;
};

struct box_item_info {
  double *time;
  asset_t *asset;
};

void shell_info_free(shell_item_info_t *info) {
  free(info->vector); // asset must be destroyed somewhere else
  free(info);
}

void box_info_free(box_item_info_t *info) {
  free(info->time); // asset must be destroyed somewhere else
  free(info);
}

asset_t *item_asset(power_up_type_t power) {
  const char *filepath = SHELL_PATH;
  switch (power) {
  case SHELL: {
    filepath = SHELL_PATH;
    break;
  }
  case REVERSE: {
    filepath = SCRAMBLE_PATH;
    break;
  }
  case BOOST: {
    filepath = BOOST_PATH;
    break;
  }
  case SHROOM: {
    filepath = SHROOM_PATH;
    break;
  }
  case STAR: {
    filepath = STAR_PATH;
    break;
  }
  case FAKE: {
    filepath = BOX_PATH;
    break;
  }
  default: {
    break;
  }
  }
  return asset_make_image(filepath, ITEM_BOUNDING_BOX);
}

asset_t *make_box(vector_t center) {
  box_item_info_t *info = malloc(sizeof(box_item_info_t));
  assert(info != NULL);
  info->time = malloc(sizeof(double));
  assert(info->time != NULL);
  *(info->time) = 0.0;
  info->asset = NULL;
  list_t *points = make_rectangle(center, BOX_SIZE, BOX_SIZE);
  body_t *body = body_init_with_info(points, INFINITY, get_blue(), info,
                                     (free_func_t)box_info_free);
  info->asset = asset_make_image_with_body(BOX_PATH, body);
  return info->asset;
}

bool update_box(asset_t *box, double dt) {
  body_t *body = asset_get_body(box);
  box_item_info_t *info = body_get_info(body);
  *(info->time) -= dt;
  return *(info->time) < 0;
}

asset_t *make_shell(vector_t center, double theta, double ang_vel) {
  shell_item_info_t *info = malloc(sizeof(shell_item_info_t));
  assert(info != NULL);
  info->vector = malloc(sizeof(vector_t));
  assert(info->vector != NULL);
  *(info->vector) = (vector_t){theta, ang_vel};
  info->asset = NULL;
  list_t *points = make_rectangle(center, SHELL_SIZE, SHELL_SIZE);
  body_t *body = body_init_with_info(points, SHELL_MASS, get_blue(), info,
                                     (free_func_t)shell_info_free);
  info->asset = asset_make_image_with_body(SHELL_PATH, body);
  return info->asset;
}

asset_t *make_boost(body_t *car) {
  list_t *points =
      make_rectangle(body_get_centroid(car), BOOST_SIZE, BOOST_SIZE);
  body_t *body = body_init(points, INFINITY, get_blue());
  asset_t *asset = asset_make_rotatable_image_with_body(BOOST_PATH, body);
  body_set_rotation(body,
                    body_get_rotation(car) - M_PI / 2); // car asset faces down
  return asset;
  ;
}

bool car_has_shell(body_t *car) {
  power_up_info_t info = car_get_powerup_state(car);
  if (info.shell == NULL || body_is_removed(info.shell)) {
    info.shell = NULL;
  }
  return info.shell != NULL;
}

double get_speed_multiplier() { return SPEED_MULTIPLIER; }

void update_shell(body_t *car, double dt) {
  if (car_has_shell(car)) {
    body_t *shell = car_get_powerup_state(car).shell;
    shell_item_info_t *info = body_get_info(shell);
    vector_t *vec = info->vector;
    vec->x += vec->y * dt; // theta += vdt
    vector_t offset = vec_rotate((vector_t){0, SHELL_DISTANCE}, vec->x);
    body_set_centroid(shell, vec_add(body_get_centroid(car), offset));
    vector_t vel = vec_rotate(offset, M_PI / 2);
    vel = vec_multiply(vec->y, vel);
    body_set_velocity(shell, vec_add(vel, body_get_velocity(car)));
  }
}

void flip_shell(body_t *shell) {
  shell_item_info_t *info = body_get_info(shell);
  info->vector->y *= -1; // flipping angular velocity
}

void box_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                           void *aux, double force_const) {
  box_item_info_t *box_info = body_get_info(body2);
  double *time = box_info->time;
  if (*time < 0.0) {
    bool *switches = aux;
    size_t total = 0;
    for (size_t i = 0; i < POSSIBLE_ITEMS; i++) {
      if (switches[i]) {
        total++;
      }
    }
    size_t count = rand() % total + 1;
    power_up_type_t power = NONE;
    while (count > 0) {
      if (switches[power]) {
        count--;
      }
      power++;
    }
    power_up_info_t info = car_get_powerup_state(body1);
    if (info.power_up == NONE) {
      info.power_up = power;
      car_set_powerup_state(body1, info);
    }
    *time = BOX_COOLDOWN;
  }
}

void create_box_collision(scene_t *scene, body_t *body1, body_t *body2,
                          bool *switches) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)box_collision_handler, switches, 0);
}

void stun_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                            void *aux, double force_const) {
  power_up_info_t info = car_get_powerup_state(body1);
  if (info.stun < -STUN_COOLDOWN && info.immune < 0) {
    info.stun = STUN_DURATION;
    car_set_powerup_state(body1, info);
    body_set_velocity(body1, VEC_ZERO);
  }
  if (force_const > 0) {
    if (force_const > 2) {
      shell_item_info_t *shell_info = body_get_info(body2);
      asset_set_body(shell_info->asset, NULL);
    } else {
      box_item_info_t *box_info = body_get_info(body2);
      asset_set_body(box_info->asset, NULL);
    }
    body_remove(body2);
  }
}

void create_stun_collision(scene_t *scene, body_t *body1, body_t *body2,
                           double remove) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)stun_collision_handler, NULL, remove);
}

void boost_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                             void *aux, double force_const) {
  power_up_info_t info = car_get_powerup_state(body1);
  info.fast = fmax(info.fast, BOOST_DURATION);
  car_set_powerup_state(body1, info);
  vector_t vel = body_get_velocity(body1);
  if (vec_get_length(vel) == 0) {
    double theta = body_get_rotation(body1);
    vel.x = sin(theta);
    vel.y = -cos(theta);
  }
  vel = vec_multiply(
      SPEED_MULTIPLIER * car_get_top_speed(body1) / vec_get_length(vel), vel);
  body_set_velocity(body1, vel);
}

void create_boost_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)boost_collision_handler, NULL, 0);
}

void car_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                           void *aux, double force_const) {
  power_up_info_t info1 = car_get_powerup_state(body1);
  if (info1.immune >= 0) {
    stun_collision_handler(body2, body1, axis, NULL, -1.0);
  } else {
    physics_collision_handler(body1, body2, axis, NULL, force_const);
  }
}

void create_car_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)car_collision_handler, NULL, CAR_EL);
}

void shell_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                             void *aux, double force_const) {
  shell_item_info_t *info1 = body_get_info(body1);
  asset_set_body(info1->asset, NULL);
  shell_item_info_t *info2 = body_get_info(body2);
  asset_set_body(info2->asset, NULL);
  body_remove(body1);
  body_remove(body2);
}

void create_shell_collision(scene_t *scene, body_t *body1, body_t *body2) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)shell_collision_handler, NULL, 0);
}

void permute(size_t *elements, size_t size) {
  for (size_t i = size; i > 1; i--) {
    size_t j = (size_t)(rand() % i);
    size_t aux = elements[j];
    if (aux > 3)
      printf("FUCKED UP %zu\n", j);
    elements[j] = elements[i - 1];
    elements[i - 1] = aux;
  }
}