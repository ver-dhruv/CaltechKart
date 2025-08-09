#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include "list.h"
#include "vector.h"
#include <stdlib.h>

/**
 * Returns a list containing the vertices of the outer end of the inside wall,
 * multiplied by a scaling factor to adjust for image resizing.
 */
list_t *get_inside_out();

/**
 * Returns a list containing the vertices of the outer end of the inside wall,
 * multiplied by a scaling factor to adjust for image resizing.
 */
list_t *get_outside_in();

/**
 * Returns a list of lists, where each list contains the four vertices
 * that make up one of the inside wall rectangles. The rectangles are built to
 * have a given width.
 * @param width the desired width of the rectangles
 */
list_t *inside_walls_points(double width);

/**
 * Returns a list of lists, where each list contains the four vertices
 * that make up one of the outside wall rectangles. The rectangles are built to
 * have a given width.
 * @param width the desired width of the rectangles
 */
list_t *outside_walls_points(double width);

/**
 * Returns the list of vertices that make up the background,
 * multiplied by the scaling factor.
 */
list_t *background_list(void);

#endif