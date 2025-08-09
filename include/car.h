#ifndef __CAR_H__
#define __CAR_H__

#include "asset.h"
#include "body.h"
#include "checkpoints.h"
#include "list.h"
#include "power_up.h"
#include <stdint.h>
#include <stdlib.h>

typedef enum {
  F1,
  GOLF_CART,
  PICKUP,
} car_type_t;

typedef struct car_info car_info_t;

typedef struct AI_info AI_info_t;

typedef struct ghost_info ghost_info_t;

typedef enum { EASY_AI, MEDIUM_AI, HARD_AI, GHOST } villain_type_t;

typedef struct power_up_info {
  size_t power_up;
  double immune;
  double stun;
  double reverse;
  double fast;
  body_t *shell;
} power_up_info_t;

/**
 * A function that initializes a car_info_t struct with the given type and
 * physical values. Returns a pointer to the initialized car info struct.
 */
car_info_t *make_car_info(car_type_t type, double friction, double top_speed,
                          double acceleration);

/**
 * A function that returns the checkpoint_state of the car.
 * Pointer to car body passed in as a parameter.
 */
checkpoint_state_t *car_get_checkpoint_state(body_t *car);

/**
 * Changes the cars top speed. Use for villain only.
 */
void change_top_speed(body_t *car, double new_top_speed);

/**
 * A function that sets the checkpoint_state of the car.
 * Pointer to car body passed in as a parameter.
 */
void car_set_checkpoint_state(body_t *car,
                              checkpoint_state_t *checkpoint_state);

/**
 * A function that gets the laps done of the car.
 * Pointer to car body passed in as a parameter.
 */
uint8_t car_get_laps_done(body_t *car);

/**
 * A function that sets the laps done of the car.
 * Pointer to car body passed in as a parameter.
 */
void car_set_laps_done(body_t *car, uint8_t laps_done);

/**
 * A function that returns the powerup state of the car.
 * Pointer to car body passed in as a parameter.
 */
power_up_info_t car_get_powerup_state(body_t *car);

/**
 * A function that sets the powerup state of the car.
 * Pointer to car body passed in as a parameter.
 */
void car_set_powerup_state(body_t *car, power_up_info_t power_up_info);

/**
 * A function that returns the type of the car.
 * Pointer to car body passed in as a parameter.
 */
car_type_t car_get_type(body_t *car);

/**
 * A function that returns the friction of the car.
 * Pointer to car body passed in as a parameter.
 */
double car_get_friction(body_t *car);

/**
 * A function that returns the acceleration of the car.
 * Pointer to car body passed in as a parameter.
 */
double car_get_acceleration(body_t *car);

/**
 * A function that returns the top_speed of the car.
 * Pointer to car body passed in as a parameter.
 */
double car_get_top_speed(body_t *car);

/**
 * Function to call in main that returns a body to the last
 * checkpoint if it accidentally leaves the track
 */
void car_respawn(body_t *car);

/**
 * A function that initializes a car body with the given type.
 * Returns a pointer to the initialized car body.
 */
body_t *make_car(car_type_t type);

/**
 * A function that initializes a image asset from the given car body.
 * Choses the image based on the car type.
 * Returns a pointer to the initialized image asset.
 */
asset_t *make_car_image(body_t *car);

/**
 * A function that returns the filepath for a car of a given type
 */
const char *get_car_img_path(car_type_t type);

/**
 * A function that takes in a car an returns a mini version of it
 * for the mini-map.
 * Returns a pointer to the initialized image asset.
 */
asset_t *make_mini_car(car_type_t type);

/**
 * A function that returns the body for the mini villain asset, given the
 * villain type.
 */
asset_t *make_mini_villain(villain_type_t type);

#endif // #ifndef __CAR_H__