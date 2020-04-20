
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Transforms.h"
#include "Utils.h"
#include <stdio.h>
#include <math.h>

#define toRadians(deg) deg * M_PI / 180.0

typedef enum { IDLE, LEFT, RIGHT, UP, DOWN, FRONT, BACK } MOTION_TYPE;

typedef float vec3[3];

static Mat4   modelMatrix, projectionMatrix, viewMatrix;
static GLuint programId1, vertexPositionLoc,  vertexNormalLoc, vertexTexCoordLoc, modelMatrixLoc,  projectionMatrixLoc,  viewMatrixLoc;
static GLuint programId2, vertexPositionLoc2, modelColorLoc2,  modelMatrixLoc2, projectionMatrixLoc2, viewMatrixLoc2;
static GLuint ambientLightLoc, materialALoc, materialDLoc;
static GLuint materialSLoc, cameraPositionLoc;

GLuint cubeVA, roomVA, rhombusVA, rhombusBuffer[3];
GLuint roomBuffers[3];

static MOTION_TYPE motionType      = IDLE;
static MOTION_TYPE lightMotionType = IDLE;

static float cameraSpeed     = 0.05;
static float cameraX         = 0;
static float cameraZ         = 5;
static float cameraAngle     = 0;
static float greenLightX     = 0;
static float greenLightY     = 0;
static float greenLightZ     = 0;
static float greenLightSpeed = 0.10;

static const int ROOM_WIDTH  = 12;
static const int ROOM_HEIGHT =  6;
static const int ROOM_DEPTH  = 40;

static vec3 ambientLight  = {0.5, 0.5, 0.5};

static vec3 materialA     = {0.8, 0.8, 0.8};
static vec3 materialD     = {0.6, 0.6, 0.6};
static vec3 materialS     = {0.6, 0.6, 0.6};

//                          Color    subcutoff,  Position  Exponent Direction  Cos(cutoff)
static float lights[]   = { 1, 0, 0,  0.8660,   -2, 0, 0,  128,	 -1, 0,  0,   0.5,		// Luz Roja
		                    0, 1, 0,  0.9659,    0, 0, 0,  128,   0, 0, -1,   0.866, 	// Luz Verde
		                    0, 0, 1,  0.9238,    2, 0, 0,  128,   1, 0,  0,	  0.7071    // Luz Azul
};


static GLuint lightsBufferId;

static GLuint textures[4];

static void initTexture(const char* filename, GLuint textureId) {
	unsigned char* data;
	unsigned int width, height;
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadBMP(filename, &data, &width, &height);
	// printf("%d, %d\n", width, height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

static void initTextures() {
	glGenTextures(4, textures);
	initTexture("textures/Brick.bmp",   textures[0]);
	initTexture("textures/Ceiling.bmp", textures[1]);
	initTexture("textures/Floor.bmp",   textures[2]);
	initTexture("textures/Square.bmp",  textures[3]);
}


static void initShaders() {
	GLuint vShader = compileShader("shaders/phong.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader)) return;
	GLuint fShader = compileShader("shaders/phong.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader)) return;
	programId1 = glCreateProgram();
	glAttachShader(programId1, vShader);
	glAttachShader(programId1, fShader);
	glLinkProgram(programId1);

	vertexPositionLoc   = glGetAttribLocation(programId1, "vertexPosition");
	vertexNormalLoc     = glGetAttribLocation(programId1, "vertexNormal");
	vertexTexCoordLoc   = glGetAttribLocation(programId1, "vertexTexcoord");
	modelMatrixLoc      = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc       = glGetUniformLocation(programId1, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId1, "projMatrix");
	ambientLightLoc     = glGetUniformLocation(programId1, "ambientLight");
	materialALoc        = glGetUniformLocation(programId1, "materialA");
	materialDLoc        = glGetUniformLocation(programId1, "materialD");
	materialSLoc        = glGetUniformLocation(programId1, "materialS");
	cameraPositionLoc   = glGetUniformLocation(programId1, "cameraPosition");

	vShader = compileShader("shaders/position_mvp.vsh", GL_VERTEX_SHADER);
	if(!shaderCompiled(vShader)) return;
	fShader = compileShader("shaders/modelColor.fsh", GL_FRAGMENT_SHADER);
	if(!shaderCompiled(fShader)) return;
	programId2 = glCreateProgram();
	glAttachShader(programId2, vShader);
	glAttachShader(programId2, fShader);
	glLinkProgram(programId2);

	vertexPositionLoc2   = glGetAttribLocation(programId2, "vertexPosition");
	modelMatrixLoc2      = glGetUniformLocation(programId2, "modelMatrix");
	viewMatrixLoc2       = glGetUniformLocation(programId2, "viewMatrix");
	projectionMatrixLoc2 = glGetUniformLocation(programId2, "projectionMatrix");
	modelColorLoc2       = glGetUniformLocation(programId2, "modelColor");
}

static void initLights() {
	glUseProgram(programId1);
	glUniform3fv(ambientLightLoc,  1, ambientLight);

	glUniform3fv(materialALoc,     1, materialA);
	glUniform3fv(materialDLoc,     1, materialD);
	glUniform3fv(materialSLoc,     1, materialS);

	glGenBuffers(1, &lightsBufferId);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBufferId);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), lights, GL_DYNAMIC_DRAW);

	GLuint uniformBlockIndex = glGetUniformBlockIndex(programId1, "LightBlock");
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsBufferId);
	glUniformBlockBinding(programId1, uniformBlockIndex, 0);
}

