/*
 * MousePicking.c
 *
 *  Created on: 08/11/2016
 *      Author: hpiza
 */

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Mat4.h"
#include "Transforms.h"
#include "Utils.h"
#include "Vec3.h"
#include "Vec4.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAMERA_SPEED 0.3
#define MIN_BOX_LENGTH 2
#define MAX_BOX_LENGTH 4
#define BOX_COUNT 200
#define MIN_X -30
#define MAX_X 30
#define MIN_Y -30
#define MAX_Y 30
#define MIN_Z -10
#define MAX_Z -70

#define max(a, b) (a > b? a : b)
#define min(a, b) (a < b? a : b)

GLuint programId, cubeVA;
GLuint vertexPositionLoc, vertexNormalLoc, modelMatrixLoc, projectionMatrixLoc, viewMatrixLoc;
GLuint materialColorLoc;
Mat4 modelMatrix, projectionMatrix, viewMatrix;
int motionType;
float cameraX, cameraY, cameraZ, cameraAngle;

typedef struct {
	Vec3 position;
	Vec3 color;
	Vec3 dimensions;
	bool shot;
} Box;

Box boxArray[BOX_COUNT];

static bool intersectRayBox(Vec3 ray_origin, Vec3 ray_direction, Box box) {
	// ************************
	//	Calcular l�mites (x1, y1, z1), (x2, y2, z2) de la caja
	float x1 = box.position.x - box.dimensions.x / 2;
	float x2 = box.position.x + box.dimensions.x / 2;
	float y1 = box.position.y - box.dimensions.y / 2;
	float y2 = box.position.y + box.dimensions.y / 2;
	float z1 = box.position.z - box.dimensions.z / 2;
	float z2 = box.position.z + box.dimensions.z / 2;

	// Calcular momento de cruce del rayo con cada l�mite de la caja
	float tx1 = (x1 - ray_origin.x) / ray_direction.x;					// tx = [ 7, 15]
	float tx2 = (x2 - ray_origin.x) / ray_direction.x;					// ty = [10, 14]
	float ty1 = (y1 - ray_origin.y) / ray_direction.y;					// tz = [12, 19]
	float ty2 = (y2 - ray_origin.y) / ray_direction.y;
	float tz1 = (z1 - ray_origin.z) / ray_direction.z;
	float tz2 = (z2 - ray_origin.z) / ray_direction.z;

	// Determinar si el rayo no pasa por la caja en (x,y)
	float maxTx = max(tx1, tx2);
	float maxTy = max(ty1, ty2);
	if(maxTx < 0 || maxTy < 0) return false;
	float minTx = min(tx1, tx2);
	float minTy = min(ty1, ty2);
	if(maxTx < minTy || maxTy < minTx) return false;

	// Determinar si el rayo no pasa por la caja en (x,y,z)
	float maxTz = max(tz1, tz2);
	if(maxTz < 0) return false;
	float minTz = min(tz1, tz2);
	if(maxTz < max(minTx, minTy) || minTz > min(maxTx, maxTy)) return false;

	return true;
}

