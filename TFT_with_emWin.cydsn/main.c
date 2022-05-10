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
#include <project.h>
#include "GUI.h"
#include "controls.h"
#include "tft.h"
#include <stdio.h>
#include "utils.h"
#include <assert.h>
#include "ship.h"
#include <stdlib.h>
#include "vector.h"
#include <stdbool.h> 

void update(float delta_ms);
void init(void);
void init_game(void);
void disp_score();
void DMA_init();
void init_astroids(uint8_t num_astroids);

const float TIME_SCALE = 0.01;
const float TO_RADIAN = 0.0174f;

const float BULLET_SPEED = 10;
const float PLAYER_TURN_SPEED = 8;
const float PLAYER_SPEED = 0.3;
const float ASTROID_SPLIT_SPEED_SCALE = 3;
const float ASTROID_INHERIT_SPEED_SCALE = 1;
const float ASTROID_SPEED_SCALE = 0.5;
const float ASTROID_ROTATE_SPEED = 0.2;
const float ASTROID_HIT_BULLET_SPEED_INHERIT = 0.4;
const float ASTROID_START_SIZE = 16;
#define ASTROID_NUM_VERTS 20
#define NUM_BULLETS 50
#define NUM_ASTROIDS 16
#define NUM_MAX_ASTROIDS 64

PairF astroid_model[ASTROID_NUM_VERTS];

Ship player = {
    {{120, 170}, {0, 0}, 1, 0}, // props
    {{0.f, -5.5f}, {-2.5f,2.5f}, {2.5f, 2.5f}}, // Points for drawing ship
    false // Shinda
};

uint16_t score = 0;

sVector astroids_vec;
SpaceObjBase astroids[NUM_ASTROIDS];
int astroids_to_remove_index[NUM_BULLETS];
uint8_t num_astroids_to_remove = 0;

sVector bullets_vec;
SpaceObjBase bullets[NUM_BULLETS];
int bullets_to_remove_index[NUM_BULLETS];
uint8_t num_bullets_to_remove = 0;

char print[100];
bool button_pushed;


// const uint16_t frame_buffer[SCREEN_SIZE_X * SCREEN_SIZE_Y] = {0};

unsigned long t_last_loop = 0;

int main()
{
    init();
    for(;;) {
        unsigned long t_start_loop = Game_Timer_ReadCounter();
        update(t_last_loop);
        disp_score();
        t_last_loop = t_start_loop - Game_Timer_ReadCounter();
    } // loop
}

