#include "camera.h"

#include "config.h"

#include "actor.h"

void updateCamera(Camera *c, const Actor *a, int mw, int mh)
{
    int camCenterX = c->x + SW / 2;
    //int camCenterY = c->y + SH / 2;

    // check if player moved out of the dead zone horizontally
    if (a->body.x < camCenterX - c->deadzoneX)
        c->x = (int)(a->body.x - (SW / 2) + c->deadzoneX);
    else if (a->body.x > camCenterX + c->deadzoneX)
        c->x = (int)(a->body.x - (SW / 2) - c->deadzoneX);

    // check if player moved out of the dead zone vertically
    /*if (a->body.y < camCenterY - c->deadzoneY)
        c->y = a->body.y - (SH / 2) + c->deadzoneY;
    else if (a->body.y > camCenterY + c->deadzoneY)
        c->y = a->body.y - (SH / 2) - c->deadzoneY;*/
}
