#include "forces.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double MIN_DIST = 5;

typedef struct body_aux {
  double force_const;
  list_t *bodies;
} body_aux_t;

typedef struct force_info {
  void *info;
  force_creator_t force_creator;
  list_t *bodies;
} force_info_t;

typedef struct collision_aux {
  double force_const;
  list_t *bodies;
  collision_handler_t handler;
  bool collided;
  void *aux; // aux (if allocated in memory) should be free'd by the caller
} collision_aux_t;

body_aux_t *body_aux_init(double force_const, list_t *bodies) {
  body_aux_t *aux = malloc(sizeof(body_aux_t));
  assert(aux);

  aux->bodies = bodies;
  aux->force_const = force_const;

  return aux;
}

force_info_t *force_info_init(void *info, force_creator_t force_creator,
                              list_t *bodies) {
  force_info_t *f_inf = malloc(sizeof(force_info_t));
  assert(f_inf != NULL);
  f_inf->force_creator = force_creator;
  f_inf->info = info;
  f_inf->bodies = bodies;
  return f_inf;
}

void body_aux_free(void *aux) {
  list_free(((body_aux_t *)aux)->bodies);
  free(aux);
}

void force_info_free(force_info_t *f_inf) {
  body_aux_free(f_inf->info);
  list_free(f_inf->bodies);
  free(f_inf);
}

force_creator_t f_info_get_f_creator(force_info_t *f_inf) {
  return f_inf->force_creator;
}

list_t *f_info_get_bodies(force_info_t *f_inf) { return f_inf->bodies; }

void *f_info_get_aux(force_info_t *f_inf) { return f_inf->info; }

collision_aux_t *collision_aux_init(double force_const, list_t *bodies,
                                    collision_handler_t handler, bool collided,
                                    void *aux) {
  collision_aux_t *collision_aux = malloc(sizeof(collision_aux_t));
  assert(collision_aux);

  collision_aux->force_const = force_const;
  collision_aux->bodies = bodies;
  collision_aux->handler = handler;
  collision_aux->collided = collided;
  collision_aux->aux = aux;
  return collision_aux;
}

/**
 * The force creator for gravitational forces between objects. Calculates
 * the magnitude of the force components and adds the force to each
 * associated body.
 *
 * @param info auxiliary information about the force and associated bodies
 */
static void newtonian_gravity(void *info) {
  body_aux_t *aux = (body_aux_t *)info;
  vector_t displacement =
      vec_subtract(body_get_centroid(list_get(aux->bodies, 0)),
                   body_get_centroid(list_get(aux->bodies, 1)));
  vector_t unit_disp =
      vec_multiply(1 / sqrt(vec_dot(displacement, displacement)), displacement);

  double distance = sqrt(vec_dot(displacement, displacement));

  if (distance > MIN_DIST) {
    vector_t grav_force = vec_multiply(
        aux->force_const * body_get_mass(list_get(aux->bodies, 0)) *
            body_get_mass(list_get(aux->bodies, 1)) /
            vec_dot(displacement, displacement),
        unit_disp);

    body_add_force(list_get(aux->bodies, 1), grav_force);
    body_add_force(list_get(aux->bodies, 0), vec_multiply(-1, grav_force));
  }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, NULL);
  list_t *aux_bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);
  body_aux_t *aux = body_aux_init(G, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)newtonian_gravity, aux,
                                 bodies);
}

/**
 * The force creator for friction forces between a body and rigid surface.
 * Calculates the magnitude of the force components and adds the force to
 * the body.
 *
 * @param info auxiliary information about the force and associated bodies
 */
static void friction_surface(void *info) {
  body_aux_t *aux = (body_aux_t *)info;
  body_t *b = list_get(aux->bodies, 0);
  body_t *surface = list_get(aux->bodies, 1);
  // if the body is on the surface add a retarding impulse
  if (find_collision(b, surface).collided) {
    vector_t fric = vec_multiply(-1 * body_get_mass(b) * aux->force_const,
                                 body_get_velocity(b));
    body_add_impulse(b, fric);
  }
}

void create_friction_surface(scene_t *scene, double mu, body_t *body1,
                             body_t *surface) {
  list_t *bodies = list_init(2, NULL);
  list_t *aux_bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, surface);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, surface);
  body_aux_t *aux = body_aux_init(mu, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)friction_surface, aux,
                                 bodies);
}

