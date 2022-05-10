/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef VECTOR_H
#define VECTOR_H
#include "space_object.h"
typedef struct{
    SpaceObjBase *items;
    int used;
    int size;
} sVector;

// Initialize the vector with a max size to an existing array
void vectorInit(sVector* v, int size, SpaceObjBase* array);
// Add a new SpaceObjBase object to the vector
void vectorPush(sVector* v, PairF loc, PairF vel, int size, float angle);
// Get the ith object's pointer
SpaceObjBase *vectorGet(sVector* v, int index);
// Delete the ith object
void vectorDelete(sVector* v, int index);
#endif
/* [] END OF FILE */