static void mouseFunc(int button, int state, int mx, int my) {
	if(state != GLUT_UP) return;
	// ************************
	// Obtener coordenadas de dispositivo normalizado
	float nx =       2.0 * mx / glutGet(GLUT_WINDOW_WIDTH)  - 1;
	float ny = -1 * (2.0 * my / glutGet(GLUT_WINDOW_HEIGHT) - 1);
//	printf("Normalizado: %.2f, %.2f\n", nx, ny);
	Vec4 rayN = { nx, ny, -1, 1 };

	// (desproyectar) Obtener coordenadas en el sistema de la c�mara
	Mat4 invProjectionMatrix;
	inverse(projectionMatrix, &invProjectionMatrix);
	Vec4 rayV;
 	multiply(invProjectionMatrix, rayN, &rayV);
//	printf("Vista: %.2f, %.2f\n", rayV.x, rayV.y);

	//	Obtener coordenadas en el sistema del mundo
	rayV.z = -1;
	rayV.w =  0;
	Mat4 invViewMatrix;
	inverse(viewMatrix, &invViewMatrix);
	Vec4 rayM;
	multiply(invViewMatrix, rayV, &rayM);
	rayM.w = 0;
	vec4_normalize(&rayM);
	printf("Mundo: %.2f, %.2f, %.2f\n", rayM.x, rayM.y, rayM.z);

	// Obtener la caja m�s cercana con quien hubo colisi�n
	Vec3 ray_origin = { cameraX, cameraY, cameraZ };
	Vec3 ray_direction = { rayM.x, rayM.y, rayM.z };
	int min_index = -1;
	float min_distance = 0;
	int i;
	for(i = 0; i < BOX_COUNT; i ++) {
		Box box = boxArray[i];
		if(box.shot) continue;
		if(!intersectRayBox(ray_origin, ray_direction, box)) continue;
		Vec3 originToBox = { box.position.x - ray_origin.x,
				             box.position.y - ray_origin.y,
							 box.position.z - ray_origin.z};
		float distance = vec3_magnitude(&originToBox);
		if(min_index < 0 || distance < min_distance) {
			min_distance = distance;
			min_index = i;
		}
	}
	if(min_index >= 0) {
		boxArray[min_index].shot = true;
		glutPostRedisplay();
	}
}

static void initShaders() {
	GLuint vShader = compileShader("shaders/cube.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader)) return;

	GLuint fShader = compileShader("shaders/cube.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader)) return;

	GLuint gShader = compileShader("shaders/cube.gsh", GL_GEOMETRY_SHADER);
	if(!shaderCompiled(gShader)) return;

	programId = glCreateProgram();

	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glAttachShader(programId, gShader);
	glLinkProgram(programId);
	vertexPositionLoc   = glGetAttribLocation(programId,  "vertexPosition");
	vertexNormalLoc     = glGetAttribLocation(programId,  "vertexNormal");
	modelMatrixLoc      = glGetUniformLocation(programId, "modelMatrix");
	viewMatrixLoc       = glGetUniformLocation(programId, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId, "projectionMatrix");
	materialColorLoc    = glGetUniformLocation(programId, "materialColor");
}

static void initCube() {
	float l1 = -0.5, l2 = 0.5;
	float positions[] = {l1, l1, l2, l2, l1, l2, l1, l2, l2, l2, l1, l2, l2, l2, l2, l1, l2, l2,  // Frente
						 l2, l1, l1, l1, l1, l1, l2, l2, l1, l1, l1, l1, l1, l2, l1, l2, l2, l1,  // Atr�s
						 l1, l1, l1, l1, l1, l2, l1, l2, l1, l1, l1, l2, l1, l2, l2, l1, l2, l1,  // Izquierda
						 l2, l2, l1, l2, l2, l2, l2, l1, l1, l2, l2, l2, l2, l1, l2, l2, l1, l1,  // Derecha
						 l1, l1, l1, l2, l1, l1, l1, l1, l2, l2, l1, l1, l2, l1, l2, l1, l1, l2,  // Abajo
						 l2, l2, l1, l1, l2, l1, l2, l2, l2, l1, l2, l1, l1, l2, l2, l2, l2, l2   // Arriba
	};

	float normals[] = { 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  // Frente
						0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  // Atr�s
					   -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,  // Izquierda
					    1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  // Derecha
					    0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  // Abajo
					    0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  // Arriba
	};

	glGenVertexArrays(1, &cubeVA);
	glBindVertexArray(cubeVA);
	GLuint buffers[2];
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexPositionLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexPositionLoc);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexNormalLoc);
	glUseProgram(programId);
}

