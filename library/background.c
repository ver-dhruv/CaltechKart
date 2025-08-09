#include "background.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>

const size_t POINTS = 16;
const double SCALING_FACTOR = 2.0;

const vector_t BACKGROUND[] = {
    {-1723, 4221}, {-1723, -1926}, {2424, -1926}, {2424, 4221}};

const vector_t INSIDE_IN[] = {
    {0, -1300},    {100, -1300},  {200, -1200}, {200, 1200},
    {600, 1600},   {600, 2400},   {700, 2500},  {1700, 2500},
    {1800, 2600},  {1800, 3500},  {1700, 3600}, {-1000, 3600},
    {-1100, 3500}, {-1100, 1900}, {-100, 900},  {-100, -1200}};

const vector_t INSIDE_OUT[] = {
    {-2, -1305},   {102, -1305},  {204, -1203}, {204, 1197},
    {604, 1597},   {604, 2397},   {702, 2495},  {1702, 2495},
    {1804, 2597},  {1804, 3501},  {1702, 3603}, {-1002, 3603},
    {-1104, 3501}, {-1104, 1897}, {-104, 897},  {-104, -1203}};

const vector_t OUTSIDE_IN[] = {
    {-49, -1420},  {150, -1420},  {320, -1250}, {320, 1150},
    {720, 1550},   {720, 2350},   {750, 2380},  {1750, 2380},
    {1920, 2550},  {1920, 3549},  {1750, 3719}, {-1049, 3719},
    {-1219, 3549}, {-1219, 1850}, {-219, 850},  {-219, -1250}};

const vector_t OUTSIDE_OUT[] = {
    {-51, -1425},  {152, -1425},  {324, -1253}, {324, 1147},
    {724, 1547},   {724, 2347},   {752, 2375},  {1752, 2375},
    {1924, 2547},  {1924, 3550},  {1752, 3722}, {-1051, 3722},
    {-1223, 3550}, {-1223, 1847}, {-223, 847},  {-223, -1253}};

list_t *get_inside_out() {
  list_t *points = list_init(POINTS, free);
  for (size_t i = 0; i < POINTS; i++) {
    vector_t *vec = malloc(sizeof(vector_t));
    assert(vec != NULL);
    *vec = vec_multiply(SCALING_FACTOR, INSIDE_OUT[i]);
    list_add(points, vec);
  }
  return points;
}

list_t *get_outside_in() {
  list_t *points = list_init(POINTS, free);
  for (size_t i = 0; i < POINTS; i++) {
    vector_t *vec = malloc(sizeof(vector_t));
    assert(vec != NULL);
    *vec = vec_multiply(SCALING_FACTOR, OUTSIDE_IN[i]);
    list_add(points, vec);
  }
  return points;
}

list_t *inside_walls_points(double width) {
  list_t *inside_walls_points = list_init(POINTS, (free_func_t)list_free);
  list_t *inner = get_inside_out();
  list_t *outer = get_outside_in();
  for (size_t i = 0; i < POINTS; i++) {
    list_t *wall = list_init(4, free);
    vector_t *p1 = malloc(sizeof(vector_t));
    assert(p1 != NULL);
    *p1 = *(vector_t *)list_get(inner, i);
    list_add(wall, p1);
    vector_t *p2 = malloc(sizeof(vector_t));
    assert(p2 != NULL);
    *p2 = *(vector_t *)list_get(inner, (i + 1) % POINTS);
    list_add(wall, p2);
    // Get wall vector
    vector_t side = vec_subtract(*p2, *p1);
    // Rotate 90 degrees
    side = (vector_t){-side.y, side.x};
    // Set length to be desired width
    side = vec_multiply(width / vec_get_length(side), side);
    // Vector pointing "roughly" in the desired direction
    vector_t vec = vec_subtract(*p1, *(vector_t *)list_get(outer, i));
    // Ensuring we extend the wall in the correct direction
    if (vec_dot(side, vec) < 0) {
      side = vec_negate(side);
    }
    vector_t *p3 = malloc(sizeof(vector_t));
    assert(p3 != NULL);
    *p3 = vec_add(*p2, side);
    list_add(wall, p3);
    vector_t *p4 = malloc(sizeof(vector_t));
    assert(p4 != NULL);
    *p4 = vec_add(*p1, side);
    list_add(wall, p4);
    list_add(inside_walls_points, wall);
  }
  return inside_walls_points;
}

list_t *outside_walls_points(double width) {
  list_t *outside_walls_points = list_init(POINTS, (free_func_t)list_free);
  list_t *inner = get_inside_out();
  list_t *outer = get_outside_in();
  for (size_t i = 0; i < POINTS; i++) {
    list_t *wall = list_init(4, free);
    vector_t *p1 = malloc(sizeof(vector_t));
    assert(p1 != NULL);
    *p1 = *(vector_t *)list_get(outer, i);
    list_add(wall, p1);
    vector_t *p2 = malloc(sizeof(vector_t));
    assert(p2 != NULL);
    *p2 = *(vector_t *)list_get(outer, (i + 1) % POINTS);
    list_add(wall, p2);
    // Get wall vector
    vector_t side = vec_subtract(*p2, *p1);
    // Rotate 90 degrees
    side = (vector_t){-side.y, side.x};
    // Set length to be desired width
    side = vec_multiply(width / vec_get_length(side), side);
    // Vector pointing "roughly" in the desired direction
    vector_t vec = vec_subtract(*p1, *(vector_t *)list_get(inner, i));
    // Ensuring we extend the wall in the correct direction
    if (vec_dot(side, vec) < 0) {
      side = vec_negate(side);
    }
    vector_t *p3 = malloc(sizeof(vector_t));
    assert(p3 != NULL);
    *p3 = vec_add(*p2, side);
    list_add(wall, p3);
    vector_t *p4 = malloc(sizeof(vector_t));
    assert(p4 != NULL);
    *p4 = vec_add(*p1, side);
    list_add(wall, p4);
    list_add(outside_walls_points, wall);
  }
  return outside_walls_points;
}

list_t *background_list() {
  list_t *points = list_init(4, free);
  for (size_t i = 0; i < 4; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    *point = vec_multiply(SCALING_FACTOR, BACKGROUND[i]);
    list_add(points, point);
  }
  return points;
}