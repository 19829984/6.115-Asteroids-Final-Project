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
#include "ship.h"
#include "utils.h"
#include "gui.h"

void ship_move_forward(Ship* ship, float thrust, float delta_ms){
    thrust = minf(0, thrust);
    ship->props.vel.x += thrust*cosf(ship->props.angle + M_PI_2); // Because ship's default orientation is up, need to offset
    ship->props.vel.y += thrust*sinf(ship->props.angle + M_PI_2);
    ship->props.loc.x += ship->props.vel.x * delta_ms;
    ship->props.loc.y += ship->props.vel.y * delta_ms;
    WrapCoordinateF(ship->props.loc.x, ship->props.loc.y, &ship->props.loc.x, &ship->props.loc.y);
}

void ship_draw(Ship* ship, float thrust, float rotation, float delta_ms){
    SpaceObjBase* p = &ship->props;
    tftDrawWireframe(ship->points, 3, p->loc.x, p->loc.y, p->angle, p->size, 0);
    p->angle += 2*rotation * delta_ms;
    ship_move_forward(ship, thrust, delta_ms);
    tftDrawWireframe(ship->points, 3, p->loc.x, p->loc.y, p->angle, p->size, 0xFFFF);
}
/* [] END OF FILE */
