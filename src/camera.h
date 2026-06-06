#ifndef CAMERA_H
#define CAMERA_H

struct Actor typedef Actor;

struct Camera
{
    int x, y;
    int deadzoneX, deadzoneY;
} typedef Camera;

void updateCamera(Camera *c, const Actor *a, int mw, int mh);

#endif /* CAMERA_H */