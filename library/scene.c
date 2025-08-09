#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "forces.h"
#include "scene.h"

const double INITIAL_BODIES = 10;
const size_t INITIAL_FORCES = 1;

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
};

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    force_info_t *f_inf = list_get(scene->force_creators, i);
    f_info_get_f_creator(f_inf)(f_info_get_aux(f_inf));
  }

  for (ssize_t i = 0; i < (ssize_t)scene->num_bodies; i++) {
    body_t *body = list_get(scene->bodies, i);
    if (body_is_removed(body)) {
      for (size_t j = 0; j < list_size(scene->force_creators); j++) {
        force_info_t *f_inf = list_get(scene->force_creators, j);
        list_t *force_bodies = f_info_get_bodies(f_inf);
        for (size_t k = 0; k < list_size(force_bodies); k++) {
          if (list_get(force_bodies, k) == body) {
            list_remove(scene->force_creators, j);
            force_info_free(f_inf);
            j--; // decrement since we remove one force_info
            break;
          }
        }
      }
      list_remove(scene->bodies, i);
      body_free(body);
      scene->num_bodies--;
      i--; // decrement since we remove one body
    } else {
      body_tick(body, dt);
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  force_info_t *f_inf = force_info_init(aux, forcer, bodies);
  list_add(scene->force_creators, f_inf);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene != NULL);
  scene->bodies = list_init(INITIAL_BODIES, (free_func_t)body_free);
  scene->force_creators =
      list_init(INITIAL_FORCES, (free_func_t)force_info_free);
  scene->num_bodies = 0;
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_creators);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  scene->num_bodies++;
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(list_get(scene->bodies, index));
}

void scene_center_body(scene_t *scene, body_t *body, vector_t center) {
  vector_t shift = vec_subtract(center, body_get_centroid(body));
  size_t bodies = scene_bodies(scene);
  for (size_t i = 0; i < bodies; i++) {
    body_t *curr = scene_get_body(scene, i);
    body_set_centroid(curr, vec_add(body_get_centroid(curr), shift));
  }
}