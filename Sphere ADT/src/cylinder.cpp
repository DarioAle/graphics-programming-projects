#include <stdio.h>
#include "cylinder.h"
#include "transforms.h"
#include <time.h>
#include <math.h>

Cylinder cylinder_create(float length, float bottomRadius, float topRadius, short sides, short stacks,
                        ColorRGB bottomColor, ColorRGB topColor) 
{
    Cylinder ret = new CylinderStr;
    ret->length = length;
    ret->bottomRadius = bottomRadius;
    ret->topRadius = topRadius;
    ret->sides = sides;
    ret->stacks = stacks;
    ret->bottomColor = bottomColor;
    ret->topColor = topColor; 
	ret->indexBuffLoc = 0;
	ret->vertexArray = 0;
    return ret;
}

float* generateVertexCoor(Cylinder vasito, int& size)
{
	const float meridians = vasito->sides;
	float angle = 0;
	const float angleIncr = 360.0 / meridians;

	const float stack_width = vasito->length / vasito->stacks;

	size = ((meridians + 1) * 3) * (vasito->stacks * 2);
	float* circle = new float[size];

	int j;
	float width = 0.0;
	int posIndex = 0;
	float radio = vasito->bottomRadius;

	float radioDecrement = ((vasito->topRadius - vasito->bottomRadius) / (float) (vasito->stacks - 1));

	for(j = 0; j < vasito->stacks; ++j, width += stack_width, radio += radioDecrement ) 
	{
		for(int i = 0; i < meridians + 1; ++i, angle += angleIncr, ++posIndex) 
		{
			circle[posIndex * 6]     = radio  * cos(to_radians(angle));
			circle[posIndex * 6 + 1] = width;
			circle[posIndex * 6 + 2] = radio * sin(to_radians(angle));

			circle[posIndex * 6 + 3] = (radio + radioDecrement) * cos(to_radians(angle));
			circle[posIndex * 6 + 4] = width + stack_width;
			circle[posIndex * 6 + 5] = (radio + radioDecrement) * sin(to_radians(angle));
		}
	}

	// for(int i = 0; i < (meridians + 1) * vasito->stacks; ++i)
	// 	printf("%.4f %.4f %.4f\n%.4f %.4f %.4f\n\n", 
	// 	circle[i * 6], circle[i * 6 + 1], circle[i * 6 + 2], circle[i * 6 + 3],
	// 	circle[i * 6 + 4],circle[i * 6 + 5]);

	// for(int a = 0; a < meridians * 2 * STACKS + STACKS; a++, indIndex++)
	// 		printf("%d\n",circleIndexes[a]);

	return circle;
}

float* generateVertexColor(Cylinder vasito, int& size) 
{
	srand(time(0));
	size = ((vasito->sides + 1) * 3) * (vasito->stacks * 2);
	float* circleColors = new float[size];

	ColorRGB colorDelta = {	.r = (vasito->topColor.r - vasito->bottomColor.r) / (vasito->stacks - 1),
							.g = (vasito->topColor.g - vasito->bottomColor.g) / (vasito->stacks - 1), 
							.b = (vasito->topColor.b - vasito->bottomColor.b) / (vasito->stacks - 1)};
	
	printf("color decrement %f %f %f\n", colorDelta.r, colorDelta.g, colorDelta.b );
	ColorRGB tempColor = {vasito->bottomColor.r, vasito->bottomColor.g, vasito->bottomColor.b};
	int colIndex = 0;
	int j;

	for(j = 0; j < vasito->stacks; ++j) 
	{
		
		// printf("Temp color %.4f %.4f %.4f\n", tempColor.r, tempColor.g, tempColor.b);
		ColorRGB randColor;
		/* Modulo 256 and then Divide between 256 to get number bwtween 0 and 1*/
		// we want 20% of the random part, 0.20 * 256.0 = 1280.0
		randColor.r = (tempColor.r * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.g = (tempColor.g * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.b = (tempColor.b * 0.8) +  ((rand() & 255) / 1280.0);

		for(int i = 0; i < vasito->sides + 1; ++i, ++colIndex) {
			circleColors[colIndex * 6]     = randColor.r;
			circleColors[colIndex * 6 + 1] = randColor.g;
			circleColors[colIndex * 6 + 2] = randColor.b;

			circleColors[colIndex * 6 + 3]  = randColor.r;
			circleColors[colIndex * 6 + 4] =  randColor.g;
			circleColors[colIndex * 6 + 5] =  randColor.b;
		}

		// Increment color by delta color
		tempColor.r += colorDelta.r;
		tempColor.g += colorDelta.g;
		tempColor.b += colorDelta.b;
	}

	return circleColors;
}

GLushort* generteVertexIndices(Cylinder vasito, int& size)
{	
	size = (vasito->stacks * 2) * (vasito->sides + 1) + vasito->stacks;
	GLushort* circleIndices = new GLushort[size];

	int indIndex = 0;
	int j;
	for(j = 0; j < vasito->stacks; ++j) 
	{
		for(int a = 0; a < (vasito->sides + 1) * 2; a++, indIndex++)
		{
			circleIndices[indIndex + j] = indIndex;
		}
		circleIndices[indIndex + j] = 0xFFFF;
	}
	return circleIndices;
}

void cylinder_bind(Cylinder vasito, uint posLoc , uint colLoc, uint normLoc ) 
{
	uint bufferId[3];
	GLuint va;
     
	int thisSize, thisSize2, thisSize3;
	float* newCircle      = generateVertexCoor(vasito, thisSize);
	float* newCOlorCircle = generateVertexColor(vasito, thisSize2);
	GLushort* newIndices  = generteVertexIndices(vasito, thisSize3);

	glGenVertexArrays(1, &va);
	glBindVertexArray(va);

	glGenBuffers(3, bufferId);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, thisSize * sizeof(float), newCircle, GL_STATIC_DRAW);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(posLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, thisSize2 * sizeof(float), newCOlorCircle, GL_STATIC_DRAW);
	glVertexAttribPointer(colLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(colLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, thisSize3 * sizeof(float), newIndices, GL_STATIC_DRAW);

	vasito->indexBuffLoc = bufferId[2];
	vasito->vertexArray = va;
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

}

void cylinder_draw(Cylinder c)
{
	glBindVertexArray(c->vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->indexBuffLoc);
	glDrawElements(GL_TRIANGLE_STRIP, (c->sides + 1) * (c->stacks * 2) + c->stacks, GL_UNSIGNED_SHORT, 0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, c->sides * 2);
	glDrawArrays(GL_TRIANGLE_FAN, (c->sides + 1) * (c->stacks * 2) - (c->sides * 2) , c->sides * 2);

}

void cylinder_delte(Cylinder c) 
{
	delete c;
}