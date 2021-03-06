#include <GL/glew.h>
#include <GL/freeglut.h>
#include "transforms.h"
#include "utils.h"
#include "cylinder.h"
#include "sphere.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <time.h>


bool changeDirectionFlag = false;
bool drawSphere = true;
vec3 directionVector = {0.0, 0.0, 0.0};

Sphere pelota;
float radius = 0.3;
short meridians = 40, parallels = 40;
ColorRGB colorBase = {.r = 0.3, .g = 0.8, .b = 0.3 };

Cylinder vasito;
float length, topRadius, bottomRadius;
short sides, stacks;	
ColorRGB ct = {.r = 1.0, .g = 0.0, .b = 0.0}, cb = {.r = 0.0, .g =  0.0, .b = 1.0};

typedef enum { IDLE, LEFT, RIGHT, UP, DOWN, FRONT, BACK } MOTION_TYPE;
typedef float vec3[3];

static Mat4   modelMatrix, projectionMatrix, viewMatrix;

static GLuint programId1, vertexPositionLoc,  vertexNormalLoc, vertexColorLoc;  
static GLuint modelMatrixLoc, projectionMatrixLoc,  viewMatrixLoc;

static GLuint ambientLightLoc, materialALoc, materialDLoc, materialSLoc, cameraPositionLoc;

GLuint roomVA;
GLuint roomBuffers[3];

GLuint bufferId[3];

static MOTION_TYPE motionType      = IDLE;
static MOTION_TYPE lightMotionType = IDLE;

static float cameraSpeed     = 0.05;
static float cameraX         = 0;
static float cameraZ         = 5;
static float cameraAngle     = 0;
static float greenLightX     = 0;
static float greenLightY     = 0;
static float greenLightZ     = 0;
static float particleSpeed = 0.1;

static const int ROOM_WIDTH  = 12;
static const int ROOM_HEIGHT =  6;
static const int ROOM_DEPTH  = 40;

static vec3 ambientLight  = {1.0, 1.0, 1.0};
static vec3 materialA     = {0.5, 0.5, 0.7};
static vec3 materialD     = {0.6, 0.6, 0.6};
static vec3 materialS     = {0.4, 0.4, 0.4};


//                          Color    subcutoff,  Position  Exponent Direction  Cos(cutoff)
static float lights[]   = { 1, 1, 1,  0.8660,   -1, 0, 0,  128,	 -1, 0,  0,   0.5,		// Luz Roja
		                    1, 1, 1,  0.9659,    0, 0, 0,  128,   0, 0, -1,   0.866, 	// Luz Verde
		                    1, 1, 1,  0.9238,    2, 0, 0,  128,   1, 0,  0,	  0.7071    // Luz Azul
};
static GLuint lightsBufferId, lightsBufferId2;

void normalize(vec3 v) 
{
	float magnitude = sqrt(  v[0] * v[0] +
							 v[1] * v[1] + 
						 	 v[2] * v[2]);
	// make unitary the direction vector
	v[0] /= magnitude;
	v[1] /= magnitude;
	v[2] /= magnitude;
} 

