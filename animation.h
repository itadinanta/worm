#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "vecmath.h"
#include "model.h"
#include "spline.h"
#include "gameboard.h"
#include "mainchar.h"

#include <bitset>
using namespace std;
using namespace spline;
using namespace model;

class Moveable {
public:
	V3 pos;
	V3 vel;
};

#define MAX_JOY_AXIS_COUNT 4
#define JOY_AXIS_X 0
#define JOY_AXIS_Y 1
#define JOY_AXIS_Z 2
#define JOY_AXIS_W 3

class JoyControl {
public:
	FLOAT axis[MAX_JOY_AXIS_COUNT];
	bitset<8> Buttons;
	JoyControl() {memset(axis, 0, sizeof(axis));}
};

class Camera: public Moveable {

};

class Animation {
	enum {CARTOON_LIGHTING_VP, 
	      CARTOON_FILLING_FP,
	      CARTOON_SILHOUETTE_VP,
	      CARTOON_OUTLINE_FP,
	      CARTOON_GROUND_VP,
	      CARTOON_GROUND_FP	};
	Camera fixedCam, *camera;
	Moveable fixedTarget, *lookAt;
	Moveable headPoint;
	FLOAT accum;
	float zoom;
	bool interactiveMode;
	bool fullscreen;
	int Width, Height;
	float testAngle;
	V3 mouse3D;
	int xmouse, ymouse;
	unsigned int hwPrograms[6];
	unsigned int texNames[10];
	unsigned int wormCount;
	const char * vertexProgram;
	const char * fragmentProgram;
protected:
	GameBoard board;
	PositionChain hero;
	Cube cube;
	Cylinder worm;
	Mesh head, tail, eyeL, eyeR;
	Mesh terrain;
	Sphere ball;
	BSplineStrip splinePath;
	void SetCamera();
	char* LoadProgramCode(char * fileName);
	void LoadTexture(char * filename, unsigned int texName);
	void LoadTexture(char * filename, unsigned int texName, unsigned int target);
	void LoadTextureCube(char * filename, unsigned int texName);
	void LoadVertexProgram(char * fileName, unsigned int VP);
	void LoadFragmentProgram(char * fileName, unsigned int FP);
	void ProgramError(char * fileName);
public:
	Animation();
	~Animation();
	JoyControl Control;
	void Timings();
	void Movement();
	void Effects();
	void PaintScene();
	void DrawFrame();
	void initSpline();
	void Init();
	void mouseMove(int x, int y);
	void setAspect(int width, int height);
	void testRotation();
	void outlineMesh(Mesh &m);
	void fillMesh(Mesh &m);
	void renderStage();
	void renderDynamicMesh(Mesh &m);
	void renderGroundMesh(Mesh &m);
	void compileMesh(Mesh &m, unsigned int fillList, unsigned int outlineList);
	void renderCompiledMesh(unsigned int fillList, unsigned int outlineList);
	void wormGrow();
	FLOAT wormScale;
};

#endif
