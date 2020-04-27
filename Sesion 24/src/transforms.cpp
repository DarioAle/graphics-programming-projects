#include "transforms.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct strNode
{
    struct strNode *below;
    Mat4 *data;
};

typedef struct strNode *Node;

Node top = NULL;

void translate(Mat4 *csMatrix, float tx, float ty, float tz)
{
    Mat4 trMatrix;
    mIdentity(&trMatrix);
    trMatrix.at[0][3] = tx;
    trMatrix.at[1][3] = ty;
    trMatrix.at[2][3] = tz;
    mMult(csMatrix, trMatrix);
    // mPrint(*csMatrix);
}

void rotateX(Mat4 *csMatrix, float degrees)
{
    Mat4 rotMatrix;
    mIdentity(&rotMatrix);
    float radians = degrees * M_PI / 180;
    rotMatrix.at[1][1] = cos(radians);
    rotMatrix.at[1][2] = -sin(radians);
    rotMatrix.at[2][1] = sin(radians);
    rotMatrix.at[2][2] = cos(radians);

    mMult(csMatrix, rotMatrix);
    // mPrint(*csMatrix);
}

void rotateY(Mat4 *csMatrix, float degrees)
{
    Mat4 rotMatrix;
    mIdentity(&rotMatrix);
    float radians = degrees * M_PI / 180;
    rotMatrix.at[0][0] = cos(radians);
    rotMatrix.at[0][2] = sin(radians);
    rotMatrix.at[2][0] = -sin(radians);
    rotMatrix.at[2][2] = cos(radians);

    mMult(csMatrix, rotMatrix);
    // mPrint(*csMatrix);
}

void rotateZ(Mat4 *csMatrix, float degrees)
{
    Mat4 rotMatrix;
    mIdentity(&rotMatrix);
    float radians = degrees * M_PI / 180;
    rotMatrix.at[0][0] = cos(radians);
    rotMatrix.at[0][1] = -sin(radians);
    rotMatrix.at[1][0] = sin(radians);
    rotMatrix.at[1][1] = cos(radians);

    mMult(csMatrix, rotMatrix);
    // mPrint(*csMatrix);
}

void scale(Mat4 *csMatrix, float sx, float sy, float sz)
{
    Mat4 scMatrix;
    mIdentity(&scMatrix);
    scMatrix.at[0][0] = sx;
    scMatrix.at[1][1] = sy;
    scMatrix.at[2][2] = sz;

    mMult(csMatrix, scMatrix);
    // mPrint(*csMatrix);
}

void emptyStack()
{
    Node current = top;
    while (current != NULL)
    {
        Node tmp = current->below;
        free(current->data);
        free(current);
        current = tmp;
    }
    top = NULL;
}

void loadIdentity(Mat4 *csMatrix)
{
    mIdentity(csMatrix);
    emptyStack();
}

void pushMatrix(Mat4 *csMatrix)
{
    Node node = (Node)malloc(sizeof(struct strNode));
    Mat4 *m = (Mat4 *)malloc(sizeof(Mat4));
    for (int i = 0; i < 16; i++)
    {
        m->values[i] = csMatrix->values[i];
    }
    node->data = m;
    node->below = top;
    top = node;
}

void popMatrix(Mat4 *csMatrix)
{
    if (top == NULL)
        return;
    for (int i = 0; i < 16; i++)
    {
        csMatrix->values[i] = top->data->values[i];
    }
    Node toPop = top;
    top = top->below;
    free(toPop->data);
    free(toPop);
}

void setOrtho(Mat4 *m, float left, float right, float bottom, float top, float far, float near)
{
    mIdentity(m);
    m->at[0][0] = 2 / (right - left);
    m->at[1][1] = 2 / (top - bottom);
    m->at[2][2] = 2 / (near - far);

    m->at[0][3] = -(right + left) / (right - left);
    m->at[1][3] = -(top + bottom) / (top - bottom);
    m->at[2][3] = -(near + far) / (near - far);
}

void setPerspective(Mat4 *m, float fovy, float aspect, float nearZ, float farZ)
{
    mIdentity(m);
    float radians = fovy * M_PI / 180;
    float tanF = tanf(radians / 2);
    m->at[0][0] = 1.0 / (aspect * tanF);
    m->at[1][1] = 1.0 / tanF;
    m->at[2][2] = (farZ + nearZ) / (nearZ - farZ);
    m->at[2][3] = (-2 * nearZ * farZ) / (nearZ - farZ);
    m->at[3][2] = -1;
    m->at[3][3] = 0;

}


float det2x2(float m[4]) {
	return m[0] * m[3] - m[1] * m[2];
}

float det3x3(float m[9]) {
	float m1[] = {m[4], m[5], m[7], m[8]};
    float d1 = m[0] * det2x2(m1);
    float m2[] = {m[3], m[5], m[6], m[8]};
    float d2 = m[1] * det2x2(m2);
    float m3[] = {m[3], m[4], m[6], m[7]};
    float d3 = m[2] * det2x2(m3);
    return d1 - d2 + d3;
}

float determinant(Mat4 m) {
	float m1[] = {m.at[1][1], m.at[2][1], m.at[3][1], m.at[1][2], m.at[2][2], m.at[3][2], m.at[1][3], m.at[2][3], m.at[3][3]};
    float d1 = m.at[0][0] * det3x3(m1);
    float m2[] = {m.at[0][1], m.at[2][1], m.at[3][1], m.at[0][2], m.at[2][2], m.at[3][2], m.at[0][3], m.at[2][3], m.at[3][3]};
    float d2 = m.at[1][0] * det3x3(m2);
    float m3[] = {m.at[0][1], m.at[1][1], m.at[3][1], m.at[0][2], m.at[1][2], m.at[3][2], m.at[0][3], m.at[1][3], m.at[3][3]};
    float d3 = m.at[2][0] * det3x3(m3);
    float m4[] = {m.at[0][1], m.at[1][1], m.at[2][1], m.at[0][2], m.at[1][2], m.at[2][2], m.at[0][3], m.at[1][3], m.at[2][3]};
    float d4 = m.at[3][0] * det3x3(m4);
    return d1 - d2 + d3 - d4;
}

void transpose(Mat4 s, Mat4* t) {
	int r, c;
    for(r = 0; r < 4; r ++) {
    	for(c = 0; c < 4; c ++) {
    		t->at[c][r] = s.at[r][c];
    	}
    }
}

void adjoint(Mat4 s, Mat4* t) {
	int r, c, rr, cc;
	for(r = 0; r < 4; r ++) {
		for(c = 0; c < 4; c ++) {
			float m3[9];
			int i = 0;
			for(rr = 0; rr < 4; rr ++) {
				if(rr == r) continue;
				for(cc = 0; cc < 4; cc ++) {
					if(cc == c) continue;
					m3[i ++] = s.at[cc][rr];
				}
			}
			int sign = (c + r) % 2 == 0? 1 : -1;
			t->at[c][r] = sign * det3x3(m3);
		}
	}
}

void inverse(Mat4 s, Mat4* t) {
	Mat4 adj;
	adjoint(s, &adj);
	transpose(adj, t);
	float d = determinant(s);
	int i, j;
	for(i = 0; i < 4; i ++) {
		for(j = 0; j < 4; j ++) {
			t->at[i][j] /= d;
		}
	}
}