/**
 * The force creator for spring forces between objects. Calculates
 * the magnitude of the force components and adds the force to each
 * associated body.
 *
 * @param info auxiliary information about the force and associated bodies
 */
static void spring_force(void *info) {
  body_aux_t *aux = info;

  double k = aux->force_const;
  body_t *body1 = list_get(aux->bodies, 0);
  body_t *body2 = list_get(aux->bodies, 1);

  vector_t center_1 = body_get_centroid(body1);
  vector_t center_2 = body_get_centroid(body2);
  vector_t distance = vec_subtract(center_1, center_2);

  vector_t spring_force = {-k * distance.x, -k * distance.y};
  body_add_force(body1, spring_force);
  body_add_force(body2, vec_negate(spring_force));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  list_t *aux_bodies = list_init(2, NULL);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);
  body_aux_t *aux = body_aux_init(k, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring_force, aux,
                                 bodies);
}

/**
 * The force creator for drag forces on an object. Calculates
 * the magnitude of the force components and adds the force to the
 * associated body.
 *
 * @param info auxiliary information about the force and associated body
 */
static void drag_force(void *info) {
  body_aux_t *aux = (body_aux_t *)info;
  vector_t cons_force = vec_multiply(
      -1 * aux->force_const * body_get_mass(list_get(aux->bodies, 0)),
      body_get_velocity(list_get(aux->bodies, 0)));
  body_add_force(list_get(aux->bodies, 0), cons_force);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  list_t *bodies = list_init(1, NULL);
  list_t *aux_bodies = list_init(1, NULL);
  list_add(bodies, body);
  list_add(aux_bodies, body);
  body_aux_t *aux = body_aux_init(gamma, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag_force, aux,
                                 bodies);
}

/**
 * The force creator for collisions. Checks if the bodies in the collision aux
 * are colliding, and if they do, runs the collision handler on the bodies.
 *
 * @param info auxiliary information about the force and associated body
 */
static void collision_force_creator(void *collision_aux) {
  collision_aux_t *col_aux = collision_aux;

  list_t *bodies = col_aux->bodies;
  body_t *body1 = list_get(bodies, 0);
  body_t *body2 = list_get(bodies, 1);

  // Check for collision; if bodies collide, call collision_handler
  bool prev_collision = col_aux->collided;

  collision_info_t info = find_collision(body1, body2);
  // avoids registering impulse multiple times while bodies are still colliding
  if (info.collided && !prev_collision) {
    collision_handler_t handler = col_aux->handler;

    handler(body1, body2, info.axis, col_aux->aux, col_aux->force_const);
    col_aux->collided = true;
  } else if (!info.collided && prev_collision) {
    col_aux->collided = false;
  }
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      double force_const) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  list_t *aux_bodies = list_init(2, NULL);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);

  collision_aux_t *collision_aux =
      collision_aux_init(force_const, aux_bodies, handler, false, aux);

  scene_add_bodies_force_creator(scene, collision_force_creator, collision_aux,
                                 bodies);
}

/**
 * The collision handler for destructive collisions.
 */
static void destructive_collision(body_t *body1, body_t *body2, vector_t axis,
                                  void *aux, double force_const) {
  if (find_collision(body1, body2).collided) {
    body_remove(body1);
    body_remove(body2);
  }
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2, destructive_collision, NULL, 0);
}

void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux, double force_const) {
  double m;
  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);
  if (m1 == INFINITY) {
    m = m2;
  } else if (m2 == INFINITY) {
    m = m1;
  } else {
    m = (m1 * m2) / (m1 + m2);
  }
  vector_t v1 = body_get_velocity(body1);
  vector_t v2 = body_get_velocity(body2);
  double impulse = m * (1 + force_const) * vec_dot(axis, vec_subtract(v2, v1));
  body_add_impulse(body1, vec_multiply(impulse, axis));
  body_add_impulse(body2, vec_multiply(-impulse, axis));
}

void create_physics_collision(scene_t *scene, body_t *body1, body_t *body2,
                              double elasticity) {
  create_collision(scene, body1, body2, physics_collision_handler, NULL,
                   elasticity);
}