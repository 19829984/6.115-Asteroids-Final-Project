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
#include "tft.h"
#include <math.h>
#include "pair.h"
#include <stdbool.h> 

// void WrapCoordinateF(float x, float y, float* out_x, float* out_y){
//     *out_x = x;
//     *out_y = y;
//     if (x < 0 ) *out_x = x + (float)SCREEN_SIZE_X;
//     if (x >= (float)SCREEN_SIZE_X) *out_x = x - (float)SCREEN_SIZE_X;
//     if (y < 0 ) *out_y = y + (float)SCREEN_SIZE_Y;
//     if (y >= (float)SCREEN_SIZE_Y) *out_y = y - (float)SCREEN_SIZE_Y;
// }

void WrapCoordinateF(float x, float y, float* out_x, float* out_y){
    *out_x = (!(x < 0) && !(x >= (float)SCREEN_SIZE_X)) * x + (x < 0) * (x + (float)SCREEN_SIZE_X) + (x >= (float)SCREEN_SIZE_X) * (x - (float)SCREEN_SIZE_X);
    *out_y = (!(y < 0) && !(y >= (float)SCREEN_SIZE_Y)) * y + (y < 0) * (y + (float)SCREEN_SIZE_Y) + (y >= (float)SCREEN_SIZE_Y) * (y - (float)SCREEN_SIZE_Y);
}

void WrapCoordinate(int x, int y, int* out_x, int* out_y){
    *out_x = (!(x < 0) && !(x >= SCREEN_SIZE_X)) * x + (x < 0) * (x + SCREEN_SIZE_X) + (x >= SCREEN_SIZE_X) * (x - SCREEN_SIZE_X);
    *out_y = (!(y < 0) && !(y >= SCREEN_SIZE_Y)) * y + (y < 0) * (y + SCREEN_SIZE_Y) + (y >= SCREEN_SIZE_Y) * (y - SCREEN_SIZE_Y);
}

bool isPointInsideCircle(PairF* point, PairF* circle, float radius){
    float x_dist = point->x - circle->x;
    float y_dist = point->y - circle->y;
    float dist_squared = pow(x_dist, 2) + pow(y_dist, 2);
    return sqrt(dist_squared) < radius;
}
/* [] END OF FILE */
