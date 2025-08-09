#include "car.h"
#include "asset.h"
#include "body.h"
#include "checkpoints.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

const double CAR_WIDTH = 30.0;
const double CAR_HEIGHT = 60.0;

const double MINI_CAR_WIDTH = 18.0;
const double MINI_CAR_HEIGHT = 36.0;
const double MINI_AI_WIDTH = 23.0;
const double MINI_AI_HEIGHT = 30.0;
const double MINI_GHOST_WIDTH = 25.0;
const double MINI_GHOST_HEIGHT = 25.0;
const double MINI_CAR_MASS = 0.0;

const char *F1_IMG = "assets/f1_car.png";
const char *GOLF_CART_IMG = "assets/golf_cart.png";
const char *PICKUP_IMG = "assets/pickup_truck.png";

const char *AI_IMG = "assets/ai_minimap.png";
const char *GHOST_IMG = "assets/ghost.png";

const double F1_MASS = 180.0;
const double F1_FRICTION = 5;
const double F1_TOP_SPEED = 250.0;
const double F1_ACCELERATION = 15.0;

const double GOLF_CART_MASS = 20.0;
const double GOLF_CART_FRICTION = 2;
const double GOLF_CART_TOP_SPEED = 200.0;
const double GOLF_CART_ACCELERATION = 30.0;

const double PICKUP_MASS = 500.0;
const double PICKUP_FRICTION = 3.5;
const double PICKUP_TOP_SPEED = 150.0;
const double PICKUP_ACCELERATION = 7.5;

const double ZERO_TOL = 0.01;
const double WRONG_WAY_TIME_TOL = 5.0;

typedef struct car_info {
  car_type_t type;
  double friction;
  double top_speed;
  double acceleration;
  power_up_info_t power_up_state;
  checkpoint_state_t *checkpoint_state;
  uint8_t laps_done;
} car_info_t;

car_info_t *make_car_info(car_type_t type, double friction, double top_speed,
                          double acceleration) {
  car_info_t *info = malloc(sizeof(car_info_t));
  assert(info != NULL);
  info->type = type;
  info->friction = friction;
  info->top_speed = top_speed;
  info->acceleration = acceleration;
  info->laps_done = 0;
  info->power_up_state = (power_up_info_t){0, 0, 0, 0, 0};
  info->checkpoint_state = NULL;
  return info;
}

void free_car_info(car_info_t *car_info) {
  checkpoint_state_free(car_info->checkpoint_state);
  free(car_info);
}

checkpoint_state_t *car_get_checkpoint_state(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->checkpoint_state;
}

void change_top_speed(body_t *car, double new_top_speed) {
  car_info_t *info = body_get_info(car);
  info->top_speed = new_top_speed;
}

uint8_t car_get_laps_done(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->laps_done;
}

void car_set_laps_done(body_t *car, uint8_t laps_done) {
  car_info_t *info = body_get_info(car);
  info->laps_done = laps_done;
}

void car_set_checkpoint_state(body_t *car,
                              checkpoint_state_t *checkpoint_state) {
  car_info_t *info = body_get_info(car);
  info->checkpoint_state = checkpoint_state;
}

power_up_info_t car_get_powerup_state(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->power_up_state;
}

void car_set_powerup_state(body_t *car, power_up_info_t power_up_info) {
  car_info_t *info = body_get_info(car);
  info->power_up_state = power_up_info;
}

car_type_t car_get_type(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->type;
}

double car_get_friction(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->friction;
}

double car_get_acceleration(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->acceleration;
}

double car_get_top_speed(body_t *car) {
  car_info_t *info = body_get_info(car);
  return info->top_speed;
}

/**
 * Helper function to check if car centre not in bound of line 1 and 2
 */