void update(float delta_ms)
{
    // If player had died
    if (player.shinda){
        sprintf(print, "Game Over");
        GUI_DispStringAt(print, 70, 170);
        sprintf(print, "Press Fire to start a new game");
        GUI_DispStringAt(print, 0, 190);
        for(;;) {
            if (Joystick_Button_Read() == 0){
                init_game();
                break;
            }
        }
    }
    int i, a;
    uint8_t button = Joystick_Button_Reg_Read();
    if (button == 0 && !button_pushed){ // Button pressed but not held
        vectorPush(&bullets_vec, player.props.loc, (PairF){BULLET_SPEED*sinf(player.props.angle), -BULLET_SPEED*cosf(player.props.angle)}, 1, 0);
        button_pushed = true; 
    }
    else if (button == 1 && button_pushed) { // Button released after being pushed
        button_pushed = false;   
    }

    // Process bullets and check for collision

    // Because we will be appending newly created astroids to the end of the vector
    // We only iterate over how many astroids we started with in collision detection
    // to prevent messing with memory and values that we're working with. 
    uint8_t current_num_astroids = astroids_vec.used;
    for (i = 0; i < bullets_vec.used; i++){
        SpaceObjBase* b = vectorGet(&bullets_vec, i);
        tftDrawPixel(b->loc.x, b->loc.y, 0);
        // Advance bullet
        b->loc.x = b->loc.x + b->vel.x * delta_ms * TIME_SCALE;
        b->loc.y = b->loc.y + b->vel.y * delta_ms * TIME_SCALE;
        // Check for bullets being out of bounds and mark it for removal
        if (isOutOfBounds(b)){
            bullets_to_remove_index[num_bullets_to_remove++] = i;
        } 
        // If not out of bounds check for collision
        else{
            bool hit_astroid = false;
            for (a = 0; a < current_num_astroids; a++){
                SpaceObjBase* ast = vectorGet(&astroids_vec, a);
                if (isPointInsideCircle(&b->loc, &ast->loc, ast->size)){
                    // Astroid hit, mark both bullet and astroid for removal
                    bullets_to_remove_index[num_bullets_to_remove++] = i;
                    astroids_to_remove_index[num_astroids_to_remove++] = a;
                    hit_astroid = true;
                    score++;
                    
                    if (ast->size > 4){
                        // Create two child astroids at 2 random angles, half the size
                        // Have them inherit the velocity of the parent astroid
                        float ang1 = srandf()*M_PI*2;
                        float ang2 = srandf()*M_PI*2;

                        vectorPush(&astroids_vec, ast->loc, 
                            (PairF){
                                ASTROID_HIT_BULLET_SPEED_INHERIT * b->vel.x + ast->vel.x * ASTROID_INHERIT_SPEED_SCALE + srandf() * ASTROID_SPLIT_SPEED_SCALE * cosf(ang1), 
                                ASTROID_HIT_BULLET_SPEED_INHERIT * b->vel.y + ast->vel.y * ASTROID_INHERIT_SPEED_SCALE + srandf() * ASTROID_SPLIT_SPEED_SCALE * sinf(ang1)
                            },
                            ast->size >> 1, srandf() * M_PI*2);
                         
                        vectorPush(&astroids_vec, ast->loc, 
                            (PairF){
                                ASTROID_HIT_BULLET_SPEED_INHERIT * b->vel.x + ast->vel.x * ASTROID_INHERIT_SPEED_SCALE + srandf() * ASTROID_SPLIT_SPEED_SCALE * cosf(ang2), 
                                ASTROID_HIT_BULLET_SPEED_INHERIT * b->vel.y + ast->vel.y * ASTROID_INHERIT_SPEED_SCALE + srandf() * ASTROID_SPLIT_SPEED_SCALE * sinf(ang2)
                            },
                            ast->size >> 1, srandf() * M_PI*2);
                    }
                }
            }
            // If no hit detected, then draw pixel. 
            if (!hit_astroid) tftDrawPixel(b->loc.x, b->loc.y, 0xFFFF);
        }
    }
    // Clean up bullets and astroids marked for removal
    for (i = 0; i < max(num_bullets_to_remove, num_astroids_to_remove); i++){
        if (i < num_bullets_to_remove && bullets_vec.used > 0){ 
            vectorDelete(&bullets_vec, bullets_to_remove_index[i]);
        };
        if (i < num_astroids_to_remove && astroids_vec.used > 0) {
            SpaceObjBase* a = vectorGet(&astroids_vec, astroids_to_remove_index[i]);
            // Erase the astroid first
            tftDrawWireframe(astroid_model, ASTROID_NUM_VERTS, (int)a->loc.x, (int)a->loc.y, a->angle, a->size, 0);
            vectorDelete(&astroids_vec, astroids_to_remove_index[i]);
        }
    }
    // Reset count
    num_bullets_to_remove = 0;
    num_astroids_to_remove = 0;
    
    if (astroids_vec.used == 0){ // All astroids destroyed
        score += 1000;
        init_astroids(4);
    }
    
    // Read input
    float joy_x = get_input_x();
    float joy_y = get_input_y();
    
    // Process astroids
    for (i = 0; i < astroids_vec.used; i++){
        SpaceObjBase* a = vectorGet(&astroids_vec, i);
        // Let's check collision with the ship while we're at it
        if (isPointInsideCircle(&player.props.loc, &a->loc, a->size)){
            player.shinda = true; // Oof
            break;
        }
        
        tftDrawWireframe(astroid_model, ASTROID_NUM_VERTS, (int)a->loc.x, (int)a->loc.y, a->angle, a->size, 0);
        a->loc.x = a->loc.x + a->vel.x * delta_ms * TIME_SCALE * ASTROID_SPEED_SCALE;
        a->loc.y = a->loc.y + a->vel.y * delta_ms * TIME_SCALE * ASTROID_SPEED_SCALE;
        a->angle += sign(a->angle) * ASTROID_ROTATE_SPEED * delta_ms * TIME_SCALE;
        WrapCoordinateF(a->loc.x, a->loc.y, &(a->loc.x), &(a->loc.y));
        
        tftDrawWireframe(astroid_model, ASTROID_NUM_VERTS, (int)a->loc.x, (int)a->loc.y, a->angle, a->size, 0xFFFF);
    }
    
    // Draw player
    ship_draw(&player, joy_y * PLAYER_SPEED, joy_x * TO_RADIAN * PLAYER_TURN_SPEED, delta_ms * TIME_SCALE);
}

