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
#include <assert.h>
#include "utils.h"
#include <math.h>

//==============================================================
// write8_command()
// writes an 8-bit value to the TFT with the D/C line low
//
// Arguments:
//      data - 8-bit value
//==============================================================
void write8_command(uint8 data)
{
	DC_Write(0x00); 						        // set DC line low
    SPIM_1_WriteTxData(data);         		        // send data to transmit buffer
    while (!(SPIM_1_ReadTxStatus() & 0x01)){};	    // wait for data to be sent
}

//==============================================================
// write8_data()
// writes an 8-bit value to the TFT with the D/C line high
//
// Arguments:
//      data - 8-bit value
//==============================================================
void write8_data(uint8 data)
{
	DC_Write(0x01); 						        // set DC line high
    SPIM_1_WriteTxData(data);                       // send data to transmit buffer
    while (!(SPIM_1_ReadTxStatus() & 0x01)){};	    // wait for data to be sent
}

//==============================================================
// writeM8_data()
// writes multiple bytes to the TFT with the D/C line high
//
// Arguments:
//      pData - pointer to an array of 8-bit data values
//      N - the size of the array *pData
//==============================================================
void writeM8_data(uint8 *pData, int N)
{
	DC_Write(0x01); 						        // set DC line high
    int i;
    for (i=0; i<N; i++)
    {
        SPIM_1_WriteTxData(pData[i]);               // send data to transmit buffer
        while (!(SPIM_1_ReadTxStatus() & 0x01)){};  // wait for data to be sent
    }
}

//==============================================================
// read8_a1()
// reads an 8-bit value to the TFT with the D/C line high
//==============================================================
uint8 read8_a1(void)
{
    SPIM_1_ReadRxData();
	for (;;) {}                                     // read function not implemented
};

//==============================================================
// readM8_a1()
// reads multple 8-bit values from the TFT with the D/C line high
//
// Arguments:
//      pData - an array where the read values we be stored
//      N - the number of values that will be read (also size of 
//          the array pData)
//==============================================================
void readM8_a1(uint8 *pData, int N)
{
	for (;;) {}                                     // read function not implemented
}

//==============================================================
// tftStart()
// this function must be called to initializes the TFT
//==============================================================
void tftStart(void)
{
    write8_command(0x01);         			// send Software Reset Command (must wait at least 5ms after command)
    CyDelay(10);
    write8_command(0x36);         			// send Memory Access Control command
	write8_data(0x88);
    write8_command(0x3A);         			// send COLMOD: Pixel Format Set command
    write8_data(0x55);
    write8_command(0x11);         			// send Sleep Out command
    write8_command(0x29);         			// send Display ON command
    CyDelay(250);            			// delay to allow all changes to take effect	
}

inline void setColumn(uint16_t x, uint16_t width){
    write8_command(0x2A);                 	// send Column Address Set command
    write8_data(x >> 8);                 // set SC[15:0]
    write8_data(x & 0x00FF);
    write8_data((x + width) >> 8);                 // set EC[15:0]
    write8_data((x + width) & 0x00FF);
}

inline void setPage(uint16_t y, uint16_t height){
    write8_command(0x2B);                 	// send Page Address Set command
    write8_data(y >> 8);                 // set SC[15:0]
    write8_data(y & 0x00FF);
    write8_data((y + height) >> 8);                 // set EC[15:0]
    write8_data((y + height) & 0x00FF);
}

inline void tftSetArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
    setColumn(x, width);
    setPage(y, height);
}

void tftDrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color){
    // Check boundary conditions
    assert(x >= 0 && x < SCREEN_SIZE_X);
    assert(y >= 0 && y < SCREEN_SIZE_Y);
    assert(width > 0 && width + x <= SCREEN_SIZE_X);
    assert(height > 0 && height + y <= SCREEN_SIZE_Y);

    tftSetArea(x, y, width-1, height-1);
    write8_command(0x2C); // Memory Write
    uint8_t color_upper = color >> 8;
    uint8_t color_lower = color & 0x00FF;
    int i;
    for (i = 0; i < width * height; i++){
        write8_data(color_upper);
        write8_data(color_lower);
    }
    write8_command(0x00); // NOP to end write
}

void tftDrawRectWrap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color){
    int wrapped_distance_x = (x + width) - SCREEN_SIZE_X;
    int wrapped_distance_y = (y + height) - SCREEN_SIZE_Y;
    // Just draw the rectangle normally if no wrapping occurs. 
    if (!(wrapped_distance_x > 0) && !(wrapped_distance_y > 0)){
        return tftDrawRect(x, y, width, height, color);
    }
    // Draw the unwrapped portion
    uint16_t x_unwrapped_distance = min(x + width, SCREEN_SIZE_X) - x;
    uint16_t y_unwrapped_distance = min(y + height, SCREEN_SIZE_Y) - y;
    tftDrawRect(x, y, x_unwrapped_distance, y_unwrapped_distance, color);
    // Now draw the wrapped portion
    // Calculate starting position of wrapped rect
    uint16_t x_wrapped_start = wrapped_distance_x > 0 ? 0 : x;
    uint16_t y_wrapped_start = wrapped_distance_y > 0 ? 0 : y;
    // Calculate width and height of the wrapped part
    uint16_t wrapped_width = wrapped_distance_x <= 0 ? width : wrapped_distance_x;
    uint16_t wrapped_height = wrapped_distance_y <= 0 ? height : wrapped_distance_y;
    tftDrawRect(x_wrapped_start, y_wrapped_start, wrapped_width, wrapped_height, color);
}

void tftDrawPixel(int x, int y, uint16_t color){
    WrapCoordinate(x, y, &x, &y);
    tftSetArea(x, y, 0, 0);
    write8_command(0x2C); // Memory Write
    uint8_t color_upper = color >> 8;
    uint8_t color_lower = color & 0x00FF;
    write8_data(color_upper);
    write8_data(color_lower);
    write8_command(0x00); // NOP to end write
}


void tftDrawLine(int x0, int y0, int x1, int y1, uint16_t color){
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 <= y1 ? 1 : -1;
    int error = dx + dy;

    for (;;) {
        tftDrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * error;
        if (e2 >= dy){
            if (x0 == x1) break;
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx){
            if (y0 == y1) break;
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void tftDrawWireframe(PairF* coordinates, int num_coords, float x, float y, float rotation, float scale, uint16_t col){
    PairF* transformed_coords = malloc(sizeof(PairF) * num_coords);
    int i;
    for (i = 0; i < num_coords; i++){
        // Perform rotation
        transformed_coords[i].x = coordinates[i].x * cosf(rotation) - coordinates[i].y * sinf(rotation);
        transformed_coords[i].y = coordinates[i].x * sinf(rotation) + coordinates[i].y * cosf(rotation);
        // Scale
        transformed_coords[i].x = transformed_coords[i].x * scale;
        transformed_coords[i].y = transformed_coords[i].y * scale;
        // Translation
        transformed_coords[i].x += x;
        transformed_coords[i].y += y;
    }
    
    for (i = 0; i <= num_coords; i++){
        int current_point_index = i%num_coords;
        int next_point_index = (i + 1)%num_coords;
        tftDrawLine(transformed_coords[current_point_index].x, transformed_coords[current_point_index].y,
         transformed_coords[next_point_index].x, transformed_coords[next_point_index].y, col);
    }
    
    free(transformed_coords);
}

inline void tftClear(void){
    tftDrawRect(0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y, (uint16_t)0x0000);
}

/* [] END OF FILE */

