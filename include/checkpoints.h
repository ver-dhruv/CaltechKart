#ifndef __CHECKPOINTS_H__
#define __CHECKPOINTS_H__

#include "asset.h"
#include "body.h"
#include "list.h"
#include "vector.h"

typedef struct checkpoint_state checkpoint_state_t;

typedef struct checkpoint_info checkpoint_info_t;

/**
 * Function to create a list of checkpoints between the given in_wall and
 * out_wall Requires that in_wall and out_wall are the same length
 * Accepts a start state which indicates the start and finish line.
 * Accepts an offset for the start index relative to the list
 */
list_t *make_checkpoints(list_t *in_wall, list_t *out_wall, vector_t start_i,
                         vector_t start_o, size_t start_offset);

/**
 * Function to get the index of a checkpoint body
 */
size_t get_checkpoint_idx(body_t *checkpoint);

/**
 * Function to initialize the checkpoint state from a list of checkpoints
 * Returns a pointer to initialized checkpoint state struct
 */
checkpoint_state_t *checkpoint_state_init(list_t *checkpoints);

/**
 * Function to reset the checkpoint state to the settings as they would
 * be at initialization
 */
void reset_checkpoint_state(checkpoint_state_t *checkpoint_state);

/**
 * Function to free the memory allocated for checkpoint state.
 */
void checkpoint_state_free(checkpoint_state_t *checkpoint_state);

/**
 * Gets checkpoint wrong way time
 */
double get_wrong_way_time(checkpoint_state_t *checkpoint_state);

/**
 * Sets state wrong way time
 */
void set_wrong_way_time(checkpoint_state_t *checkpoint_state, double time);

/**
 * Function to get the index of the current checkpoint.
 */
size_t get_current_checkpoint(checkpoint_state_t *checkpoint_state);

/**
 * Function to get the index of the furthest checkpoint reached.
 */
size_t get_furthest_checkpoint(checkpoint_state_t *checkpoint_state);

/**
 * Function to check if a lap is over.
 */
bool get_lap_over(checkpoint_state_t *checkpoint_state);

/**
 * Function to determine if the car is going the wrong way.
 */
bool get_wrong_way(body_t *car, checkpoint_state_t *checkpoint_state);

/**
 * Function to get the list of checkpoints.
 */
list_t *get_checkpoints(checkpoint_state_t *checkpoint_state);

/**
 * Function to get the right way direction vector from the current to
 * the next vector
 */
vector_t get_right_way(checkpoint_state_t *checkpoint_state);

/**
 * Function to get the right way direction vector from the current poisition
 * of the car to the next centroid of the checkpoint
 */
vector_t get_right_way_from_position(checkpoint_state_t *checkpoint_state,
                                     body_t *car);

/**
 * Function to handle collision of a car with a checkpoint.
 * Aux should be the checkpoint state of game which is updated after
 * colision
 */
void checkpoint_collision(body_t *car, body_t *checkpoint, vector_t axis,
                          void *aux, double force_const);

#endif // #ifndef __CHECKPOINT_H__