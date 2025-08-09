#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"

const double BALL_RADIUS = 15;
const double BALL_MASS = 5;
const vector_t BALL_INIT_VEL = {400, 400};
const vector_t BALL_INIT_POS = {500, 70};
const double ELASTICITY = 1;

const double RECTANGLE_WIDTH = 98;
const double RECTANGLE_HEIGHT = 40;
const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const double resting_speed = 300;
const double ACCEL = 100;

const size_t BRICKS_IN_ROW = 10;
const size_t ROWS = 3;
const double BRICK_INIT_Y = 475;
const double BRICK_OFFSET = 3;

const double WALL_DIM = 1;

const double USER_HEIGHT = 25;
const vector_t USER_INIT_POS = {500, 25};
const double USER_MASS = INFINITY;

rgb_color_t user_color = (rgb_color_t){0.5, 0.5, 0.5};
rgb_color_t white = (rgb_color_t){1, 1, 1};
vector_t FIRST_STONE = {1, 7};
vector_t SECOND_STONE = {2, 2};

const size_t CIRC_NPOINTS = 100;

struct state {
  scene_t *scene;
  double time_pressed;
  body_t *ball;
  body_t *user;
};

typedef enum { BALL, WALL, BRICK, GROUND } body_type_t;

body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  *info = type;
  return info;
}

body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

/** Make a circle-shaped body object.
 *
 * @param center a vector representing the center of the body.
 * @param radius the radius of the circle
 * @param mass the mass of the body
 * @param color the color of the circle
 * @return pointer to the circle-shaped body
 */
list_t *make_circle(vector_t center, double radius) {
  list_t *c = list_init(CIRC_NPOINTS, free);
  for (size_t i = 0; i < CIRC_NPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_NPOINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(c, v);
  }
  return c;
}

/** Make a rectangle-shaped body object.
 *
 * @param center a vector representing the center of the body.
 * @param width the width of the rectangle
 * @param height the height of the rectangle
 * @return pointer to the rectangle-shaped body
 */
list_t *make_rectangle(vector_t center, double width, double height) {
  list_t *points = list_init(4, free);
  vector_t *p1 = malloc(sizeof(vector_t));
  *p1 = (vector_t){center.x - width / 2, center.y - height / 2};

  vector_t *p2 = malloc(sizeof(vector_t));
  *p2 = (vector_t){center.x + width / 2, center.y - height / 2};

  vector_t *p3 = malloc(sizeof(vector_t));
  *p3 = (vector_t){center.x + width / 2, center.y + height / 2};

  vector_t *p4 = malloc(sizeof(vector_t));
  *p4 = (vector_t){center.x - width / 2, center.y + height / 2};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);

  return points;
}

/**
 * Wrap object around other side of screen display if it reaches any edge of the
 * display.
 *
 * @param body the body object representing pacman
 */
void user_wrap_edges(body_t *body) {
  vector_t centroid = body_get_centroid(body);
  if (centroid.x - RECTANGLE_WIDTH / 2 > MAX.x) {
    body_set_centroid(body, (vector_t){MIN.x, centroid.y});
  } else if (centroid.x + RECTANGLE_WIDTH / 2 < MIN.x) {
    body_set_centroid(body, (vector_t){MAX.x, centroid.y});
  }
}

/**
 * Move player on display screen based on key pressed.
 *
 * @param key the character of the key pressed
 * @param type event type connected to key
 * @param held_time double value representing the amount of time the key is held
 * down
 * @param state the state representing the current demo
 */
void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  // TODO: implement this!
  vector_t vel = VEC_ZERO;
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW: {
      vel = (vector_t){-(resting_speed + ACCEL * held_time), 0};
      break;
    }
    case RIGHT_ARROW: {
      vel = (vector_t){resting_speed + ACCEL * held_time, 0};
      break;
    }
    default:
      break;
    }
  }
  body_set_velocity(state->user, vel);
}

