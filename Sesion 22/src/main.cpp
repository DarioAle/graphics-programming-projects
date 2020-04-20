
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "transforms.h"
#include "utils.h"
#include <stdio.h>
#include <math.h>

#define toRadians(deg) deg * M_PI / 180.0

typedef enum { IDLE, LEFT, RIGHT, UP, DOWN, FRONT, BACK } MOTION_TYPE;

typedef float vec3[3];

static Mat4   modelMatrix, projectionMatrix, viewMatrix;
static GLuint programId1, positionLoc1,  texcoordLoc1, modelMatrixLoc1, projectionMatrixLoc1,  viewMatrixLoc1;
static GLuint programId2, positionLoc2,  texcoordLoc2, timeLoc2, windowSizeLoc2;
static GLuint programId3;

static GLuint roomVA, starVA, waterVA, boxVA;
static GLuint roomBuffers[3];
static GLuint boxBuffers[3];

static GLfloat time = 0;

static MOTION_TYPE motionType      = IDLE;
static MOTION_TYPE lightMotionType = IDLE;

static float cameraSpeed     = 0.05;
static float cameraX         = 0;
static float cameraZ         = 5;
static float cameraAngle     = 0;

static const int ROOM_WIDTH  = 12;
static const int ROOM_HEIGHT =  6;
static const int ROOM_DEPTH  = 40;