void init_game(void){
    tftClear();
    Game_Timer_Start();
    score = 0;
    
    player.shinda = false;
    player.props.loc = (PairF){120, 170};
    player.props.vel = (PairF){0, 0};
    player.props.angle = 0;
    
    int i;
    for (i = 0; i < ASTROID_NUM_VERTS; i++){
        float angle = ((float) i / (float) ASTROID_NUM_VERTS) * 2 * M_PI;
        astroid_model[i].x = cosf(angle) + srandf()/8;
        astroid_model[i].y = sinf(angle) + srandf()/8;
    }
    memset(bullets, 0, NUM_BULLETS);
    vectorInit(&bullets_vec, NUM_BULLETS, bullets);

    
    memset(astroids, 0, NUM_BULLETS);
    vectorInit(&astroids_vec, NUM_ASTROIDS, astroids);
    
    init_astroids(3);
}

void init(void){
    CyGlobalIntEnable;                      // Enable global interrupts
    SPIM_1_Start();                         // initialize SPIM component 
    GUI_Init();                             // initilize graphics library
    ADC_SAR_1_Start();
    ADC_SAR_1_SetScaledGain(7174); // round(3300*10/4.6)
    Joystick_Mux_Start();
    ADC_SAR_1_StartConvert();
    GUI_SetFont(&GUI_Font8x16);
    GUI_SetColor(GUI_WHITE);
    
    // tftSetArea(0, 0, SCREEN_SIZE_X, SCREEN_SIZE_Y);
    // int i;
    // //tftDrawRect(0,0,100,100,0xFFFF);
    // for (i = 0; i < SCREEN_SIZE_X; i++){
    //     frame_buffer[i] = 0xFFFF;
    // }
    // //memset(frame_buffer, 0xFF, 480);
    // for (i = 10; i < 25; i++){
    //     frame_buffer[i] = 0x0000;
    // }
    // for (i = 25; i < 39; i++){
    //     frame_buffer[i] = 0x001F;
    // }
    // for (i = 39; i < 50; i++){
    //     frame_buffer[i] = 0x0000;
    // }
    // write8_command(0x2C); // Memory Write
    
    
    //DMA
    // DMA_init();
    /* Defines for SPI_DMA */
    // #define SPI_DMA_BYTES_PER_BURST 2
    // #define SPI_DMA_REQUEST_PER_BURST 1
    // #define SPI_DMA_SRC_BASE (CYDEV_SRAM_BASE)
    // #define SPI_DMA_DST_BASE (CYDEV_SRAM_BASE)

    // /* Variable declarations for SPI_DMA */
    // /* Move these variable declarations to the top of the function */
    // uint8 SPI_DMA_Chan;
    // uint8 SPI_DMA_TD[1];

    // /* DMA Configuration for SPI_DMA */
    // SPI_DMA_Chan = SPI_DMA_DmaInitialize(SPI_DMA_BYTES_PER_BURST, SPI_DMA_REQUEST_PER_BURST, 
    //     HI16(frame_buffer), HI16(SPIM_1_TXDATA_PTR));
    // SPI_DMA_TD[0] = CyDmaTdAllocate();
    // CyDmaTdSetConfiguration(SPI_DMA_TD[0], 480, SPI_DMA_TD[0], TD_INC_SRC_ADR);
    // CyDmaTdSetAddress(SPI_DMA_TD[0], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaChSetInitialTd(SPI_DMA_Chan, SPI_DMA_TD[0]);
    // CyDmaChEnable(SPI_DMA_Chan, 1);

    // write8_data(0x00);
    // write8_data(0x00);

    
    init_game();
}