bool check_in_bounds(vector_t car_cent, vector_t in_1, vector_t in_2,
                     vector_t out_1, vector_t out_2, vector_t direction) {

  double x_min = fmin(fmin(in_1.x, in_2.x), fmin(out_1.x, out_2.x));
  double x_max = fmax(fmax(in_1.x, in_2.x), fmax(out_1.x, out_2.x));
  double y_min = fmin(fmin(in_1.y, in_2.y), fmin(out_1.y, out_2.y));
  double y_max = fmax(fmax(in_1.y, in_2.y), fmax(out_1.y, out_2.y));

  // Check if straight
  if ((-ZERO_TOL < direction.x && direction.x < ZERO_TOL) ||
      (-ZERO_TOL < direction.y && direction.y < ZERO_TOL)) {
    return x_min <= car_cent.x && car_cent.x <= x_max && y_min <= car_cent.y &&
           car_cent.y <= y_max;
  }

  // else its 45 degreess

  double cos_t = direction.x;
  double sin_t = direction.y;

  vector_t in_1_rot = (vector_t){.x = in_1.x * cos_t - in_1.y * sin_t,
                                 .y = in_1.x * sin_t + in_1.y * cos_t};
  vector_t in_2_rot = (vector_t){.x = in_2.x * cos_t - in_2.y * sin_t,
                                 .y = in_2.x * sin_t + in_2.y * cos_t};
  vector_t out_1_rot = (vector_t){.x = out_1.x * cos_t - out_1.y * sin_t,
                                  .y = out_1.x * sin_t + out_1.y * cos_t};
  vector_t out_2_rot = (vector_t){.x = out_2.x * cos_t - out_2.y * sin_t,
                                  .y = out_2.x * sin_t + out_2.y * cos_t};
  vector_t car_cent_rot =
      (vector_t){.x = car_cent.x * cos_t - car_cent.y * sin_t,
                 .y = car_cent.x * sin_t + car_cent.y * cos_t};

  x_min = fmin(fmin(in_1_rot.x, in_2_rot.x), fmin(out_1_rot.x, out_2_rot.x));
  x_max = fmax(fmax(in_1_rot.x, in_2_rot.x), fmax(out_1_rot.x, out_2_rot.x));
  y_min = fmin(fmin(in_1_rot.y, in_2_rot.y), fmin(out_1_rot.y, out_2_rot.y));
  y_max = fmax(fmax(in_1_rot.y, in_2_rot.y), fmax(out_1_rot.y, out_2_rot.y));

  return x_min <= car_cent_rot.x && car_cent_rot.x <= x_max &&
         y_min <= car_cent_rot.y && car_cent_rot.y <= y_max;
}

void car_respawn(body_t *car) {
  checkpoint_state_t *checkpoint_state = car_get_checkpoint_state(car);
  list_t *checkpoints = get_checkpoints(checkpoint_state);
  size_t curr = get_current_checkpoint(checkpoint_state);
  size_t next = (curr + 1) % list_size(checkpoints);
  body_t *cpoint_1 = list_get(checkpoints, curr);
  body_t *cpoint_2 = list_get(checkpoints, next);
  list_t *points_1 = polygon_get_points(body_get_polygon(cpoint_1));
  list_t *points_2 = polygon_get_points(body_get_polygon(cpoint_2));
  vector_t car_cent = body_get_centroid(car);
  vector_t in_1 = *(vector_t *)list_get(points_1, 0);
  vector_t out_1 = *(vector_t *)list_get(points_1, 1);
  vector_t in_2 = *(vector_t *)list_get(points_2, 0);
  vector_t out_2 = *(vector_t *)list_get(points_2, 1);
  vector_t turn_direction = get_right_way(checkpoint_state);
  if (get_wrong_way(car, checkpoint_state) &&
      get_wrong_way_time(checkpoint_state) < WRONG_WAY_TIME_TOL) {
    return;
  }
  if (!check_in_bounds(car_cent, in_1, in_2, out_1, out_2, turn_direction)) {
    printf("Stay On Track!!\n");
    body_set_velocity(car, VEC_ZERO);
    double theta = body_get_rotation(car);
    vector_t car_dir = {.x = sin(theta), .y = -cos(theta)};
    theta = theta + atan2(turn_direction.y, turn_direction.x) -
            atan2(car_dir.y, car_dir.x);
    body_set_rotation(car, theta);
    body_set_centroid(
        car, vec_multiply(0.5, vec_add(*(vector_t *)list_get(points_1, 1),
                                       *(vector_t *)list_get(points_1, 0))));
  }
  if (get_wrong_way_time(checkpoint_state) >= WRONG_WAY_TIME_TOL) {
    printf("Stay On Track!!\n");
    body_set_velocity(car, VEC_ZERO);
    double theta = body_get_rotation(car);
    vector_t car_dir = {.x = sin(theta), .y = -cos(theta)};
    theta = theta + atan2(turn_direction.y, turn_direction.x) -
            atan2(car_dir.y, car_dir.x);
    body_set_rotation(car, theta);
    body_set_centroid(
        car, vec_multiply(0.5, vec_add(*(vector_t *)list_get(points_1, 1),
                                       *(vector_t *)list_get(points_1, 0))));
  }
}

