#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
  R = 6,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * The possible types of mouse events.
 */
typedef enum { MOUSE_CLICKED } mouse_event_type_t;

/**
 * A mousepress handler.
 *
 * @param type the type of mouse event
 * @param state the state of emscripten
 * @param x the x coorindtae of the click
 * @param y the y coorindtae of the click
 */
typedef void (*mouse_handler_t)(void *state, double x, double y);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Loads the image found in the given file location as an SDL_Texture.
 *
 * @param path the file path to the image
 * @return the loaded image
 */
SDL_Texture *sdl_load_image(const char *path);

/**
 * Displays the image, centering it and scaling it to fill the screen.
 * @param img the image to display
 * @param loc the upper coordinates value of the image
 * @param size the size of the image
 */
void sdl_display_image(SDL_Texture *img, vector_t loc, vector_t size);

/**
 * Displays the image, centering it and scaling it to fill the screen.
 * @param img the image to display
 * @param bounding_box the image bounding box
 * @param theta the angle of the image
 * @param centroid the image centroid
 */
void sdl_display_image_with_angle(SDL_Texture *img, SDL_Rect bounding_box,
                                  double theta, vector_t centroid);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */

/**
 * Loads the font found in the given file location as an TTF_Font
 *
 * @param path the file path to the font
 * @return the loaded font
 */
TTF_Font *sdl_load_font(const char *path, size_t size);

/**
 * Displays the message, in a rectangular box
 *
 * @param font the font to display message in
 * @param message the message to be displayed
 * @param loc the upper coordinates value of the message
 * @param color the color used for the text
 */
void sdl_display_message(TTF_Font *font, const char *message, vector_t loc,
                         SDL_Color color);
/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called when the mouse is clicked
 *
 * @param handler the function to call with each mouse press
 */
void sdl_on_click(mouse_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Gets the bounding box of a body
 *
 * @param body the body to get the bounding box of
 * @return the bounding box of body
 */
SDL_Rect sdl_get_bounding_box(body_t *body);

/**
 * Updates the bounding box of a body
 *
 * @param body the body to get the bounding box of
 * @param body the previous bounding box
 * @return the next bounding box of body
 */
SDL_Rect sdl_update_bounding_box_body(body_t *body, SDL_Rect bounding_box);

#endif // #ifndef __SDL_WRAPPER_H__
