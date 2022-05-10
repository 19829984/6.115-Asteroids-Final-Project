#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>
#include "space_object.h"
#include "tft.h"
#include <stdlib.h>

// Wrap coordinate around the tft screen
void WrapCoordinate(int x, int y, int* out_x, int* out_y);
// Wrap float coordinate around the tft screen
void WrapCoordinateF(float x, float y, float* out_x, float* out_y);

bool isPointInsideCircle(PairF* point, PairF* circle, float radius);

static inline int max(int a, int b){
    return a > b ? a : b;
}

static inline float maxf(float a, float b){
    return a > b ? a : b;
}

static inline float minf(float a, float b){
    return a < b ? a : b;
}

static inline int min(int a, int b){
    return a < b ? a : b;
}

// Return random number from 0 to 1 inclusive
static inline float randf(){
    return ((float)rand() / (float)RAND_MAX);
}

// Return random number from -1 to 1 inclusive
static inline float srandf(){
    return randf()*2 - 1;
}

static inline int sign(int x) {
    return (x > 0) - (x < 0);
}

static inline bool isOutOfBounds(SpaceObjBase* obj){
    return (obj->loc.x < 0 || obj->loc.y < 0 || obj->loc.x >= SCREEN_SIZE_X || obj->loc.y >= SCREEN_SIZE_Y);
}
#endif
