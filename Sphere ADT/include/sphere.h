#ifndef SPHERE_H_
#define SPHERE_H_

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cylinder.h"

typedef struct {
    float radius;
    short meridians;
    short parallels;
    uint  indexBuffLoc;
    int vertexArray;
    ColorRGB baseColor;
}SphereStr;

typedef SphereStr* Sphere;


Sphere sphere_create(float radius, short meridians, short parallels, ColorRGB baseColor);

void sphere_bind(Sphere sph, uint posLoc , uint colLoc, uint normLoc );

void sphere_draw(Sphere sph);

void sphere_delte(Sphere sph);

#endif /* CYLINDER_H_ */