/**
 * Create a list of colors for the rainbow bricks in breakout demo.
 *
 * @return a pointer to a list containing the colors of the bricks.
 */
list_t *get_colors() {
  list_t *color = list_init(BRICKS_IN_ROW, free);
  for (double i = 0; i < 1; i += (1. / BRICKS_IN_ROW)) {
    list_add(color, color_init(i, 0.7, 0.7));
  }
  return color;
}

/**
 * Adds bricks as bodies to the scene.
 *
 * @param state the current state of the demo
 */
void add_bricks(state_t *state) {
  list_t *colors = get_colors();
  double rect_width = MAX.x / BRICKS_IN_ROW;

  for (int j = 0; j < BRICKS_IN_ROW; j++) {
    double x = rect_width / 2 + j * (rect_width + BRICK_OFFSET);
    for (int i = 0; i < ROWS; i++) {
      double y = BRICK_INIT_Y - i * (RECTANGLE_HEIGHT + BRICK_OFFSET);
      list_t *rect =
          make_rectangle((vector_t){x, y}, rect_width, RECTANGLE_HEIGHT);

      body_t *box;
      // add stones that cannot be destroyed
      if ((i == FIRST_STONE.x && j == FIRST_STONE.y) ||
          (i == SECOND_STONE.x && j == SECOND_STONE.y)) {
        rgb_color_t color = user_color;
        box = body_init_with_info(rect, INFINITY, color, make_type_info(WALL),
                                  free);
      }

      // add bricks that can be destroyed
      else {
        rgb_color_t color = *((rgb_color_t *)(list_get(colors, j)));
        box = body_init_with_info(rect, INFINITY, color, make_type_info(BRICK),
                                  free);
      }

      scene_add_body(state->scene, box);
    }
  }
  list_free(colors);
}

/**
 * Adds walls as bodies to the scene.
 *
 * @param state the current state of the demo
 */
