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
    return ret;
}


void cylinder_bind(Cylinder vasito, uint posLoc , uint colLoc, uint normLoc ) 
{
	uint bufferId[3];
    	srand(time(0)); 
	// printf("%d\n", rand());
	const float meridians = vasito->sides;
	float angle = 0;
	const float total_degrees = 360.0;
	const float increment = total_degrees / meridians;
	const float stack_width = vasito->length / vasito->stacks;


	// Every stack has twice the meridians up and down.
	int size = ((meridians + 1) * 3) * (vasito->stacks * 2);
	int numberOfIndexes = (vasito->stacks * 2) * (meridians + 1) + vasito->stacks;

	float* circle = new float[size];
	float* circleColors = new float[size];
	GLushort* circleIndexes = new GLushort[numberOfIndexes];
	
	// printf("Value of pi: %f, size of circle: %d\n", M_PI, sizeof(circle));

	int posIndex = 0;
	int colIndex = 0;
	int indIndex = 0;
	int j;
	float width = 0.0;
	float radio = vasito->bottomRadius;

	float radioDecrement = ((vasito->topRadius - vasito->bottomRadius) / (float) (vasito->stacks - 1));
	printf("radio decrement %f\n", radioDecrement );
	ColorRGB colorDelta = {	.r = (vasito->topColor.r - vasito->bottomColor.r) / (vasito->stacks - 1),
							.g = (vasito->topColor.g - vasito->bottomColor.g) / (vasito->stacks - 1), 
							.b = (vasito->topColor.b - vasito->bottomColor.b) / (vasito->stacks - 1)};
	
	printf("color decrement %f %f %f\n", colorDelta.r, colorDelta.g, colorDelta.b );
	ColorRGB tempColor = {vasito->bottomColor.r, vasito->bottomColor.g, vasito->bottomColor.b};
	
	for(j = 0; j < vasito->stacks; ++j, width += stack_width, radio += radioDecrement ) {

		for(int i = 0; i < meridians + 1; ++i, angle += increment, ++posIndex) {
			circle[posIndex * 6]     = radio  * cos(to_radians(angle));
			circle[posIndex * 6 + 1] = width;
			circle[posIndex * 6 + 2] = radio * sin(to_radians(angle));


			circle[posIndex * 6 + 3] = (radio + radioDecrement) * cos(to_radians(angle));
			circle[posIndex * 6 + 4] = width + stack_width;
			circle[posIndex * 6 + 5] = (radio + radioDecrement) * sin(to_radians(angle));
			// radio -= radioDecrement;
			// printf("degrees: %f\n", angle);
			// printf("%.2f %.2f %.2f\n", circle[i * 3], circle[i * 3 + 1], circle[i * 3 + 2]);
		}
		
		// printf("Temp color %.4f %.4f %.4f\n", tempColor.r, tempColor.g, tempColor.b);
		ColorRGB randColor;
		/* Modulo 256 and then Divide between 256 to get number bwtween 0 and 1*/
		// we want 20% of the random part, 0.20 * 256.0 = 1280.0
		randColor.r = (tempColor.r * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.g = (tempColor.g * 0.8) +  ((rand() & 255) / 1280.0);
		randColor.b = (tempColor.b * 0.8) +  ((rand() & 255) / 1280.0);

		for(int i = 0; i < meridians + 1; ++i, ++colIndex) {
			circleColors[colIndex * 6]     = randColor.r;
			circleColors[colIndex * 6 + 1] = randColor.g;
			circleColors[colIndex * 6 + 2] = randColor.b;

			circleColors[colIndex * 6 + 3]  = randColor.r;
			circleColors[colIndex * 6 + 4] =  randColor.g;
			circleColors[colIndex * 6 + 5] =  randColor.b;
		}

		for(int a = 0; a < (meridians + 1) * 2; a++, indIndex++){
			circleIndexes[indIndex + j] = indIndex;
		}
		circleIndexes[indIndex + j] = 0xFFFF;

		// Increment color by delta color
		tempColor.r += colorDelta.r;
		tempColor.g += colorDelta.g;
		tempColor.b += colorDelta.b;

	}

	

	// for(int i = 0; i < (meridians + 1) * vasito->stacks; ++i)
	// 	printf("%.4f %.4f %.4f\n%.4f %.4f %.4f\n\n", 
	// 	circle[i * 6], circle[i * 6 + 1], circle[i * 6 + 2], circle[i * 6 + 3],
	// 	circle[i * 6 + 4],circle[i * 6 + 5]);

	// for(int a = 0; a < meridians * 2 * STACKS + STACKS; a++, indIndex++)
	// 		printf("%d\n",circleIndexes[a]);

	glGenBuffers(3, bufferId);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), circle, GL_STATIC_DRAW);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(posLoc);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), circleColors, GL_STATIC_DRAW);
	glVertexAttribPointer(colLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(colLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numberOfIndexes * sizeof(float), circleIndexes, GL_STATIC_DRAW);

	vasito->indexBuffLoc = bufferId[2];
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

}

void cylinder_draw(Cylinder c)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c->indexBuffLoc);
	glDrawElements(GL_TRIANGLE_STRIP, (c->sides + 1) * (c->stacks * 2) + c->stacks, GL_UNSIGNED_SHORT, 0);
}

void cylinder_delte(Cylinder c) 
{
	delete c;
}