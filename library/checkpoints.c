#include "checkpoints.h"
#include "asset.h"
#include "background.h"
#include "body.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const size_t CHEKCPOINT_FREQ = 1; // no. checkpoint every 100 pixels
const double CHECKPOINT_MASS = INFINITY;
const rgb_color_t CHECKPOINT_COLOR = (rgb_color_t){0, 0, 0};
const vector_t CHECKPOINT_OFF = (vector_t){0, 5};
const double WRONG_WAY_TOL = -0.173;

typedef struct checkpoint_state {
  size_t current;
  size_t furthest;
  bool lap_over;
  vector_t right_way;
  list_t *checkpoints;
  double wrong_way_time;
} checkpoint_state_t;

typedef struct checkpoint_info {
  size_t idx;
} checkpoint_info_t;

/**
 * Helper function to make a checkpoint given a vector representing
 * the point on inside wall, on outside wall and the index for the point
 */
body_t *make_checkpoint(vector_t in_pos, vector_t out_pos, size_t idx) {
  vector_t *in = malloc(sizeof(vector_t));
  assert(in != NULL);
  *in = in_pos;

  vector_t *in_2 = malloc(sizeof(vector_t));
  assert(in_2 != NULL);
  *in_2 = vec_add(in_pos, CHECKPOINT_OFF);

  vector_t *out = malloc(sizeof(vector_t));
  assert(out != NULL);
  *out = out_pos;

  vector_t *out_2 = malloc(sizeof(vector_t));
  assert(out_2 != NULL);
  *out_2 = vec_add(out_pos, CHECKPOINT_OFF);

  list_t *shape = list_init(4, free);
  list_add(shape, in);
  list_add(shape, out);
  list_add(shape, out_2);
  list_add(shape, in_2);

  checkpoint_info_t *checkpoint_info = malloc(sizeof(checkpoint_info_t));
  assert(checkpoint_info != NULL);
  checkpoint_info->idx = idx;

  return body_init_with_info(shape, CHECKPOINT_MASS, CHECKPOINT_COLOR,
                             checkpoint_info, free);
}

list_t *make_checkpoints(list_t *in_wall, list_t *out_wall, vector_t start_i,
                         vector_t start_o, size_t start_offset) {
  assert(list_size(in_wall) == list_size(out_wall));
  list_t *checkpoints = list_init(CHEKCPOINT_FREQ * list_size(in_wall),
                                  NULL); // will be cleared in scene free

  body_t *start = make_checkpoint(start_i, start_o, 0);
  list_add(checkpoints, start);
  size_t no_walls = list_size(in_wall);
  size_t idx = 1;

  for (size_t i = start_offset; i < no_walls + start_offset; i++, idx++) {
    vector_t in = *(vector_t *)list_get(in_wall, i % no_walls);
    vector_t out = *(vector_t *)list_get(out_wall, i % no_walls);
    body_t *checkpoint = make_checkpoint(in, out, idx);
    list_add(checkpoints, checkpoint);
  }

  return checkpoints;
}

size_t get_checkpoint_idx(body_t *checkpoint) {
  checkpoint_info_t *info = body_get_info(checkpoint);
  return info->idx;
}

/**
 * Helper function to get the direction vector between two checkpoints
 * at given indices {from idx1 to idx 2}
 */
vector_t get_direction_vector(list_t *checkpoints, size_t idx1, size_t idx2) {
  assert(idx2 > idx1 && idx2 <= list_size(checkpoints));
  idx2 = idx2 % list_size(checkpoints);
  body_t *cpoint_1 = list_get(checkpoints, idx1);
  body_t *cpoint_2 = list_get(checkpoints, idx2);
  list_t *points_1 = polygon_get_points(body_get_polygon(cpoint_1));
  list_t *points_2 = polygon_get_points(body_get_polygon(cpoint_2));
  vector_t vec_1_cent =
      vec_multiply(0.5, vec_add(*(vector_t *)list_get(points_1, 0),
                                *(vector_t *)list_get(points_1, 1)));
  vector_t vec_2_cent =
      vec_multiply(0.5, vec_add(*(vector_t *)list_get(points_2, 0),
                                *(vector_t *)list_get(points_2, 1)));

  vector_t way = vec_subtract(vec_2_cent, vec_1_cent);
  return vec_multiply(1 / vec_get_length(way), way);
}

checkpoint_state_t *checkpoint_state_init(list_t *checkpoints) {
  checkpoint_state_t *checkpoint_state = malloc(sizeof(checkpoint_state_t));
  assert(checkpoint_state != NULL);
  checkpoint_state->current = 0;
  checkpoint_state->furthest = 0;
  checkpoint_state->lap_over = false;
  checkpoint_state->right_way = get_direction_vector(checkpoints, 0, 1);
  checkpoint_state->checkpoints = checkpoints;
  return checkpoint_state;
}

void reset_checkpoint_state(checkpoint_state_t *checkpoint_state) {
  checkpoint_state->current = 0;
  checkpoint_state->furthest = 0;
  checkpoint_state->lap_over = false;
  checkpoint_state->right_way =
      get_direction_vector(checkpoint_state->checkpoints, 0, 1);
}

void checkpoint_state_free(checkpoint_state_t *checkpoint_state) {
  list_free(checkpoint_state->checkpoints);
  free(checkpoint_state);
}

double get_wrong_way_time(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->wrong_way_time;
}

void set_wrong_way_time(checkpoint_state_t *checkpoint_state, double time) {
  checkpoint_state->wrong_way_time = time;
}

size_t get_current_checkpoint(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->current;
}

size_t get_furthest_checkpoint(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->furthest;
}

bool get_lap_over(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->lap_over;
}

bool get_wrong_way(body_t *car, checkpoint_state_t *checkpoint_state) {
  return vec_dot(checkpoint_state->right_way, body_get_velocity(car)) <
         WRONG_WAY_TOL * vec_get_length(body_get_velocity(
                             car)); // wrong way is an 160 degree window
}

list_t *get_checkpoints(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->checkpoints;
}

vector_t get_right_way(checkpoint_state_t *checkpoint_state) {
  return checkpoint_state->right_way;
}

vector_t get_right_way_from_position(checkpoint_state_t *checkpoint_state,
                                     body_t *car) {
  list_t *next_points = polygon_get_points(
      body_get_polygon(list_get(checkpoint_state->checkpoints,
                                (checkpoint_state->current + 1) %
                                    list_size(checkpoint_state->checkpoints))));
  vector_t next_p =
      vec_multiply(0.5, vec_add(*(vector_t *)list_get(next_points, 0),
                                *(vector_t *)list_get(next_points, 1)));

  vector_t right_way = vec_subtract(next_p, body_get_centroid(car));
  double magnitude = vec_get_length(right_way);
  if (magnitude != 0) {
    right_way = vec_multiply(1 / magnitude, right_way);
  }
  return right_way;
}

void checkpoint_collision(body_t *car, body_t *checkpoint, vector_t axis,
                          void *aux, double force_const) {
  checkpoint_state_t *checkpoint_state = (checkpoint_state_t *)aux;
  checkpoint_info_t *info = body_get_info(checkpoint);
  size_t current = info->idx;
  checkpoint_state->current = current;

  if (current == (checkpoint_state->furthest + 1)) {
    checkpoint_state->furthest = current;
  }

  if (checkpoint_state->furthest ==
          (list_size(checkpoint_state->checkpoints) - 1) &&
      current == 0) {
    checkpoint_state->lap_over = true;
  }
  checkpoint_state->right_way =
      get_direction_vector(checkpoint_state->checkpoints, current, current + 1);
}