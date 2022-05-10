#ifndef SHIP_H
#define SHIP_H

#include "space_object.h"
#include "math.h"
#include "utils.h"

typedef struct {
    SpaceObjBase props;
    PairF points[3]; // // Coordinates for the triangle of the ship in neutral position
    bool shinda; // If player is dead
} Ship;

// Rotate ship to its latest angle
void ship_rotate(Ship* ship);
// Move ship forward depending on its angle
void ship_move_forward(Ship* ship, float thrust, float delta_ms);
// Draw the ship while moving it forward and rotating it (rotation in radians)
void ship_draw(Ship* ship, float thrust, float rotation, float delta_ms);
#endif