body_t *make_car(car_type_t type) {
  double mass = 0.0;
  double friction = 0.0;
  double acceleration = 0.0;
  double top_speed = 0.0;
  switch (type) {
  case F1:
    mass = F1_MASS;
    friction = F1_FRICTION;
    acceleration = F1_ACCELERATION;
    top_speed = F1_TOP_SPEED;
    break;
  case GOLF_CART:
    mass = GOLF_CART_MASS;
    friction = GOLF_CART_FRICTION;
    acceleration = GOLF_CART_ACCELERATION;
    top_speed = GOLF_CART_TOP_SPEED;
    break;
  case PICKUP:
    mass = PICKUP_MASS;
    friction = PICKUP_FRICTION;
    acceleration = PICKUP_ACCELERATION;
    top_speed = PICKUP_TOP_SPEED;
    break;
  default:
    assert(false && "Invalid Car Type");
    break;
  }
  list_t *shape = make_rectangle(VEC_ZERO, CAR_WIDTH, CAR_HEIGHT);
  car_info_t *info = make_car_info(type, friction, top_speed, acceleration);
  body_t *car = body_init_with_info(shape, mass, get_blue(), info,
                                    (free_func_t)free_car_info);
  return car;
}

const char *get_car_img_path(car_type_t type) {
  switch (type) {
  case F1:
    return F1_IMG;
  case GOLF_CART:
    return GOLF_CART_IMG;
  case PICKUP:
    return PICKUP_IMG;
  default:
    assert(false && "Invalid Car Type");
  }
}

asset_t *make_car_image(body_t *car) {
  const char *path = get_car_img_path(car_get_type(car));
  return asset_make_rotatable_image_with_body(path, car);
}

asset_t *make_mini_car(car_type_t car_type) {
  const char *path = get_car_img_path(car_type);
  list_t *shape = make_rectangle(VEC_ZERO, MINI_CAR_WIDTH, MINI_CAR_HEIGHT);
  body_t *mini_car = body_init(shape, MINI_CAR_MASS, get_blue());
  return asset_make_rotatable_image_with_body(path, mini_car);
}

asset_t *make_mini_villain(villain_type_t type) {
  switch (type) {
  case EASY_AI:
  case MEDIUM_AI:
  case HARD_AI: {
    list_t *shape = make_rectangle(VEC_ZERO, MINI_AI_WIDTH, MINI_AI_HEIGHT);
    body_t *body = body_init(shape, MINI_CAR_MASS, get_blue());
    return asset_make_rotatable_image_with_body(AI_IMG, body);
    break;
  }
  case GHOST: {
    list_t *shape =
        make_rectangle(VEC_ZERO, MINI_GHOST_WIDTH, MINI_GHOST_HEIGHT);
    body_t *body = body_init(shape, MINI_CAR_MASS, get_blue());
    return asset_make_rotatable_image_with_body(GHOST_IMG, body);
  }
  default:
    return NULL;
    break;
  }
}