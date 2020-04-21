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
    float theta = 0.0;
	float phi 	= 0.0;

    float thetaIncrement = 2 * M_PI / (float)sph->meridians;
	float phiIncrement   = M_PI / (float)sph->parallels;
    

    size = ((sph->meridians + 1) * 3) * sph->parallels * 2;
    float* sphCoord = new float[size];
	
    int j;
    int posIndex = 0;

    for(j = 0; j < sph->parallels; ++j, phi += phiIncrement) 
	{
		theta = 0.0;
		for(int i = 0; i < sph->meridians + 1; ++i, theta += thetaIncrement, ++posIndex) 
		{
			sphCoord[posIndex * 6]     = sph->radius * sin(phi)  * cos(theta);
			sphCoord[posIndex * 6 + 1] = sph->radius * cos(phi);
			sphCoord[posIndex * 6 + 2] = sph->radius * sin(phi)  * sin(theta);

			float tempPhi = phi + phiIncrement;
			sphCoord[posIndex * 6 + 3] = sph->radius * sin(tempPhi) * cos(theta);
			sphCoord[posIndex * 6 + 4] = sph->radius * cos(tempPhi);
			sphCoord[posIndex * 6 + 5] = sph->radius * sin(tempPhi) * sin(theta);
		}
	}
    return sphCoord;
}

float* generateVertexColor(Sphere sph, int& size)
{
	srand(time(0));
    size = ((sph->meridians + 1) * 3) * sph->parallels * 2;
    float* sphColor = new float[size];
	
	int j;
	int colorIndex = 0;
	for(j = 0; j < sph->parallels; ++j) 
	{	
		/* Modulo 256 and then Divide between 256 to get number bwtween 0 and 1*/
		// we want 20% of the random part, 0.20 * 256.0 = 1280.0

		ColorRGB randColor;
		randColor.r = (sph->baseColor.r * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.g = (sph->baseColor.g * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.b = (sph->baseColor.b * 0.8) +  ((rand() & 255) / 1280.0);

		for(int i = 0; i < sph->meridians + 1; ++i, ++colorIndex) 
		{	
			sphColor[colorIndex * 6] 	 = randColor.r;
			sphColor[colorIndex * 6 + 1] = randColor.g;
			sphColor[colorIndex * 6 + 2] = randColor.b;

			sphColor[colorIndex * 6 + 3] = randColor.r;
			sphColor[colorIndex * 6 + 4] = randColor.g;
			sphColor[colorIndex * 6 + 5] = randColor.b;
		}
	}

    return sphColor;
}

GLuint* generateVertexIndices(Sphere sph, int& size) 
{
    size = ((sph->meridians + 1) * 3) * (sph->parallels *  2) + sph->parallels;
    GLuint* sphIndices = new GLuint[size];

	int indIndex = 0;
	int j;
	for(j = 0; j < sph->parallels; ++j) 
	{
		for(int i = 0; i < (sph->meridians + 1) * 2; ++i, indIndex++)
		{
			sphIndices[indIndex + j] = indIndex;
		}
		sphIndices[indIndex + j] = 0xFFFF;
	}

    return sphIndices;
}

void sphere_bind(Sphere sph, uint posLoc , uint colLoc, uint normLoc )
{
	uint bufferId[4];
	GLuint va;

    int coorSize, colorSize, indSize;
    float* sphCoor       = generateVertexCoord(sph, coorSize);
	float* sphColor 	 = generateVertexColor(sph, colorSize);
	GLuint* sphIndices = generateVertexIndices(sph, indSize);
	

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
	glBufferData(GL_ARRAY_BUFFER, coorSize * sizeof(float), sphCoor, GL_STATIC_DRAW);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(normLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize * sizeof(int), sphIndices, GL_STATIC_DRAW);

	sph->indexBuffLoc = bufferId[3];
	sph->vertexArray = va;
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

}

void sphere_draw(Sphere sph)
{
    glBindVertexArray(sph->vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sph->indexBuffLoc);
	glDrawElements(GL_TRIANGLE_STRIP, (sph->meridians + 1) * (sph->parallels * 2) + sph->parallels, GL_UNSIGNED_INT, 0);

}

void sphere_delte(Sphere sph)
{
    delete sph;
}
