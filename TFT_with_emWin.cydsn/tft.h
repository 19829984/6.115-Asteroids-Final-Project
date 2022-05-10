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
#ifndef TFT_H
#define TFT_H
#include "project.h"
#include "pair.h"
    
#ifndef SCREEN_SIZE_X
#define SCREEN_SIZE_X 240
#endif

#ifndef SCREEN_SIZE_Y
#define SCREEN_SIZE_Y 320
#endif

// Set area on the screen to draw on
inline void tftSetArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
// Send 8 bit command to screen
void write8_command(uint8 data);
// Send 8 bit data to screen
void write8_data(uint8 data);
// Send multiple 8 bit data to screen
void writeM8_data(uint8 *pData, int N);
uint8 read8_a1(void);
void readM8_a1(uint8 *pData, int N);
// Start tft screen
void tftStart(void);
// Draw rectangle on tft screen
void tftDrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
// Draw rectangle on tft screen with wrapping
void tftDrawRectWrap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
// Draw pixel on tft screen
void tftDrawPixel(int x, int y, uint16_t color);
// Draw a line on tft screen
void tftDrawLine(int x0, int y0, int x1, int y1, uint16_t color);
// Draw a wireframe on tft screen
void tftDrawWireframe(PairF* coordinates, int num_coords, float x, float y, float rotation, float scale, uint16_t col);
// Clear tft screen
inline void tftClear();

/* [] END OF FILE */
#endif