static void initShaders() {
	GLuint vShader1 = compileShader("shaders/pos_tex_mvp.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader1)) return;
	GLuint fShader1 = compileShader("shaders/texture.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader1)) return;
	programId1 = glCreateProgram();
	glAttachShader(programId1, vShader1);
	glAttachShader(programId1, fShader1);
	glLinkProgram(programId1);

	positionLoc1         = glGetAttribLocation(programId1, "position");
	texcoordLoc1         = glGetAttribLocation(programId1, "texcoord");
	modelMatrixLoc1      = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc1       = glGetUniformLocation(programId1, "viewMatrix");
	projectionMatrixLoc1 = glGetUniformLocation(programId1, "projectionMatrix");

	GLuint vShader2 = compileShader("shaders/water.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader2)) return;
	GLuint fShader2 = compileShader("shaders/water.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader2)) return;
	programId2 = glCreateProgram();
	glAttachShader(programId2, vShader2);
	glAttachShader(programId2, fShader2);
	glLinkProgram(programId2);
	positionLoc2       = glGetAttribLocation(programId2, "position");
	texcoordLoc2       = glGetAttribLocation(programId2, "texcoord");
	timeLoc2           = glGetUniformLocation(programId2, "time");
	windowSizeLoc2     = glGetUniformLocation(programId2, "windowSize");

	GLuint fShader3 = compileShader("shaders/box.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader3)) return;
	programId3 = glCreateProgram();
	glAttachShader(programId3, vShader1);
	glAttachShader(programId3, fShader3);
	glLinkProgram(programId3);
}

static GLuint textures[4];

static void initTexture(const char* filename, GLuint textureId) {
	unsigned char* data;
	unsigned int width, height;
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadBMP(filename, &data, &width, &height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

}

static void initTextures() {
	glGenTextures(4, textures);
	initTexture("textures/Pool.bmp",   textures[0]);
	initTexture("textures/Star.bmp",   textures[1]);
	initTexture("textures/Water1.bmp", textures[2]);
	initTexture("textures/Water2.bmp", textures[3]);
}

static void initStar() {
	float positions[] = { -1, -1, 0,  -1, 1, 0,   1, -1, 0,  1, 1, 0 };
	float texcoords[] = {  0, 0,   0, 1,   1, 0,  1, 1  };
	glUseProgram(programId1);
	glGenVertexArrays(1, &starVA);
	glBindVertexArray(starVA);
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc1, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(positionLoc1);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(texcoordLoc1, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(texcoordLoc1);
}

static void initWater() {
	float positions[] = { -1, -1, 0,  -1, 1, 0,   1, -1, 0,  1, 1, 0 };
	float texcoords[] = {  0, 0,   0, 4,   4, 0,  4, 4  };
	glUseProgram(programId2);
	glGenVertexArrays(1, &waterVA);
	glBindVertexArray(waterVA);
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc2, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(positionLoc2);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(texcoordLoc2, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(texcoordLoc2);
}

static void initBox() {
	float min = -0.5, max = 0.5;

	float positions[] = { min, min, min,   min, max, min,   max, min, min,   max, max, min, 		// Frente
						  max, min, max,   max, max, max,   min, min, max,   min, max, max, 		// Atr�s
			              min, min, max,   min, max, max,   min, min, min,   min, max, min, 		// Izquierda
						  max, min, min,   max, max, min,   max, min, max,   max, max, max   		// Derecha
	};
	float texcoords[] = {  0, 0,   0, 1,   1, 0,   1, 1,   // Frente
						   0, 0,   0, 1,   1, 0,   1, 1,   // Atr�s
						   0, 0,   0, 1,   1, 0,   1, 1,   // izquierda
						   0, 0,   0, 1,   1, 0,   1, 1    // Derecha
	};
	GLushort indices[]  = {  0, 1, 2, 3, 0xFFFF,  4, 5, 6, 7, 0xFFFF, 8, 9, 10, 11, 0xFFFF, 12, 13, 14, 15 };
	glUseProgram(programId3);
	glGenVertexArrays(1, &boxVA);
	glBindVertexArray(boxVA);
	glGenBuffers(3, boxBuffers);

	glBindBuffer(GL_ARRAY_BUFFER, boxBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc1, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(positionLoc1);

	glBindBuffer(GL_ARRAY_BUFFER, boxBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(texcoordLoc1, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(texcoordLoc1);

	glPrimitiveRestartIndex(0xFFFF);
	glEnable(GL_PRIMITIVE_RESTART);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxBuffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

static void initRoom() {
	float w1 = -ROOM_WIDTH  / 2, w2 = ROOM_WIDTH  / 2;
	float h1 = -ROOM_HEIGHT / 2, h2 = ROOM_HEIGHT / 2;
	float d1 = -ROOM_DEPTH  / 2, d2 = ROOM_DEPTH  / 2;

	float positions[] = {w1, h2, d1, w1, h1, d1, w2, h1, d1,   w2, h1, d1, w2, h2, d1, w1, h2, d1,  // Frente
			             w2, h2, d2, w2, h1, d2, w1, h1, d2,   w1, h1, d2, w1, h2, d2, w2, h2, d2,  // Atr�s
			             w1, h2, d2, w1, h1, d2, w1, h1, d1,   w1, h1, d1, w1, h2, d1, w1, h2, d2,  // Izquierda
			             w2, h2, d1, w2, h1, d1, w2, h1, d2,   w2, h1, d2, w2, h2, d2, w2, h2, d1,  // Derecha
			             w1, h1, d1, w1, h1, d2, w2, h1, d2,   w2, h1, d2, w2, h1, d1, w1, h1, d1   // Abajo
	};

	float wh = (float) ROOM_WIDTH / ROOM_HEIGHT;
	float dh = (float) ROOM_DEPTH / ROOM_HEIGHT;
	float wd = (float) ROOM_WIDTH  / ROOM_DEPTH;

	float texcoords[] = {           0, 2,       0, 0,    2 * wh, 0,   2 * wh, 0,   2 * wh, 2,       0, 2,
					           2 * wh, 2,  2 * wh, 0,         0, 0,        0, 0,        0, 2,  2 * wh, 2,

							        0, 2,       0, 0,    2 * dh, 0,   2 * dh, 0,   2 * dh, 2,       0, 2,
							   2 * dh, 2,  2 * dh, 0,         0, 0,        0, 0,        0, 2,  2 * dh, 2,

							 	    0, 0,    0, 2*dh,   2*dh*wd, 2*dh,  2*dh*wd, 2*dh,  2*dh*wd, 0,  0, 0
	};

	glUseProgram(programId1);
	glGenVertexArrays(1, &roomVA);
	glBindVertexArray(roomVA);
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(positionLoc1, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(positionLoc1);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(texcoordLoc1, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(texcoordLoc1);
}

static void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	Actualizar posici�n de la c�mara
	float dist = 0.2;
	float w1 = -ROOM_WIDTH  / 2 + dist, w2 = ROOM_WIDTH  / 2 - dist;
	float d1 = -ROOM_DEPTH  / 2 + dist, d2 = ROOM_DEPTH  / 2 - dist;
	switch(motionType) {
  		case  LEFT  :  if(cameraX - cameraSpeed >  w1) cameraX -= cameraSpeed; break;
  		case  RIGHT :  if(cameraX + cameraSpeed <  w2) cameraX += cameraSpeed; break;
		case  FRONT :  if(cameraZ - cameraSpeed >  d1) cameraZ -= cameraSpeed; break;
		case  BACK  :  if(cameraZ + cameraSpeed <  d2) cameraZ += cameraSpeed; break;
		case  UP    :
		case  DOWN  :
		case  IDLE  :  ;
	}

	//	Env�o de proyecci�n, vista y posici�n de la c�mara
	glUseProgram(programId1);
	glUniformMatrix4fv(projectionMatrixLoc1, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, 0, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc1, 1, true, viewMatrix.values);

//	Dibujar el cuarto
	mIdentity(&modelMatrix);
	glUniformMatrix4fv(modelMatrixLoc1, 1, true, modelMatrix.values);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glBindVertexArray(roomVA);
	glDrawArrays(GL_TRIANGLES, 0, 30);

//	Dibujar la estrella
//	mIdentity(&modelMatrix);
//	translate(&modelMatrix, 3, 1, -14);
//	static float starAngle = 0, incAngle = 0.5;
//	rotateZ(&modelMatrix, starAngle);
//	starAngle += incAngle;
//	if(starAngle >= 45 || starAngle <= -45) incAngle *= -1;
//	scale(&modelMatrix, 0.8, 0.8, 1);
//	glUniformMatrix4fv(modelMatrixLoc1, 1, true, modelMatrix.values);
//	glBindTexture(GL_TEXTURE_2D, textures[1]);
//	glBindVertexArray(starVA);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Dibujar la caja
	glUseProgram(programId3);
	glUniformMatrix4fv(projectionMatrixLoc1, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, 0, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc1, 1, true, viewMatrix.values);
	mIdentity(&modelMatrix);
	translate(&modelMatrix, -3, 0, -10);
	static float boxAngle = 0;
	rotateY(&modelMatrix, boxAngle += 0.5);
	rotateZ(&modelMatrix, boxAngle);
	glUniformMatrix4fv(modelMatrixLoc1, 1, true, modelMatrix.values);
	glBindVertexArray(boxVA);
	glDrawElements(GL_TRIANGLE_STRIP, 19, GL_UNSIGNED_SHORT, 0);

//	Dibujar el agua
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glUseProgram(programId2);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programId2, "texture0"), 0);
	glBindTexture(GL_TEXTURE_2D, textures[2]);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(programId2, "texture1"), 1);
	glBindTexture(GL_TEXTURE_2D, textures[3]);

	glBindVertexArray(waterVA);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);

	glutSwapBuffers();
}

static void reshapeFunc(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    float aspect = (float) w / h;
    setPerspective(&projectionMatrix, 45, aspect, -0.1, -500);
    glUniform2f(windowSizeLoc2, (float) w, (float) h);
}

static void timerFunc(int id) {
	glutTimerFunc(10, timerFunc, id);

	glUseProgram(programId2);
	glUniform1f(timeLoc2, time += 0.01);

	glutPostRedisplay();
}

static void specialKeyReleasedFunc(int key,int x, int y) {
	motionType = IDLE;
}

static void keyReleasedFunc(unsigned char key,int x, int y) {
	lightMotionType = IDLE;
}

static void specialKeyPressedFunc(int key, int x, int y) {
	switch(key) {
		case 100: motionType = LEFT;  break;
		case 102: motionType = RIGHT; break;
		case 101: motionType = FRONT; break;
		case 103: motionType = BACK;
	}
}

static void keyPressedFunc(unsigned char key, int x, int y) {
	if(key == 27) exit(0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Alberca");
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutKeyboardFunc(keyPressedFunc);
    glutSpecialFunc(specialKeyPressedFunc);
    glutSpecialUpFunc(specialKeyReleasedFunc);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    initShaders();
    initTextures();
    initRoom();
    initStar();
    initWater();
    initBox();
    glClearColor(0.7, 1.0, 1.0, 1.0);
    glutMainLoop();
	return 0;
}