void add_walls(state_t *state) {
  list_t *wall1_shape =
      make_rectangle((vector_t){MAX.x, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall1 = body_init_with_info(wall1_shape, INFINITY, white,
                                      make_type_info(WALL), free);
  list_t *wall2_shape =
      make_rectangle((vector_t){0, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall2 = body_init_with_info(wall2_shape, INFINITY, white,
                                      make_type_info(WALL), free);
  list_t *ceiling_shape =
      make_rectangle((vector_t){MAX.x / 2, MAX.y}, MAX.x, WALL_DIM);
  body_t *ceiling = body_init_with_info(ceiling_shape, INFINITY, white,
                                        make_type_info(WALL), free);
  list_t *ground_shape =
      make_rectangle((vector_t){MAX.x / 2, 0}, MAX.x, WALL_DIM);
  body_t *ground = body_init_with_info(ground_shape, INFINITY, white,
                                       make_type_info(GROUND), free);
  scene_add_body(state->scene, wall1);
  scene_add_body(state->scene, wall2);
  scene_add_body(state->scene, ceiling);
  scene_add_body(state->scene, ground);
}

/**
 * Adds a ball as a body to the scene.
 *
 * @param state the current state of the demo
 */
void add_ball(state_t *state) {
  list_t *ball_shape = make_circle(BALL_INIT_POS, BALL_RADIUS);
  // TODO: implement this!
  body_t *ball = body_init_with_info(ball_shape, BALL_MASS, user_color,
                                     make_type_info(BALL), free);
  body_set_velocity(ball, BALL_INIT_VEL);
  scene_add_body(state->scene, ball);
  state->ball = ball;
}

/**
 * Adds the user as a body to the scene.
 *
 * @param state the current state of the demo
 */
void add_user(state_t *state) {
  list_t *user_shape =
      make_rectangle(USER_INIT_POS, RECTANGLE_WIDTH, USER_HEIGHT);
  // TODO: implement this!
  body_t *user = body_init_with_info(user_shape, USER_MASS, user_color,
                                     make_type_info(WALL), free);
  scene_add_body(state->scene, user);
  state->user = user;
}

/**
 * The collision handler for collisions between the ball and the brick.
 *
 * @param body1 the body for the ball
 * @param body2 the body for the brick
 * @param axis the axis of collision
 * @param aux the aux passed in from `create_breakout_collision`
 * @param elasticity the elasticity of the collision between the ball and the
 * brick
 */
void breakout_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                void *aux, double force_const) {
  // TODO: fill this in!
  physics_collision_handler(body1, body2, axis, aux, force_const);
  body_remove(body2);
}

/**
 * The breakout collision creator for `breakout_collision_handler`.
 *
 * @param scene the scene of the game
 * @param body1 the body for the ball
 * @param body2 the body for the brick
 * @param elasticity the elasticity of the collision between the ball and the
 * brick
 */
void create_breakout_collision(scene_t *scene, body_t *body1, body_t *body2,
                               double elasticity) {
  collision_handler_t handler = (collision_handler_t)breakout_collision_handler;
  create_collision(scene, body1, body2, handler, scene, elasticity);
}

/**
 * Reset the game to initial starting positions/velocities.
 * This function is a collision handler.
 *
 * @param body1 pointer to a body involved in collision
 * @param body2 pointer to a body involved in collision
 * @param axis the axis of collision
 * @param aux auxiliary information for the collision (in this case it is a
 * pointer to a state_t)
 */
void reset_game(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  // When reset_game was "registered" as a collision handler, the game state
  // should have been passed in as the aux. This gives us access to the
  // state after the collision between the ball and the ground, allowing us
  // to reset the game.

  state_t *state = aux;

  if ((get_type(body1) == BALL && get_type(body2) == GROUND) ||
      (get_type(body2) == BALL && get_type(body1) == GROUND)) {
    // TODO: clear and re-add bricks. Which function initializes bricks in the
    // beginning of the game?
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *body = scene_get_body(state->scene, i);
      if (get_type(body) == BRICK && state->user != body) {
        body_remove(body);
      }
    }
    add_bricks(state);

    // TODO: reset ball's velocity and position
    body_set_centroid(state->ball, BALL_INIT_POS);
    body_set_velocity(state->ball, BALL_INIT_VEL);

    // TODO: reset ball's forces and impulses
    body_reset(state->ball);

    // TODO: re-add force creators for bricks
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *body = scene_get_body(state->scene, i);
      if (get_type(body) == BRICK) {
        create_breakout_collision(state->scene, state->ball, body, ELASTICITY);
      }
    }
  }
}

/**
 * Adds collision handler force creators between appropriate bodies.
 *
 * @param state the current state of the demo
 */
void add_force_creators(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    switch (get_type(body)) {
    case BRICK:
      // TODO: register the collision handler that should run when the ball and
      // the brick collides
      create_breakout_collision(state->scene, state->ball, body, ELASTICITY);
      break;
    case WALL:
      // TODO: register the collision handler that should run when the ball and
      // the wall collides
      create_physics_collision(state->scene, state->ball, body, ELASTICITY);
      break;
    case GROUND:
      // TODO: register the collision handler that should run when the ball and
      // the ground collides
      create_collision(state->scene, state->ball, body,
                       (collision_handler_t)reset_game, state, 0);
      break;
    default:
      break;
    }
  }
}

state_t *emscripten_init() {
  // TODO: implement this!
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->scene = scene_init();
  add_bricks(state);
  add_walls(state);
  add_ball(state);
  add_user(state);
  add_force_creators(state);
  state->time_pressed = 0.0;
  sdl_on_key((void *)on_key);
  return state;
}

bool emscripten_main(state_t *state) {
  // TODO: implement this!
  double dt = time_since_last_tick();
  scene_tick(state->scene, dt);
  user_wrap_edges(state->user);
  sdl_render_scene(state->scene, NULL);
  return false;
}

void emscripten_free(state_t *state) {
  // TODO: implement this!
  scene_free(state->scene);
  free(state);
}