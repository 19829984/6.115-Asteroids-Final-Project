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
#include "controls.h"
#include <project.h>
#include "utils.h"

const float MAX_VOLTAGE = 4.6;
const float NEUTRAL_VOLTAGE = 2.3;
const float DEAD_ZONE = 1;

float get_input_x(){
    Joystick_Mux_Select(0);
    float x_volt = minf(MAX_VOLTAGE, ADC_SAR_1_CountsTo_Volts(ADC_SAR_1_GetResult16()));
    if (x_volt >= NEUTRAL_VOLTAGE - DEAD_ZONE && x_volt <= NEUTRAL_VOLTAGE + DEAD_ZONE) return 0;
    if (x_volt > NEUTRAL_VOLTAGE + DEAD_ZONE) x_volt = x_volt - 2*DEAD_ZONE; // Offset upper bracket to eliminate discontinuty introduced by deadzone
    return 2*(x_volt - (NEUTRAL_VOLTAGE - DEAD_ZONE))/(MAX_VOLTAGE - 2*DEAD_ZONE); // Calculate output with neutral voltage and max voltage shifted by deadzone value
}

float get_input_y(){
    Joystick_Mux_Select(1);
    float y_volt = minf(MAX_VOLTAGE, ADC_SAR_1_CountsTo_Volts(ADC_SAR_1_GetResult16()));
    if (y_volt >= NEUTRAL_VOLTAGE - DEAD_ZONE && y_volt <= NEUTRAL_VOLTAGE + DEAD_ZONE) return 0;
    if (y_volt > NEUTRAL_VOLTAGE + DEAD_ZONE) y_volt = y_volt - 2*DEAD_ZONE;
    return 2*(y_volt - (NEUTRAL_VOLTAGE - DEAD_ZONE))/(MAX_VOLTAGE - 2*DEAD_ZONE);
}

/* [] END OF FILE */