// Initialize a set number of astroids at safe places 
// that wouldn't immediately kill the player
void init_astroids(uint8_t num_astroids){
    int i;
    for (i = 0; i < num_astroids; i++){
        vectorPush(&astroids_vec, 
            (PairF){
                50 + srandf() * 30 * cosf(player.props.angle + (i%2)*M_PI) + player.props.loc.x, 
                50 + srandf() * 30 * sinf(player.props.angle + (i%2)*M_PI) + player.props.loc.y
            },
            (PairF){srandf() * 5 * sinf(srandf() * player.props.angle), srandf() * 5 * cosf(srandf() * player.props.angle)},
            ASTROID_START_SIZE, srandf() * M_PI);
    }
}

void DMA_init(){
    //  /* Defines for SPI_DMA */
    // #define SPI_DMA_BYTES_PER_BURST 127
    // #define SPI_DMA_REQUEST_PER_BURST 1
    // #define SPI_DMA_SRC_BASE (CYDEV_SRAM_BASE)
    // #define SPI_DMA_DST_BASE (CYDEV_SRAM_BASE)

    // /* Variable declarations for SPI_DMA */
    // /* Move these variable declarations to the top of the function */
    // uint8 SPI_DMA_Chan;
    // uint8 SPI_DMA_TD[38];

    // /* DMA Configuration for SPI_DMA */
    // SPI_DMA_Chan = SPI_DMA_DmaInitialize(SPI_DMA_BYTES_PER_BURST, SPI_DMA_REQUEST_PER_BURST, 
    //     HI16(frame_buffer), HI16(SPIM_1_TXDATA_PTR));
    // SPI_DMA_TD[0] = CyDmaTdAllocate();
    // SPI_DMA_TD[1] = CyDmaTdAllocate();
    // SPI_DMA_TD[2] = CyDmaTdAllocate();
    // SPI_DMA_TD[3] = CyDmaTdAllocate();
    // SPI_DMA_TD[4] = CyDmaTdAllocate();
    // SPI_DMA_TD[5] = CyDmaTdAllocate();
    // SPI_DMA_TD[6] = CyDmaTdAllocate();
    // SPI_DMA_TD[7] = CyDmaTdAllocate();
    // SPI_DMA_TD[8] = CyDmaTdAllocate();
    // SPI_DMA_TD[9] = CyDmaTdAllocate();
    // SPI_DMA_TD[10] = CyDmaTdAllocate();
    // SPI_DMA_TD[11] = CyDmaTdAllocate();
    // SPI_DMA_TD[12] = CyDmaTdAllocate();
    // SPI_DMA_TD[13] = CyDmaTdAllocate();
    // SPI_DMA_TD[14] = CyDmaTdAllocate();
    // SPI_DMA_TD[15] = CyDmaTdAllocate();
    // SPI_DMA_TD[16] = CyDmaTdAllocate();
    // SPI_DMA_TD[17] = CyDmaTdAllocate();
    // SPI_DMA_TD[18] = CyDmaTdAllocate();
    // SPI_DMA_TD[19] = CyDmaTdAllocate();
    // SPI_DMA_TD[20] = CyDmaTdAllocate();
    // SPI_DMA_TD[21] = CyDmaTdAllocate();
    // SPI_DMA_TD[22] = CyDmaTdAllocate();
    // SPI_DMA_TD[23] = CyDmaTdAllocate();
    // SPI_DMA_TD[24] = CyDmaTdAllocate();
    // SPI_DMA_TD[25] = CyDmaTdAllocate();
    // SPI_DMA_TD[26] = CyDmaTdAllocate();
    // SPI_DMA_TD[27] = CyDmaTdAllocate();
    // SPI_DMA_TD[28] = CyDmaTdAllocate();
    // SPI_DMA_TD[29] = CyDmaTdAllocate();
    // SPI_DMA_TD[30] = CyDmaTdAllocate();
    // SPI_DMA_TD[31] = CyDmaTdAllocate();
    // SPI_DMA_TD[32] = CyDmaTdAllocate();
    // SPI_DMA_TD[33] = CyDmaTdAllocate();
    // SPI_DMA_TD[34] = CyDmaTdAllocate();
    // SPI_DMA_TD[35] = CyDmaTdAllocate();
    // SPI_DMA_TD[36] = CyDmaTdAllocate();
    // SPI_DMA_TD[37] = CyDmaTdAllocate();
    // CyDmaTdSetConfiguration(SPI_DMA_TD[0], 4095, SPI_DMA_TD[1], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[1], 4095, SPI_DMA_TD[2], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[2], 4095, SPI_DMA_TD[3], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[3], 4095, SPI_DMA_TD[4], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[4], 4095, SPI_DMA_TD[5], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[5], 4095, SPI_DMA_TD[6], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[6], 4095, SPI_DMA_TD[7], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[7], 4095, SPI_DMA_TD[8], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[8], 4095, SPI_DMA_TD[9], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[9], 4095, SPI_DMA_TD[10], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[10], 4095, SPI_DMA_TD[11], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[11], 4095, SPI_DMA_TD[12], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[12], 4095, SPI_DMA_TD[13], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[13], 4095, SPI_DMA_TD[14], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[14], 4095, SPI_DMA_TD[15], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[15], 4095, SPI_DMA_TD[16], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[16], 4095, SPI_DMA_TD[17], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[17], 4095, SPI_DMA_TD[18], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[18], 4095, SPI_DMA_TD[19], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[19], 4095, SPI_DMA_TD[20], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[20], 4095, SPI_DMA_TD[21], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[21], 4095, SPI_DMA_TD[22], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[22], 4095, SPI_DMA_TD[23], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[23], 4095, SPI_DMA_TD[24], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[24], 4095, SPI_DMA_TD[25], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[25], 4095, SPI_DMA_TD[26], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[26], 4095, SPI_DMA_TD[27], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[27], 4095, SPI_DMA_TD[28], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[28], 4095, SPI_DMA_TD[29], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[29], 4095, SPI_DMA_TD[30], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[30], 4095, SPI_DMA_TD[31], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[31], 4095, SPI_DMA_TD[32], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[32], 4095, SPI_DMA_TD[33], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[33], 4095, SPI_DMA_TD[34], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[34], 4095, SPI_DMA_TD[35], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[35], 4095, SPI_DMA_TD[36], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[36], 4095, SPI_DMA_TD[37], TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetConfiguration(SPI_DMA_TD[37], 2085, CY_DMA_DISABLE_TD, TD_INC_SRC_ADR | TD_AUTO_EXEC_NEXT);
    // CyDmaTdSetAddress(SPI_DMA_TD[0], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[1], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[2], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[3], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[4], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[5], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[6], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[7], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[8], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[9], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[10], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[11], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[12], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[13], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[14], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[15], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[16], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[17], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[18], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[19], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[20], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[21], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[22], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[23], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[24], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[25], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[26], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[27], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[28], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[29], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[30], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[31], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[32], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[33], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[34], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[35], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[36], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaTdSetAddress(SPI_DMA_TD[37], LO16((uint32)frame_buffer), LO16((uint32)SPIM_1_TXDATA_PTR));
    // CyDmaChSetInitialTd(SPI_DMA_Chan, SPI_DMA_TD[0]);
    // CyDmaChEnable(SPI_DMA_Chan, 1);
   
}

inline void disp_score(){
    sprintf(print, "Score: %d", score);
    GUI_DispStringAt(print, 0, 0);
}
/* [] END OF FILE */
