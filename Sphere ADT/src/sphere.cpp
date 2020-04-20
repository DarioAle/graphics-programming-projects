#include <stdio.h>
#include "sphere.h"
#include "transforms.h"
#include <time.h>
#include <math.h>

Sphere sphere_create(float radius, short meridians, short parallels, ColorRGB baseColor)
{
    Sphere sph = new SphereStr;
    sph->radius = radius;
    sph->meridians = meridians;
    sph->parallels = parallels;
    sph->baseColor = baseColor;
    sph->indexBuffLoc = 0;
    sph->vertexArray = 0;
    return sph;
}

float* generateVertexCoord(Sphere sph, int& size)
{
    float angle = 0;
    const float angleIncr = 360.0 / sph->meridians;
    
    size = ((sph->meridians + 1) * 3) * sph->parallels;
    float* sphCoord = new float[size];

    int j;
    int posIndex;
    float radius = sph->radius;

    // for(j = 0; j < vasito->stacks; ++j, width += stack_width, radio += radioDecrement ) 
	// {
	// 	for(int i = 0; i < meridians + 1; ++i, angle += angleIncr, ++posIndex) 
	// 	{
	// 		sphCoord[posIndex * 6]     = radio  * cos(to_radians(angle));
	// 		sphCoord[posIndex * 6 + 1] = width;
	// 		sphCoord[posIndex * 6 + 2] = radio * sin(to_radians(angle));

	// 		sphCoord[posIndex * 6 + 3] = (radio + radioDecrement) * cos(to_radians(angle));
	// 		sphCoord[posIndex * 6 + 4] = width + stack_width;
	// 		sphCoord[posIndex * 6 + 5] = (radio + radioDecrement) * sin(to_radians(angle));
	// 	}
	// }

    return sphCoord;
}

float* generateVertexColor(Sphere sph, int& size)
{
    size = ((sph->meridians + 1) * 3) * sph->parallels;
    float* sphColor = new float[size];

    return sphColor;
}

float* generateVertexNormals(Sphere sph, int& size) 
{
    size = ((sph->meridians + 1) * 3) * sph->parallels;
    float* sphNormal = new float[size];

    return sphNormal;
}

GLushort* generateVertexIndices(Sphere sph, int& size) 
{
    size = ((sph->meridians + 1) * 3) * sph->parallels;
    GLushort* sphIndices = new GLushort[size];

    return sphIndices;
}

void sphere_bind(Sphere sph, uint posLoc , uint colLoc, uint normLoc )
{
    int coorSize, colorSize, normSize, indSize;
    float* sphCoor      = generateVertexCoor(sph, coorSize);
	float* sphColor = generateVertexColor(sph, colorSize);
	float* sphNorm	  = generateVertexNormals(sph, normSize, sphCoor);
	GLushort* sphIndices  = generteVertexIndices(sph, indSize);

    glGenVertexArrays(1, &va);
	glBindVertexArray(va);

	glGenBuffers(4, bufferId);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, coorSize * sizeof(float), sphCoor, GL_STATIC_DRAW);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(posLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, colorSize * sizeof(float), sphColor, GL_STATIC_DRAW);
	glVertexAttribPointer(colLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(colLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[2]);
	glBufferData(GL_ARRAY_BUFFER, normSize * sizeof(float), sphNorm, GL_STATIC_DRAW);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(normLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize * sizeof(float), sphIndices, GL_STATIC_DRAW);

	sph->indexBuffLoc = bufferId[3];
	sph->vertexArray = va;
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

}

void sphere_draw(Sphere sph)
{
    glBindVertexArray(sph->vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sph->indexBuffLoc);
	glDrawElements(GL_TRIANGLE_STRIP, (sph->meridians + 1) * (sph->parallels * 2) + sph->parallels, GL_UNSIGNED_SHORT, 0);

}

void sphere_delte(Sphere sph)
{
    delete c;
}