static void initLightCubes() {
	float l1 = -0.2, l2 = 0.2;
	float positions[] = {l1, l1, l2, l2, l1, l2, l1, l2, l2, l2, l1, l2, l2, l2, l2, l1, l2, l2,  // Frente
						 l2, l1, l1, l1, l1, l1, l2, l2, l1, l1, l1, l1, l1, l2, l1, l2, l2, l1,  // Atras
						 l1, l1, l1, l1, l1, l2, l1, l2, l1, l1, l1, l2, l1, l2, l2, l1, l2, l1,  // Izquierda
						 l2, l2, l1, l2, l2, l2, l2, l1, l1, l2, l2, l2, l2, l1, l2, l2, l1, l1,  // Derecha
						 l1, l1, l1, l2, l1, l1, l1, l1, l2, l2, l1, l1, l2, l1, l2, l1, l1, l2,  // Abajo
						 l2, l2, l1, l1, l2, l1, l2, l2, l2, l1, l2, l1, l1, l2, l2, l2, l2, l2   // Arriba
	};
	glUseProgram(programId2);
	glGenVertexArrays(1, &cubeVA);
	glBindVertexArray(cubeVA);
	GLuint bufferId;
	glGenBuffers(1, &bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionLoc2, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexPositionLoc2);
}

