#ifndef __POWER_UP_H__
#define __POWER_UP_H__

#include "asset.h"
#include "body.h"
#include "list.h"

typedef enum {
  NONE,
  SHELL,
  REVERSE,
  BOOST,
  SHROOM,
  STAR,
  FAKE
} power_up_type_t;

typedef struct shell_item_info shell_item_info_t;

/**
 * Frees the info of a shell body
 *
 * @param info the information to free
 */
void shell_info_free(shell_item_info_t *info);

typedef struct box_item_info box_item_info_t;

/**
 * Frees the info of a box body
 *
 * @param info the information to free
 */
void box_info_free(box_item_info_t *info);

/**
 * Makes an asset to display the player's item.
 *
 * @param power the player's item
 * @return the asset
 */
asset_t *item_asset(power_up_type_t power);

/**
 * Makes a box asset with the given center.
 *
 * @param center the center for the box
 * @return the box asset
 */
asset_t *make_box(vector_t center);

/**
 * Updates the information of a box, and returns whether or not it should be
 * rendered
 * @param box the box
 * @param dt the time ellapsed
 * @returns whether it should be rendered
 */
bool update_box(asset_t *box, double dt);

/**
 * Makes a shell asset with the given center and angular velocity.
 *
 * @param center the center for the shell
 * @param theta the angular position
 * @param ang_vel the angular velocity
 * @return the shell asset
 */
asset_t *make_shell(vector_t center, double theta, double ang_vel);

/**
 * Makes a boost asset with the car's center and direction.
 *
 * @param car the car placing the boost
 * @return the boost asset
 */
asset_t *make_boost(body_t *car);

/**
 * Checks if the car has a shell currently rotating around it
 *
 * @param car the car
 * @return true if it has a shell, false if it does not
 */
bool car_has_shell(body_t *car);

/**
 * Returns the speed multiplier from mushrooms and boost pads
 *
 * @return the speed multiplier
 */
double get_speed_multiplier();

/**
 * Updates the position of the shell rotating around the car, if any.
 *
 * @param car the car
 * @param dt time elapsed
 */
void update_shell(body_t *car, double dt);

/**
 * Flips the angular velocity of the shell
 *
 * @param shell the shell
 */
void flip_shell(body_t *shell);

/**
 * Collision handler for the boxes. Gives the car (body1) an item.
 */
void box_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                           void *aux, double force_const);

/**
 * Adds a force creator to a scene that gives the first body an item
 * when it collides with the second.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body, corresponding to the car
 * @param body2 the second body, corresponding to the power up box
 * @param switches the power ups selected by the user to include in the game
 */
void create_box_collision(scene_t *scene, body_t *body1, body_t *body2,
                          bool *switches);

/**
 * Collision handler for stun. Updates the stun attribute of the first body
 * (the car).
 */
void stun_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                            void *aux, double force_const);

/**
 * Adds a force creator to a scene that stuns the first body if its stun
 * cooldown is over when it collides with the second.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body; the body to be stunned
 * @param body2 the second body
 * @param remove a double indicating if the second body is a car (under 0),
 * shell (between 0 and 2), or a box (over 2)
 */
void create_stun_collision(scene_t *scene, body_t *body1, body_t *body2,
                           double remove);

/**
 * Collision handler for the boost panes. Gives the car (body1) a speed boost.
 */
void boost_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                             void *aux, double force_const);

/**
 * Adds a force creator to a scene that gives the first body a speed boost
 * when it collides with the second.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body, corresponding to the car
 * @param body2 the second body, corresponding to boost panel
 */
void create_boost_collision(scene_t *scene, body_t *body1, body_t *body2);

/**
 * Collision handler for the boost panes. Gives the car (body1) a speed boost.
 */
void car_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                           void *aux, double force_const);

/**
 * Adds a force creator to a scene that resolves the collision between two cars.
 * The collision is a normal physics collision if both cars are immune or
 * neither is, but if only one car is immune, the collision should stun the
 * other.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_car_collision(scene_t *scene, body_t *body1, body_t *body2);

/**
 * Collision handler for the collision between two shells. Removes both bodies
 * and sets the body of their assets to NULL, to indicate they should no longer
 * be rendered.
 */
void shell_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                             void *aux, double force_const);

/**
 * Adds a force creator to a scene that resolves the collision between two
 * shells.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_shell_collision(scene_t *scene, body_t *body1, body_t *body2);

/**
 * Permutes the elements of an array
 *
 * @param elements the array of elements
 * @param size the size of the array
 */
void permute(size_t *elements, size_t size);

#endif // #ifndef __POWER_UP_H__