float dot(vec3 v1, vec3 v2) 
{
	return v1[0] * v2[0] + v1[1] * v2[1] * v1[2] * v2[2];
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
	vertexColorLoc		 = glGetAttribLocation(programId1, "vertexColor");
	modelMatrixLoc      = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc       = glGetUniformLocation(programId1, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId1, "projMatrix");

	ambientLightLoc     = glGetUniformLocation(programId1, "ambientLight");
	materialALoc        = glGetUniformLocation(programId1, "materialA");
	materialDLoc        = glGetUniformLocation(programId1, "materialD");
	materialSLoc        = glGetUniformLocation(programId1, "materialS");

	cameraPositionLoc   = glGetUniformLocation(programId1, "cameraPosition");

	//	printf("%d, %d, %d, %d, %d\n", vertexPositionLoc, vertexNormalLoc, modelMatrixLoc, viewMatrixLoc, projectionMatrixLoc);	
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

static void initCylinder() {

	float l  = ((rand() % 2001) / 1000.0)+ 0.001;
	float br = ((rand() % 2001) / 1000.0) + 0.001;
	float tr = ((rand() % 2001) / 1000.0) + 0.001;
	short s  =  (short)(rand() % 200) + 2;
	short st =  (short)(rand() % 200) + 2;
	ColorRGB a = {.r = 0.8, .g = 0.3, .b = 0.8};
	ColorRGB b = {.r = 0.3, .g = 0.8, .b = 0.3};

	ColorRGB bot = {.r = ((rand() & 255) / 256.0), .g = ((rand() & 255) / 256.0), .b = ((rand() & 255) / 256.0)};
	ColorRGB top = {.r = ((rand() & 255) / 256.0), .g = ((rand() & 255) / 256.0), .b = ((rand() & 255) / 256.0)};
	vasito = cylinder_create(l, br, tr, s, st, bot, top);
	glUseProgram(programId1);	
	cylinder_bind(vasito, vertexPositionLoc, vertexColorLoc, vertexNormalLoc);
	// vasito = cylinder_create(length, bottomRadius, topRadius, sides, stacks, cb, ct);
	// glUseProgram(programId1);	
	// cylinder_bind(vasito, vertexPositionLoc, vertexColorLoc, vertexNormalLoc);

}

static void initSphere() {
	float r  = ((rand() % 2001) / 1000.0) +0.001;
	short m  =  (short)(rand() % 40) + 2;
	short p =   (short)(rand() % 40) + 2;
	ColorRGB a = {.r = 0.8, .g = 0.3, .b = 0.8};
	ColorRGB bot = {.r = ((rand() & 255) / 256.0), .g = ((rand() & 255) / 256.0), .b = ((rand() & 255) / 256.0)};

	pelota = sphere_create(r, m, p, bot);
	glUseProgram(programId1);
	sphere_bind(pelota, vertexPositionLoc, vertexColorLoc, vertexNormalLoc);
	
}

static void initRoom() {
	float w1 = -ROOM_WIDTH  / 2, w2 = ROOM_WIDTH  / 2;
	float h1 = -ROOM_HEIGHT / 2, h2 = ROOM_HEIGHT / 2;
	float d1 = -ROOM_DEPTH  / 2, d2 = ROOM_DEPTH  / 2;

	float positions[] = {w1, h2, d1, w1, h1, d1, w2, h1, d1,   w2, h1, d1, w2, h2, d1, w1, h2, d1,  // Frente
			             w2, h2, d2, w2, h1, d2, w1, h1, d2,   w1, h1, d2, w1, h2, d2, w2, h2, d2,  // Atrás
			             w1, h2, d2, w1, h1, d2, w1, h1, d1,   w1, h1, d1, w1, h2, d1, w1, h2, d2,  // Izquierda
			             w2, h2, d1, w2, h1, d1, w2, h1, d2,   w2, h1, d2, w2, h2, d2, w2, h2, d1,  // Derecha
			             w1, h1, d1, w1, h1, d2, w2, h1, d2,   w2, h1, d2, w2, h1, d1, w1, h1, d1,  // Abajo
						 w1, h2, d2, w1, h2, d1, w2, h2, d1,   w2, h2, d1, w2, h2, d2, w1, h2, d2   // Arriba
	};

	ColorRGB rc = {.r = 0.5, .g = 0.5, .b = 0.7};
	float colors[] =  {rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,
					   rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,
					   rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,
					   rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,
					   rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,
					   rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b,  rc.r,rc.g,rc.b, rc.r,rc.g,rc.b
					};

	float normals[] = { 0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  // Frente
						0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  // Atrás
					    1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  // Izquierda
					   -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0,  // Derecha
					    0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  // Abajo
					    0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  0, -1,  0,  // Arriba
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(vertexColorLoc, 3, GL_FLOAT, 0, 0, 0);
	glEnableVertexAttribArray(vertexColorLoc);
}

void check_colissions() 
{
		
	float dist = pelota->radius;
	float w1 = -ROOM_WIDTH  / 2 + dist, w2 = ROOM_WIDTH  / 2 - dist;
	float h1 = -ROOM_HEIGHT / 2 + dist, h2 = ROOM_HEIGHT / 2 - dist;
	float d1 = -ROOM_DEPTH  / 2 + dist, d2 = ROOM_DEPTH  / 2 - dist;
	
	// To do calculate reflected ray for this normal
	// r = 2n (n ° l) -
	// r is the reflected ray, in this case must be the new direction vector}

	// printf("collided with left wall\n");
	// vec3 nLeft = {1, 0, 0}; // points to right
	// float temp = dot(nLeft, directionVector) * 2.0;
	// nLeft[0] *= temp;
	// nLeft[1] *= temp;
	// nLeft[2] *= temp;
	// directionVector[0]  = directionVector[0] - nLeft[0];
	// directionVector[1]  = directionVector[1] - nLeft[1];
	// directionVector[2]  = directionVector[2] - nLeft[2];
	// printf("New direction: %.4f %.4f %.4f\n", directionVector[0], directionVector[1], directionVector[2]);
	if(greenLightX <  w1) {
		directionVector[0] *= -1;
	} else if(greenLightX > w2) {
		directionVector[0] *= -1;
	} else if(greenLightY < h1) {
		directionVector[1] *= -1;
	}else if(greenLightY > h2) {
		directionVector[1] *= -1;
	}else if(greenLightZ < d1) {
		directionVector[2] *= -1;
	}else if(greenLightZ > d2) {
		directionVector[2] *= -1;
	}

}

static void displayFunc() {
	srand(time(0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	Actualizar posición de la cámara
	float dist = 0.2;
	float w1 = -ROOM_WIDTH  / 2 + dist, w2 = ROOM_WIDTH  / 2 - dist;
	float h1 = -ROOM_HEIGHT / 2 + dist, h2 = ROOM_HEIGHT / 2 - dist;
	float d1 = -ROOM_DEPTH  / 2 + dist, d2 = ROOM_DEPTH  / 2 - dist;
	
	if(changeDirectionFlag) 
	{
		changeDirectionFlag = false;
		directionVector[0]  =  cos(to_radians((rand() % 181) * 1.0));
		directionVector[1]  =  cos(to_radians((rand() % 181) * 1.0));
		directionVector[2]  =  cos(to_radians((rand() % 181) * 1.0));

		normalize(directionVector);
		// printf("Rand direction: %.4f, %.4f, %.4f\n", directionVector[0], directionVector[1], directionVector[2]);
		
	}

	switch(motionType) {
  		case  LEFT  :  if(cameraX - cameraSpeed >  w1) cameraX -= cameraSpeed; break;
  		case  RIGHT :  if(cameraX + cameraSpeed <  w2) cameraX += cameraSpeed; break;
		case  FRONT :  if(cameraZ - cameraSpeed >  d1) cameraZ -= cameraSpeed; break;
		case  BACK  :  if(cameraZ + cameraSpeed <  d2) cameraZ += cameraSpeed; break;
		case  UP    :
		case  DOWN  :
		case  IDLE  :  ;
	}

	//	Actualizar posición de laesfera
	switch(lightMotionType) {
		case  LEFT  :  if(greenLightX - pelota->radius - particleSpeed >  w1) greenLightX -= particleSpeed; break;
		case  RIGHT :  if(greenLightX + pelota->radius + particleSpeed <  w2) greenLightX += particleSpeed; break;
		case  UP    :  if(greenLightY + pelota->radius + particleSpeed <  h2) greenLightY += particleSpeed; break;
		case  DOWN  :  if(greenLightY - pelota->radius - particleSpeed >  h1) greenLightY -= particleSpeed; break;
		case  FRONT :  if(greenLightZ - pelota->radius - particleSpeed >  d1) greenLightZ -= particleSpeed; break;
		case  BACK  :  if(greenLightZ + pelota->radius + particleSpeed <  d2) greenLightZ += particleSpeed; break;
		case  IDLE  : ;
	}

	//	Envío de proyección, vista y posición de la cámara al programa 1 (cuarto, rombo)
	glUseProgram(programId1);
	glUniformMatrix4fv(projectionMatrixLoc, 1, true, projectionMatrix.values);
	mIdentity(&viewMatrix);
	rotateY(&viewMatrix, -cameraAngle);
	translate(&viewMatrix, -cameraX, 0, -cameraZ);
	glUniformMatrix4fv(viewMatrixLoc, 1, true, viewMatrix.values);
	glUniform3f(cameraPositionLoc, cameraX, 0, cameraZ);

	//	Dibujar el cuarto
	mIdentity(&modelMatrix);
	glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
	glBindVertexArray(roomVA);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	if(drawSphere) 
	{
		// Dibujar esfera movible
		mIdentity(&modelMatrix);
		// to do multiply speed for the unitary direction vector
		greenLightX += directionVector[0] * particleSpeed;
		greenLightY += directionVector[1] * particleSpeed;
		greenLightZ += directionVector[2] * particleSpeed;
		check_colissions();
		translate(&modelMatrix, greenLightX, greenLightY, greenLightZ);
		// translate(&modelMatrix, directionVector[0], directionVector);
		static int angle = 0;
		rotateX(&modelMatrix, angle += 1);
		glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
		sphere_draw(pelota);
	} else {
		// Dibujar cilindro movible
		mIdentity(&modelMatrix);
		translate(&modelMatrix, greenLightX, greenLightY, greenLightZ);
		static int angle = 0;
		rotateX(&modelMatrix, angle += 1);
		glUniformMatrix4fv(modelMatrixLoc, 1, true, modelMatrix.values);
		cylinder_draw(vasito);
	}

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
	srand(time(0));
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
		case 'x': 
		case 'X': changeDirectionFlag = true; break;
		case 'y': 
		case 'Y': drawSphere = !drawSphere; break;
		case 'z': 
		case 'Z': initCylinder();break; 
		case 'q':
		case 'Q': initSphere(); break;			
		case 27 : exit(0);
	}
}

int main(int argc, char **argv) {
	// printf("Length, radio top, radius bottom: \n");
	// scanf("%f %f %f", &length, &topRadius, &bottomRadius);
	// printf("sides and stacks: \n");
	// scanf("%d %d", &sides, &stacks);
	
	printf("\n\n\nInstrucciones: \n");
	printf("Presione las flechas para navegar dentro de la habitacion\n");
	printf("Presiones las teclas a,w,s,d,r,f pra mover el objeto\n");
	printf("Presione \'y\' para intercambiar visualizacion entre cilindro y esfera\n");
	printf("Presione \'z\' para producir un nuevo cilindro\n");
	printf("Presione \'q\' para producir una nueva esferas\n");
	printf("Presione \'x\' para producir un vecotr direccion aleatorio para animarla esfera\n\n\n");

	getchar(); 
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
    
	initShaders();
    initLights();
    initCylinder();
	initSphere();
    initRoom();
    
	glClearColor(0.1, 0.1, 0.1, 1.0);
    glutMainLoop();

	return 0;
}
