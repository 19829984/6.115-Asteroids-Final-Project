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
#include "vector.h"
#include "assert.h"
#include <stdlib.h>
#include "GUI.h"

void vectorInit(sVector* v, int size, SpaceObjBase* array){
    assert(size > 0);
    assert(v != NULL);
    v->items = array;
    v->size = size;
    v->used = 0;
}

void vectorPush(sVector* v, PairF loc, PairF vel, int size, float angle){
    if (v){
        if (v->used < v->size){
            SpaceObjBase* obj = &v->items[v->used++];
            obj->loc = loc;
            obj->vel = vel;
            obj->size = size;
            obj->angle = angle;
        }
    }
}

SpaceObjBase *vectorGet(sVector* v, int index){
    if (v){
        if ((index >= 0) && (index < v->size)){
            return &v->items[index];
        }
    }
    return NULL;
}

// Swap index with last item, and decrement used
void vectorDelete(sVector *v, int index){
    if (v){
        if ((index < 0) || (index >= v->size)) return;

        SpaceObjBase* last_item = vectorGet(v, v->used-1); 
        if (last_item == NULL) return;
        v->items[index].loc = last_item->loc;
        v->items[index].vel = last_item->vel;
        v->items[index].angle = last_item->angle;
        v->items[index].size = last_item->size;
        
        v->used--;
    }
}
/* [] END OF FILE */