static void initRoom() {
	float w1 = -ROOM_WIDTH  / 2, w2 = ROOM_WIDTH  / 2;
	float h1 = -ROOM_HEIGHT / 2, h2 = ROOM_HEIGHT / 2;
	float d1 = -ROOM_DEPTH  / 2, d2 = ROOM_DEPTH  / 2;

	float positions[] = {w1, h2, d1, w1, h1, d1, w2, h1, d1,   w2, h1, d1, w2, h2, d1, w1, h2, d1,  // Frente
			             w2, h2, d2, w2, h1, d2, w1, h1, d2,   w1, h1, d2, w1, h2, d2, w2, h2, d2,  // Atr�s
			             w1, h2, d2, w1, h1, d2, w1, h1, d1,   w1, h1, d1, w1, h2, d1, w1, h2, d2,  // Izquierda
			             w2, h2, d1, w2, h1, d1, w2, h1, d2,   w2, h1, d2, w2, h2, d2, w2, h2, d1,  // Derecha
			             w1, h1, d1, w1, h1, d2, w2, h1, d2,   w2, h1, d2, w2, h1, d1, w1, h1, d1,  // Abajo
						 w1, h2, d2, w1, h2, d1, w2, h2, d1,   w2, h2, d1, w2, h2, d2, w1, h2, d2   // Arriba
	};

	float normals[] = { 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  // Frente
						0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  // Atr�s
					    1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  // Izquierda
					   -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,  // Derecha
					    0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  // Abajo
					    0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0   // Arriba
	};

	float wh = (float) ROOM_WIDTH / ROOM_HEIGHT;
	float dh = (float) ROOM_DEPTH / ROOM_HEIGHT;
	float dw = (float) ROOM_DEPTH / ROOM_WIDTH;

	float texcoords[] = {	0, 2,    0, 0,   2 * wh, 0,   2 * wh, 0,   2 * wh, 2,   0, 2,
							0, 2,    0, 0,   2 * wh, 0,   2 * wh, 0,   2 * wh, 2,   0, 2,
							2*dh ,2, 2*dh,0,    0,0,        0,0,          0,2,     2*dh, 2,
							0, 2,    0,0,     2*dh,0,        2*dh,0,   2*dh,2,     0,2,
							0,0,     2*dw,0,     2*dw,2,        2*dw,2,   0,2,     0,0,
							2*dw,0,     0,0,     0,2,        0,2,   2*dw,2,     2*dw,0
						};

	glUseProgram(programId1);
	glGenVertexArrays(1, &roomVA);
	glBindVertexArray(roomVA);
	GLuint buffers[3];
	glGenBuffers(3, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexPositionLoc);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexNormalLoc);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexTexCoordLoc, 2, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexTexCoordLoc);
}

static void crossProduct(vec3 p1, vec3 p2, vec3 p3, vec3 res) {
	vec3 u = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
	vec3 v = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
	res[0] = u[1] * v[2] - u[2] * v[1];
	res[1] = u[2] * v[0] - u[0] * v[2];
	res[2] = u[0] * v[1] - u[1] * v[0];
}

static void initRhombus() {
	float  positions[] = { 0.0, 1.0, 0.6,  -0.7, 0.0, 0.0,  0.7, 0.0, 0.0,  0.0, -1.0, 0.6};
	float  normals[12] = {};
	GLuint indexes[]   = { 0, 1, 2,  1, 3, 2};

	crossProduct(positions, positions + 3, positions + 6, normals);
	crossProduct(positions + 3, positions + 9, positions + 6, normals + 9);
	int i;
	for(i = 0; i < 3; i ++) {
		normals[i + 3] = normals[i] + normals[i + 9];
		normals[i + 6] = normals[i] + normals[i + 9];
	}

	glUseProgram(programId1);
	glGenVertexArrays(1, &rhombusVA);
	glBindVertexArray(rhombusVA);
	glGenBuffers(3, rhombusBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, rhombusBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexPositionLoc);

	glBindBuffer(GL_ARRAY_BUFFER, rhombusBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexNormalLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rhombusBuffer[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
}

static void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	Actualizar posici�n de la c�mara
	float dist = 0.2;
	float w1 = -ROOM_WIDTH  / 2 + dist, w2 = ROOM_WIDTH  / 2 - dist;
	float h1 = -ROOM_HEIGHT / 2 + dist, h2 = ROOM_HEIGHT / 2 - dist;
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

//	Actualizar posici�n de la fuente de luz verde
	switch(lightMotionType) {
		case  LEFT  :  if(greenLightX - greenLightSpeed >  w1) greenLightX -= greenLightSpeed; break;
		case  RIGHT :  if(greenLightX + greenLightSpeed <  w2) greenLightX += greenLightSpeed; break;
		case  UP    :  if(greenLightY + greenLightSpeed <  h2) greenLightY += greenLightSpeed; break;
		case  DOWN  :  if(greenLightY - greenLightSpeed >  h1) greenLightY -= greenLightSpeed; break;
		case  FRONT :  if(greenLightZ - greenLightSpeed >  d1) greenLightZ -= greenLightSpeed; break;
		case  BACK  :  if(greenLightZ + greenLightSpeed <  d2) greenLightZ += greenLightSpeed; break;
		case  IDLE  : ;
	}

//	Env�o de proyecci�n, vista y posici�n de la c�mara al programa 1 (cuarto, rombo)
	glUseProgram(programId1);
	glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, 0, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);
	glUniform3f(cameraPositionLoc, cameraX, 0, cameraZ);

//	Envío de la posici�n de la fuente de luz verde
	lights[16] = greenLightX;
	lights[17] = greenLightY;
	lights[18] = greenLightZ;
	glBindBuffer(GL_UNIFORM_BUFFER, lightsBufferId);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lights), lights, GL_DYNAMIC_DRAW);
	GLuint uniformBlockIndex = glGetUniformBlockIndex(programId1, "LightBlock");
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightsBufferId);
	glUniformBlockBinding(programId1, uniformBlockIndex, 0);

