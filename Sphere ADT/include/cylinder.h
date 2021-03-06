#ifndef CYLINDER_H_
#define CYLINDER_H_

#include <GL/glew.h>
#include <GL/freeglut.h>

typedef float vec3[3];

typedef struct {
    float r;
    float g; 
    float b;
}ColorRGB;

typedef unsigned int uint;

typedef struct {
    float length;
    float bottomRadius;
    float topRadius;
    short sides;
    short stacks;
    uint  indexBuffLoc;
    int vertexArray;
    ColorRGB bottomColor;
    ColorRGB topColor;
}CylinderStr;

typedef CylinderStr* Cylinder;

void crossProduct(vec3 p1, vec3 p2, vec3 p3, vec3 res);

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, short sides, short stacks,
                        ColorRGB bottomColor, ColorRGB topColor);

void cylinder_bind(Cylinder c, uint posLoc , uint colLoc, uint normLoc );

void cylinder_draw(Cylinder c);

void cylinder_delte(Cylinder c);

#endif /* CYLINDER_H_ */
