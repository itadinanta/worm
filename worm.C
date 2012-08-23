#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#ifdef __WIN32__
#  include <windows.h>
#  undef RGB
#endif

#include "vecmath.h"
#include "spline.h"
#include "animation.h"

using namespace std;


Animation *animation = 0;

static void Reshape(int width, int height) {
	if (!animation) return;
	glViewport(0, 0, (GLint)width, (GLint)height);
	animation->setAspect(width, height);
}

static void Key(unsigned char key, int x, int y) {
	switch (key) {
	case 27: exit(0);
	case ' ':animation->testRotation();
	case 'x':animation->wormGrow();
	}
	glutPostRedisplay();
}

static void KeyUp(unsigned char key, int x, int y) {
	switch (key) {
	case ' ': break;
	}
	glutPostRedisplay();
}

static void setKey(int key) {
	switch (key) {

	}
}

static void FKeyUp(int key, int x, int y)  {
	switch (key) {
	case GLUT_KEY_UP: 
	case GLUT_KEY_DOWN:
		animation->Control.axis[JOY_AXIS_Y] = 0;
		break;
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		animation->Control.axis[JOY_AXIS_X] = 0;
		break;
	case GLUT_KEY_PAGE_UP: 
	case GLUT_KEY_PAGE_DOWN:
		animation->Control.axis[JOY_AXIS_Z] = 0;
		break;
	}
}

static void FKeyDown(int key, int x, int y)  {
	switch (key) {
	case GLUT_KEY_RIGHT:
		animation->Control.axis[JOY_AXIS_X] = 1;
		break;
	case GLUT_KEY_LEFT:
		animation->Control.axis[JOY_AXIS_X] = -1;
		break;
	case GLUT_KEY_UP: 
		animation->Control.axis[JOY_AXIS_Y] = 1;
		break;
	case GLUT_KEY_DOWN:
		animation->Control.axis[JOY_AXIS_Y] = -1;
		break;
	case GLUT_KEY_PAGE_UP: 
		animation->Control.axis[JOY_AXIS_Z] = 1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		animation->Control.axis[JOY_AXIS_Z] = -1;
		break;
	case GLUT_KEY_INSERT:
		animation->Control.Buttons.flip(0);
		break;
	case GLUT_KEY_HOME:
		animation->wormScale *= 1.1;
		break;
	}
}

static void Auto() {
	animation->testRotation();
	glutPostRedisplay();
}

static void FMouse(int x, int y) {
	animation->mouseMove(x,y);
	glutPostRedisplay();
}

static void Draw(void) {
	if (!animation) return;	
	animation->Timings();
	animation->Movement();
	animation->Effects();
	animation->DrawFrame();
}

int main(int argc, char **argv) {
	GLenum type;
	bool fullscreen = false;
	glutInit(&argc, argv);
	for (int i=0; i<argc; i++) {
		if (!strcmp(argv[i],"-fullscreen"))
			fullscreen = true;
	}
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	if (fullscreen) {
		glutGameModeString("800x600:24");
		glutEnterGameMode();
	}
	else {
	glutInitWindowPosition(0,0);
	glutInitWindowSize( 640, 480);
	if (glutCreateWindow("flex") == GL_FALSE)
		exit(1);
	}
	animation = new Animation();
	animation->Init();
#ifndef PS2
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(FKeyUp);
	glutMotionFunc(FMouse);
//	glutPassiveMotionFunc(FMouse);
#endif
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(FKeyDown);
	glutDisplayFunc(Draw);
	glutIdleFunc(Auto);
	glutMainLoop();
	if (fullscreen)
		glutLeaveGameMode();
	return 0;
}