//	Dibujar el cuarto
	mIdentity(&modelMatrix);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	glBindVertexArray(roomVA);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 24, 6);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 30, 6);

//	Dibujar el rombo frontal
	translate(&modelMatrix, 2, 1, -5);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	glBindVertexArray(rhombusVA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rhombusBuffer[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

//	Dibujar el rombo derecho
	mIdentity(&modelMatrix);
	translate(&modelMatrix, ROOM_WIDTH / 2 - 1, 0, 0);
	rotateY(&modelMatrix, -90);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	glBindVertexArray(rhombusVA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rhombusBuffer[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

//	Env�o de proyecci�n y vista al programa 2
	glUseProgram(programId2);
	glBindVertexArray(cubeVA);
	glUniformMatrix4fv(projectionMatrixLoc2, 1, true, projectionMatrix.values);
	glUniformMatrix4fv(viewMatrixLoc2, 1, true, viewMatrix.values);

//	Dibujar fuente de luz roja
	glUniform3f(modelColorLoc2, 1, 0, 0);
	mIdentity(&modelMatrix);
	translate(&modelMatrix, -2, 0, 0);
	glUniformMatrix4fv(modelMatrixLoc2, 1, true, modelMatrix.values);
	glDrawArrays(GL_TRIANGLES, 0, 36);

//	Dibujar fuente de luz verde
	mIdentity(&modelMatrix);
	translate(&modelMatrix, greenLightX, greenLightY, greenLightZ);
	glUniform3f(modelColorLoc2, 0, 1, 0);
	glUniformMatrix4fv(modelMatrixLoc2, 1, true, modelMatrix.values);
	glDrawArrays(GL_TRIANGLES, 0, 36);

//	Dibujar fuente de luz azul
	glUniform3f(modelColorLoc2, 0, 0, 1);
	mIdentity(&modelMatrix);
	translate(&modelMatrix,  2, 0, 0);
	glUniformMatrix4fv(modelMatrixLoc2, 1, true, modelMatrix.values);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glutSwapBuffers();
}

static void reshapeFunc(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    float aspect = (float) w / h;
    setPerspective(&projectionMatrix, 45, aspect, -0.1, -500);
}

static void timerFunc(int id) {
	glutTimerFunc(10, timerFunc, id);
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
	switch(key) {
		case 'a':
		case 'A': lightMotionType = LEFT; break;
		case 'd':
		case 'D': lightMotionType = RIGHT; break;
		case 'w':
		case 'W': lightMotionType = UP; break;
		case 's':
		case 'S': lightMotionType = DOWN; break;
		case 'r':
		case 'R': lightMotionType = FRONT; break;
		case 'f':
		case 'F': lightMotionType = BACK; break;
		case 27 : exit(0);
	}
 }

int main1(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tres luces de colores");
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutSpecialFunc(specialKeyPressedFunc);
    glutSpecialUpFunc(specialKeyReleasedFunc);
    glewInit();
    glEnable(GL_DEPTH_TEST);
    initTextures();
    initShaders();
    initLights();
    initLightCubes();
    initRoom();
    initRhombus();
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glutMainLoop();
	return 0;
}
