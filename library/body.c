#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "vector.h"

struct body {
  polygon_t *poly;

  double mass;

  vector_t force;
  vector_t impulse;
  bool removed;
  double rotation;

  void *info;
  free_func_t info_freer;
};

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(mass >= 0);
  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);
  body->poly = polygon_init(shape, VEC_ZERO, 0, color.r, color.g, color.b);
  body->mass = mass;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->removed = false;
  body->rotation = 0;
  body->info = info;
  body->info_freer = info_freer;
  return body;
}

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

list_t *body_get_shape(body_t *body) {
  list_t *points = polygon_get_points(body->poly);
  list_t *shape = list_init(list_size(points), free);
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *point = list_get(points, i);
    vector_t *copy = malloc(sizeof(vector_t));
    assert(copy != NULL);
    copy->x = point->x;
    copy->y = point->y;
    list_add(shape, copy);
  }
  return shape;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  vector_t velocity;
  velocity.x = polygon_get_velocity(body->poly)->x;
  velocity.y = polygon_get_velocity(body->poly)->y;
  return velocity;
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_translate(body->poly, vec_subtract(x, body_get_centroid(body)));
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) { return body->rotation; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->poly, angle - body_get_rotation(body),
                 body_get_centroid(body));
  body->rotation = angle;
}

void body_free(body_t *body) {
  polygon_free(body->poly);
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  free(body);
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

void body_tick(body_t *body, double dt) {
  vector_t old_vel = body_get_velocity(body);
  vector_t dv1 = vec_multiply(dt / body->mass, body->force);
  vector_t dv2 = vec_multiply(1 / body->mass, body->impulse);
  vector_t dv = vec_add(dv1, dv2);
  vector_t new_vel = vec_add(old_vel, dv);
  vector_t mid_vel = vec_multiply(0.5, vec_add(old_vel, new_vel));
  vector_t displacement = integrate_simpson(old_vel, mid_vel, new_vel, dt);
  polygon_translate(body->poly, displacement);
  body_set_velocity(body, new_vel);
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(force, body->force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(impulse, body->impulse);
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}