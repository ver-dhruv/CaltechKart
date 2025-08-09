#include "vector.h"
#include <math.h>

const vector_t VEC_ZERO = {0, 0};

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum;
  sum.x = v1.x + v2.x;
  sum.y = v1.y + v2.y;
  return sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v1) { return vec_multiply(-1, v1); }

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t prod;
  prod.x = scalar * v.x;
  prod.y = scalar * v.y;
  return prod;
}

double vec_dot(vector_t v1, vector_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

double vec_cross(vector_t v1, vector_t v2) {
  return (v1.x * v2.y) - (v1.y * v2.x);
}

vector_t vec_rotate(vector_t v, double angle) {
  vector_t rot;
  rot.x = v.x * cos(angle) - v.y * sin(angle);
  rot.y = v.x * sin(angle) + v.y * cos(angle);
  return rot;
}

vector_t integrate_simpson(vector_t start, vector_t mid, vector_t end,
                           double dt) {
  return vec_multiply(dt / 6.0,
                      vec_add(vec_add(start, vec_multiply(4.0, mid)), end));
}