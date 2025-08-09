#include "polygon.h"
#include "color.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct polygon {
  list_t *vertices;
  vector_t velocity;
  double rotation_speed;
  rgb_color_t *color;
  vector_t centroid;
} polygon_t;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon != NULL);
  polygon->vertices = points;
  polygon->velocity = initial_velocity;
  polygon->rotation_speed = rotation_speed;
  polygon->color = color_init(red, green, blue);
  polygon_set_center(polygon, polygon_centroid(polygon));
  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->vertices; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  vector_t translation = vec_multiply(time_elapsed, polygon->velocity);
  polygon_translate(polygon, translation);
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity.x = vel.x;
  polygon->velocity.y = vel.y;
}

void polygon_free(polygon_t *polygon) {
  list_free(polygon->vertices);
  color_free(polygon->color);
  free(polygon);
}

vector_t *polygon_get_velocity(polygon_t *polygon) {
  return &(polygon->velocity);
}

double polygon_area(polygon_t *polygon) {
  double area = 0.0;
  size_t len = list_size(polygon->vertices);
  size_t next = 0;

  for (size_t i = 0; i < len; i++) {
    next = (i + 1) % len;
    vector_t *current = list_get(polygon->vertices, i);
    vector_t *next_vertex = list_get(polygon->vertices, next);
    area += current->x * next_vertex->y;
    area -= current->y * next_vertex->x;
  }
  return fabs(area) / 2.0;
}

vector_t polygon_centroid(polygon_t *polygon) {
  size_t len = list_size(polygon->vertices);
  vector_t c = {0.0, 0.0};
  double area = polygon_area(polygon);
  size_t next = 0;
  vector_t *current;
  vector_t *next_vertex;

  for (size_t i = 0; i < len; i++) {
    next = (i + 1) % len;
    current = list_get(polygon->vertices, i);
    next_vertex = list_get(polygon->vertices, next);
    double step = (current->x * next_vertex->y) - (next_vertex->x * current->y);
    c.x += (current->x + next_vertex->x) * step;
    c.y += (current->y + next_vertex->y) * step;
  }
  return vec_multiply(1 / (6.0 * area), c);
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  size_t len = list_size(polygon->vertices);
  for (size_t i = 0; i < len; i++) {
    vector_t *vertex = list_get(polygon->vertices, i);
    vector_t trans = vec_add(*vertex, translation);
    vertex->x = trans.x;
    vertex->y = trans.y;
  }
  polygon->centroid = vec_add(polygon->centroid, translation);
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  size_t len = list_size(polygon->vertices);
  polygon_translate(polygon, vec_negate(point));
  for (size_t i = 0; i < len; i++) {
    vector_t *vertex = list_get(polygon->vertices, i);
    vector_t rot = vec_rotate(*vertex, angle);
    vertex->x = rot.x;
    vertex->y = rot.y;
  }
  polygon->centroid = vec_rotate(polygon->centroid, angle);
  polygon_translate(polygon, point);
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  polygon->color = color;
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  polygon->centroid = centroid;
}

vector_t polygon_get_center(polygon_t *polygon) { return polygon->centroid; }

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon->rotation_speed = rot;
}

double polygon_get_rotation(polygon_t *polygon) {
  return polygon->rotation_speed;
}

list_t *make_rectangle(vector_t center, double w, double h) {
  list_t *points = list_init(4, free);

  vector_t *p1 = malloc(sizeof(vector_t));
  assert(p1 != NULL);
  *p1 = (vector_t){center.x - w / 2, center.y - h / 2};
  vector_t *p2 = malloc(sizeof(vector_t));
  assert(p2 != NULL);
  *p2 = (vector_t){center.x + w / 2, center.y - h / 2};
  vector_t *p3 = malloc(sizeof(vector_t));
  assert(p3 != NULL);
  *p3 = (vector_t){center.x + w / 2, center.y + h / 2};
  vector_t *p4 = malloc(sizeof(vector_t));
  assert(p4 != NULL);
  *p4 = (vector_t){center.x - w / 2, center.y + h / 2};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);

  return points;
}