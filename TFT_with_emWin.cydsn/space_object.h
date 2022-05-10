#ifndef SPACE_OBJ_H
#define SPACE_OBJ_H
#include "pair.h"
typedef struct {
   PairF loc; // Object location
   PairF vel; // Object velocity
   int size; // Object size
   float angle; // In radians
} SpaceObjBase;

#endif