static void initBoxes() {
	srand(time(NULL));
	int i;
	for(i = 0; i < BOX_COUNT; i ++) {
		boxArray[i].position.x = MIN_X + (MAX_X - MIN_X) * rand() / RAND_MAX;
		boxArray[i].position.y = MIN_Y + (MAX_Y - MIN_Y) * rand() / RAND_MAX;
		boxArray[i].position.z = MIN_Z + (MAX_Z - MIN_Z) * rand() / RAND_MAX;
		boxArray[i].color.x = (float) rand() / RAND_MAX;
		boxArray[i].color.y = (float) rand() / RAND_MAX;
		boxArray[i].color.z = (float) rand() / RAND_MAX;
		boxArray[i].dimensions.x = MIN_BOX_LENGTH + (MAX_BOX_LENGTH - MIN_BOX_LENGTH) * rand() / RAND_MAX;
		boxArray[i].dimensions.y = MIN_BOX_LENGTH + (MAX_BOX_LENGTH - MIN_BOX_LENGTH) * rand() / RAND_MAX;
		boxArray[i].dimensions.z = MIN_BOX_LENGTH + (MAX_BOX_LENGTH - MIN_BOX_LENGTH) * rand() / RAND_MAX;
		boxArray[i].shot = false;
	}
}

static void moveForward() {
	cameraZ -= CAMERA_SPEED * cos(cameraAngle / 180 * M_PI);
	cameraX += CAMERA_SPEED * sin(cameraAngle / 180 * M_PI);
}

static void moveBack() {
	cameraZ += CAMERA_SPEED * cos(cameraAngle / 180 * M_PI);
	cameraX -= CAMERA_SPEED * sin(cameraAngle / 180 * M_PI);
}

static void turnRight() {
	cameraAngle ++;
}

static void turnLeft() {
	cameraAngle --;
}

static void moveUp() {
	cameraY += CAMERA_SPEED;
}

static void moveDown() {
	cameraY -= CAMERA_SPEED;
}

static void specialKeyReleasedFunc(int key,int x, int y) {
	motionType = 0;
}

static void keyReleasedFunc(unsigned char key,int x, int y) {
	motionType = 0;
}

static void specialKeyPressedFunc(int key, int x, int y) {
	switch(key) {
		case 101: motionType = 1; break;
		case 103: motionType = 2; break;
		case 102: motionType = 3; break;
		case 100: motionType = 4;
	}
}

static void keyPressedFunc(unsigned char key, int x, int y) {
	if(key == 'u' || key == 'U') motionType = 5;
	else if(key == 'd' || key == 'D') motionType = 6;
	else if (key == 27) {
        exit(0);
    }
}

static void moveOrStay() {
	switch(motionType) {
		case 1: moveForward(); break;
		case 2: moveBack();    break;
		case 3: turnRight();   break;
		case 4: turnLeft();	   break;
		case 5: moveUp();      break;
		case 6: moveDown();
	}
}

static void reshapeFunc(int width, int height) {
	glViewport(0, 0, width, height);
	float ratio = (float) width / height;
	setPerspective(&projectionMatrix, 50, ratio, -1, -500);
}

static void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programId);
	glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
	moveOrStay();
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, cameraAngle);
	translate(&viewMatrix, -cameraX, -cameraY, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);

	int i;
	for(i = 0; i < BOX_COUNT; i ++) {
		mIdentity(&modelMatrix);
		translate(&modelMatrix, boxArray[i].position.x, boxArray[i].position.y, boxArray[i].position.z);
		scale(&modelMatrix, boxArray[i].dimensions.x, boxArray[i].dimensions.y, boxArray[i].dimensions.z);
		glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
		glUniform3f(materialColorLoc, boxArray[i].color.x, boxArray[i].color.y, boxArray[i].color.z);
		glBindVertexArray(cubeVA);
		// if(boxArray[i].shot) continue;
		glUniform1i( glGetUniformLocation(programId, "shot"), boxArray[i].shot);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glutSwapBuffers();
}

static void timerFunc(int id) {
	glutTimerFunc(10, timerFunc, id);
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(600, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Mi primera experiencia con Mouse Picking");
	glutDisplayFunc(displayFunc);
	glutKeyboardFunc(keyPressedFunc);
    glutKeyboardUpFunc(keyReleasedFunc);
    glutSpecialFunc(specialKeyPressedFunc);
    glutSpecialUpFunc(specialKeyReleasedFunc);
    glutReshapeFunc(reshapeFunc);
    glutTimerFunc(10, timerFunc, 1);
    glutMouseFunc(mouseFunc);
    glewInit();
    initShaders();
    initCube();
    initBoxes();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.85, 0.95, 1.0, 1.0);
	glutMainLoop();
	return 0;
}
