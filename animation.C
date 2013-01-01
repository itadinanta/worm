#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "animation.h"
#include "spline.h"
#include "texture_storage.h"
#include "math.h"
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

namespace spline {
	enum {
		MESH_ZERO,
		MESH_CUBE_FILL, MESH_CUBE_OUTLINE,
		MESH_BALL_FILL, MESH_BALL_OUTLINE,	 
		MESH_FRAMEH_FILL, MESH_FRAMEH_OUTLINE,
		MESH_FRAMEV_FILL, MESH_FRAMEV_OUTLINE,
		MESH_FRAMEUL_FILL, MESH_FRAMEUL_OUTLINE,
		MESH_FRAMEUR_FILL, MESH_FRAMEUR_OUTLINE,
		MESH_FRAMEBR_FILL, MESH_FRAMEBR_OUTLINE,
		MESH_FRAMEBL_FILL, MESH_FRAMEBL_OUTLINE,
		MESH_FRAMEVR_FILL, MESH_FRAMEVR_OUTLINE,
		MESH_FRAMEHT_FILL, MESH_FRAMEHT_OUTLINE,
		MESH_FRAMEVL_FILL, MESH_FRAMEVL_OUTLINE,
		MESH_FRAMEHB_FILL, MESH_FRAMEHB_OUTLINE,
		MESH_HEAD_FILL, MESH_HEAD_OUTLINE,
		MESH_EYEL_FILL, MESH_EYEL_OUTLINE,
		MESH_EYER_FILL, MESH_EYER_OUTLINE,
		MESH_TAIL_FILL, MESH_TAIL_OUTLINE
	};

	inline FLOAT randr(float min, float max) {
		return min+(max-min)*((float)rand() / (float)RAND_MAX);
	}
	
	inline bool prob(int num, int den=100) {
		return (rand()%den) < num;
	}

	using namespace std;	
	using namespace texture;
	char * Animation::LoadProgramCode(char* fileName) {
		char * code = 0;
		struct stat statBuf;
		if (!stat(fileName,&statBuf)) {
			int pLen = statBuf.st_size;
			FILE *f=fopen(fileName,"rb");
			if (f) {
				code=(char*)malloc(pLen+1);
				if (code && !fread((void*)code,pLen,1,f)) {
					free(code);
					code=0;
				} else code[pLen]=0;
				fclose(f);
			}
		}
		if (!code)
			cerr << "Error loading program "<< fileName << "\n";
		return code;
	}

	void Animation::ProgramError(char * fileName) {
		int errorPos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		cerr << "[glProgramStringARB] in " << fileName <<", "
		     << glGetString(GL_PROGRAM_ERROR_STRING_ARB) 
		     << " at position "
		     << errorPos
		     << '\n';
		exit(1);	
	}

	void Animation::LoadVertexProgram(char * fileName, unsigned int VP) {
		if (vertexProgram = LoadProgramCode(fileName)) {
			glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VP);
			glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vertexProgram), vertexProgram);
			if (glGetError() == GL_INVALID_OPERATION)
				ProgramError(fileName);
			else {
				glEnable(GL_VERTEX_PROGRAM_ARB);
				cerr << "Vertex Program loaded successfully\n";
			}
		}
		free((void*)vertexProgram);		
		vertexProgram=0;
	};

	void Animation::LoadFragmentProgram(char * fileName, unsigned int FP) {
		
		if (fragmentProgram=LoadProgramCode(fileName)) {
			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FP);
			glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB, 
					   strlen(fragmentProgram), fragmentProgram);
			if (glGetError() == GL_INVALID_OPERATION) {
				ProgramError(fileName);
			}
			else {
				glEnable(GL_FRAGMENT_PROGRAM_ARB);
				cerr << "Fragment Program loaded successfully\n";
			}
		}
		free((void*)fragmentProgram);
		fragmentProgram=0;
	};

	void Animation::Timings() {

	}

	void Animation::Movement() {
		if (!Control.Buttons.test(0)) {
			const FLOAT responsiveness=0.1f*wormScale; // parametri empirici, da scalare
			const FLOAT smoothness=1.0f-wormScale/2;
			const FLOAT speed=wormScale;
			headPoint.vel.x+=Control.axis[JOY_AXIS_X]*responsiveness;
			if (headPoint.vel.x>2) headPoint.vel.x=2;
			else if (headPoint.vel.x<-2) headPoint.vel.x=-2;
			if (!Control.axis[JOY_AXIS_X]) headPoint.vel.x*=smoothness;
			
			if (headPoint.vel.z>2) headPoint.vel.z=2;
			else if (headPoint.vel.z<-2) headPoint.vel.z=-2;
			
			headPoint.vel.z-=Control.axis[JOY_AXIS_Y]*responsiveness;
			if (!Control.axis[JOY_AXIS_Y]) headPoint.vel.z*=smoothness;

			headPoint.vel.normalize();
			V3 vel=headPoint.vel;
			vel*=speed;
			headPoint.pos+=vel;
			Normal N;
			if (board.Y(headPoint.pos,N))
				headPoint.pos.y+=1.5;
			fixedTarget.pos = headPoint.pos;
			fixedTarget.pos.z = headPoint.pos.z-1;
			fixedTarget.pos.x = headPoint.pos.x;
			V3 UpVector=fixedTarget.pos; UpVector+=N;
//			fixedTarget.pos.y = 0;
			hero.Move(fixedTarget.pos, vel, UpVector);
		}
		FLOAT x0 = 5;
		FLOAT x1 = 145;
		FLOAT z0 = 25;
		FLOAT z1 = 175;

		FLOAT tx = headPoint.pos.x/150;
		FLOAT tz = headPoint.pos.z/150;
		if (tx<0) tx=0; else if (tx>1) tx=1;
		if (tz<0) tz=0; else if (tz>1) tz=1;
		fixedCam.pos.y+=Control.axis[JOY_AXIS_Z];
		fixedCam.pos.x=x0+tx*(x1-x0);
		fixedCam.pos.z=z0+tz*(z1-z0);
	}

	void Animation::Effects() {

	}

	void Animation::mouseMove(int x, int y) {
		xmouse = x;
		ymouse = y;
	}

	void Animation::SetCamera() {
		glProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, 
			camera->pos.x, camera->pos.y, camera->pos.z, 1.0);
		if (!lookAt || lookAt==camera)
			gluLookAt(camera->pos.x,
				  camera->pos.y,
				  camera->pos.z,
				  camera->pos.x+camera->vel.x,
				  camera->pos.y+camera->vel.y,
				  camera->pos.z+camera->vel.z,
				  0, 1 ,0);
		else
			gluLookAt(camera->pos.x,
				  camera->pos.y,
				  camera->pos.z,
				  lookAt->pos.x,
				  lookAt->pos.y,
				  lookAt->pos.z,
				  0, 1 ,0);
		FLOAT viewmat[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, viewmat);
		glMatrixMode(GL_MATRIX0_ARB);
		glLoadMatrixf(viewmat);
		glMatrixMode(GL_MODELVIEW);
	}

	void Animation::initSpline() {
		splinePath.clear();
		const int npts = 10;
		V3 evpt, uppt, dirpt,leftpt;
		const FLOAT ray=10;
		FLOAT angle=randr(0,2*M_PI);
		for (int i=0; i<npts; i++) {			
			evpt.set(sin(angle)*ray,
				 randr(-2,2),
				 cos(angle)*ray);
			uppt=evpt; uppt.y+=1;
			splinePath.add(evpt, uppt);
			angle+=M_PI/2;
		}
		splinePath.regenerate();
	}

	void Animation::fillMesh(Mesh &m) {
		PrimitiveArray::iterator pIter;
		for (pIter=m.Primitives.begin(); pIter!=m.Primitives.end(); pIter++) {
			Primitive *p=(*pIter);
			Index vCount=p->getVertexCount();
			Index vertexList[vCount];
			Index normalList[vCount];
			Index attributesList[vCount];			
			p->getVertexList(vertexList);
			p->getNormalList(normalList);
			p->getAttributesList(attributesList);
			switch (p->getPrimType()) {
			case PRIM_POLY: glBegin(GL_TRIANGLES); // bah!
				break;
			case PRIM_STRIP: glBegin(GL_TRIANGLE_STRIP);
				break;
			case PRIM_FAN: glBegin(GL_TRIANGLE_FAN);
				break;
			default: glBegin(GL_TRIANGLES);
				// thou shalt not enter this!
				break;
			}			
			for (int i=0; i<vCount; i++) {
				if (!i || (normalList[i]!=normalList[i-1])) {
					if (normalList[i]>=0)
						glNormal3fv((GLfloat*)(&m.Normals[normalList[i]]));
				}
				if (!i || (attributesList[i]!=attributesList[i-1])) {
					if (attributesList[i]>=0) {						
						Attributes &a=m.Attributes[attributesList[i]];
						if (a.flags & ATTRIB_COLOR)
							glColor3ubv((GLubyte*)(&(a.Color)));
						if (a.flags & ATTRIB_TEXCOORD0)
							glMultiTexCoord3fv(GL_TEXTURE0,(GLfloat*)(&(a.TexCoord[0])));
						if (a.flags & ATTRIB_TEXCOORD1)
							glMultiTexCoord3fv(GL_TEXTURE1,(GLfloat*)(&(a.TexCoord[1])));
						if (a.flags & ATTRIB_TEXCOORD2)
							glMultiTexCoord3fv(GL_TEXTURE2,(GLfloat*)(&(a.TexCoord[2])));
						if (a.flags & ATTRIB_TEXCOORD3)
							glMultiTexCoord3fv(GL_TEXTURE3,(GLfloat*)(&(a.TexCoord[3])));
					}
				}
				if (vertexList[i]>=0) {
					V3 &V=m.Vertices[vertexList[i]];
					glVertex3fv((GLfloat*)(&V));
				}
			}
			glEnd();			
		}
	}

	void Animation::outlineMesh(Mesh &m) {
		glBegin(GL_LINES);
		V3 n0d( 0, 0, 0);
		V3 n1d( 1, 1, 1);
		bool drawEdge;
		Index iV1, iV2;
		for (EdgeArray::iterator edge=m.Edges.begin(); edge!=m.Edges.end(); edge++) {
			drawEdge=true;
			if (edge->isCulled()) continue;
			V3 &a=m.Vertices[edge->Vertex[0]];
			V3 &b=m.Vertices[edge->Vertex[1]];
			if (edge->isVisible()) {
				glNormal3fv((GLfloat*)&n0d); // forces visibility
				glVertex3fv((GLfloat*)&a);
				glVertex3fv((GLfloat*)&b);
			} 
			else if ((iV1=edge->CompVertex[0])>=0 && (iV2=edge->CompVertex[1])>=0) {
				glNormal3fv((GLfloat*)&n1d); // forces comparison
				// Use only the first point to compute visibility
				glVertexAttrib3fvARB(9, (GLfloat*)&a); 
				glVertexAttrib3fvARB(10, (GLfloat*)&b); 
				if (iV1>=0)
					glVertexAttrib3fvARB(11, (GLfloat*)&m.Vertices[iV1]);
				if (iV2>=0)
					glVertexAttrib3fvARB(12, (GLfloat*)&m.Vertices[iV2]);
				glVertex3fv((GLfloat*)&a);
				glVertex3fv((GLfloat*)&b);
			}
		}
		glEnd();

	};

	void Animation::compileMesh(Mesh &m, unsigned int fillList, unsigned int outlineList) {
		glNewList(fillList, GL_COMPILE);
		fillMesh(m);
		glEndList();

		glNewList(outlineList, GL_COMPILE);
		outlineMesh(m);
		glEndList();
	};

	void Animation::renderCompiledMesh(unsigned int fillList, unsigned int outlineList) {
		// Filling
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_LIGHTING_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_FILLING_FP]);
		glDepthMask(GL_TRUE);
		glCallList(fillList);

		// Outline
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_SILHOUETTE_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_OUTLINE_FP]);	
		glLineWidth(3.0);
		glDepthMask(GL_FALSE);
		glCallList(outlineList);
	};

	void Animation::renderGroundMesh(Mesh &m) {
		// replace all with a smarter Octree renderer

		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_GROUND_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_GROUND_FP]);
		
		// Filling
		glDepthMask(GL_TRUE);
		fillMesh(m);

		// Outline
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_SILHOUETTE_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_OUTLINE_FP]);

		glLineWidth(4.0);
		glDepthMask(GL_FALSE);
		outlineMesh(m);
	};

	void Animation::renderDynamicMesh(Mesh &m) {
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_LIGHTING_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_FILLING_FP]);
		
		// Filling
		glDepthMask(GL_TRUE);
		fillMesh(m);
		// Outline
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, hwPrograms[CARTOON_SILHOUETTE_VP]);
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, hwPrograms[CARTOON_OUTLINE_FP]);

		glLineWidth(4.0);
		glDepthMask(GL_FALSE);
		outlineMesh(m);
	}

	void Animation::renderStage() {
		int i,j;
		renderGroundMesh(terrain);
		/*
		glColor3ub(127,127,255);
		for (i=0; i<board.Cells.getRows(); i++) 
			for (j=0; j<board.Cells.getCols(); j++) 
				if (abs(fixedTarget.pos.z/5 - i) <10 && abs(fixedTarget.pos.x/5 - j) <10) {
					glPushMatrix();
					glTranslatef(j*5,-5,i*5);
					switch (board.Cells[i][j].item) {
					case ' ':
					case '+':
					case '#': renderCompiledMesh(MESH_CUBE_FILL, MESH_CUBE_OUTLINE); break;
					case '-': renderCompiledMesh(MESH_FRAMEH_FILL, MESH_FRAMEH_OUTLINE); break;
					case '|': renderCompiledMesh(MESH_FRAMEV_FILL, MESH_FRAMEV_OUTLINE); break;

					case '/': renderCompiledMesh(MESH_FRAMEUL_FILL, MESH_FRAMEUL_OUTLINE); break;
					case 'L': renderCompiledMesh(MESH_FRAMEBL_FILL, MESH_FRAMEBL_OUTLINE); break;
					case '7': renderCompiledMesh(MESH_FRAMEUR_FILL, MESH_FRAMEUR_OUTLINE); break;
					case '\'': renderCompiledMesh(MESH_FRAMEBR_FILL, MESH_FRAMEBR_OUTLINE); break;

					case '^': renderCompiledMesh(MESH_FRAMEHT_FILL, MESH_FRAMEHT_OUTLINE); break;
					case 'T': renderCompiledMesh(MESH_FRAMEHB_FILL, MESH_FRAMEHB_OUTLINE); break;
					case 'E': renderCompiledMesh(MESH_FRAMEVR_FILL, MESH_FRAMEVR_OUTLINE); break;
					case '3': renderCompiledMesh(MESH_FRAMEVL_FILL, MESH_FRAMEVL_OUTLINE); break;
						
					}
					glPopMatrix();
				}
		*/
		
	}

	void Animation::PaintScene() {
		int i,j,k; int n=5;
		glPushMatrix();
		GLdouble model[16], proj[16];
		GLint view[4];
		GLdouble X,Y,Z;
		glGetDoublev(GL_MODELVIEW_MATRIX, model);
		glGetDoublev(GL_PROJECTION_MATRIX, proj);
		glGetIntegerv(GL_VIEWPORT, view);
		gluUnProject(xmouse, Height-ymouse, 0.95, model, proj, view, &X, &Y, &Z);
		mouse3D.set(X, Y, Z);
		glBindTexture(GL_TEXTURE_2D, texNames[3]);
		renderStage();
		glColor3ub(255,0,0);
		int ninter = worm.getSections();
		M44 Mdef;
		V3Vector V3V;
		V3 evpt, uppt;

		const int wSectors = worm.getSectors();
		worm.Attributes.clear();
		RGBA col=head.Attributes[0].Color;
		worm.addAttributes(col);
		worm.addAttributes(col);
		for (i=0; i<=wSectors; i++) {
			V3V.push_back(V3(-sin(i*(2*M_PI)/(FLOAT)wSectors)*1.5,
					 cos(i*(2*M_PI)/(FLOAT)wSectors)*1.5,
					 0));
		}
		Index strip[wSectors*2+2];
		V4 nV;
		V4 nO;
		V3 nN;
		
	        int step;
		PositionChainCursor cc(&hero);
		int bigSegmentOffset = 0;
		for (int segment=0; segment<wormCount; segment++) {
			worm.Vertices.clear();
			worm.Normals.clear();
			for (step=0; step<=worm.getSections(); step++) {
				FLOAT tC=step+bigSegmentOffset;
				tC=cc.seekDist(tC);			
				hero.getMatrix(tC,Mdef);			
				for (i=0; i<wSectors; i++) {
					nO=V3V[i];
					if (!step || step==worm.getSections()) 
						nO*=0.8;
					Mdef.mult(nV, nO);
					worm.addVertex(nV);
					Mdef.mult3(nN,V3V[i]);
					nN.normalize();
					worm.addNormal(nN);
				}
			}
			bigSegmentOffset+=worm.getSections();
			renderDynamicMesh(worm);		
		}

		glPushMatrix();
//		hero.getMatrix(0,Mdef);			
		Mdef.transpose();
		glMultMatrixf((FLOAT*)(&Mdef));
//		glRotatef(180,0,1,0);
		renderCompiledMesh(MESH_TAIL_FILL, MESH_TAIL_OUTLINE);
		glPopMatrix();
		
		glPushMatrix();
		hero.getMatrix(0,Mdef);			
		Mdef.transpose();
		glMultMatrixf((FLOAT*)(&Mdef));
//		glRotatef(180,0,1,0);
		renderCompiledMesh(MESH_HEAD_FILL, MESH_HEAD_OUTLINE);
//		renderCompiledMesh(MESH_TAIL_FILL, MESH_TAIL_OUTLINE);
		glBindTexture(GL_TEXTURE_2D, texNames[1]);
		renderCompiledMesh(MESH_EYEL_FILL, MESH_EYEL_OUTLINE);
		renderCompiledMesh(MESH_EYER_FILL, MESH_EYER_OUTLINE);
		glPopMatrix();
	}

	void Animation::DrawFrame() {	
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		float fov = 45.0/zoom;
		float fcp = 1;
		gluPerspective(fov, (GLfloat)Width/(GLfloat)Height, fcp, 1000);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		SetCamera();
		GLfloat lightPos[]={fixedTarget.pos.x-3,fixedTarget.pos.y+20,fixedTarget.pos.z,1};
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		PaintScene();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// Do billboarding here
	
		glFlush();
		glutSwapBuffers();
	}

	void Animation::Init() {

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE0);
		glEnable(GL_TEXTURE1);
		glEnable(GL_TEXTURE2);
		glEnable(GL_TEXTURE3);
		glEnable(GL_BLEND);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);	
		glCullFace(GL_BACK);
		glShadeModel(GL_SMOOTH);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);

		GLfloat fspec[4]={1,1,1,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fspec);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fspec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 12);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LINE_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.0, 1, 1, 0.0);
		glClearAccum(0.0, 0.0, 0.0, 0.0);
		glClear(GL_ACCUM_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		glGenProgramsARB(6,hwPrograms);	
		glGenTextures(10,texNames);
		LoadVertexProgram("cartoon_silhouette.vp", hwPrograms[CARTOON_SILHOUETTE_VP]);
		LoadFragmentProgram("cartoon_outline.fp", hwPrograms[CARTOON_OUTLINE_FP]);
		LoadVertexProgram("cartoon_lighting.vp", hwPrograms[CARTOON_LIGHTING_VP]);
		LoadFragmentProgram("cartoon_filling.fp", hwPrograms[CARTOON_FILLING_FP]);
		LoadVertexProgram("cartoon_ground.vp", hwPrograms[CARTOON_GROUND_VP]);
		LoadFragmentProgram("cartoon_ground.fp", hwPrograms[CARTOON_GROUND_FP]);
		glActiveTextureARB(GL_TEXTURE2_ARB);
		LoadTextureCube("reflect_%s.png", texNames[2]);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		LoadTexture("test1.png", texNames[0]);
		LoadTexture("images/terrain.png", texNames[3]);
		LoadTexture("images/eyeball.png", texNames[1]);
		glActiveTextureARB(GL_TEXTURE3_ARB);
		glBindTexture(GL_TEXTURE_2D, texNames[0]);
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glBindTexture(GL_TEXTURE_2D, texNames[0]);
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, texNames[0]);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		compileMesh(cube, MESH_CUBE_FILL, MESH_CUBE_OUTLINE);
		compileMesh(ball, MESH_BALL_FILL, MESH_BALL_OUTLINE);
		compileMesh(head, MESH_HEAD_FILL, MESH_HEAD_OUTLINE);
		compileMesh(eyeL, MESH_EYEL_FILL, MESH_EYEL_OUTLINE);
		compileMesh(eyeR, MESH_EYER_FILL, MESH_EYER_OUTLINE);
		compileMesh(tail, MESH_TAIL_FILL, MESH_TAIL_OUTLINE);
		int c=6;
		FLOAT r=2.5;
		CylDome vMesh(c, r, r, 1, 0);
		CylDome hMesh(c, r, r, 1, 1);
		CylDome ulMesh(c, r, r, 2, 0);
		CylDome urMesh(c, r, r, 2, 1);
		CylDome brMesh(c, r, r, 2, 2);
		CylDome blMesh(c, r, r, 2, 3);
		CylDome vrMesh(c, r, r, 3, 0);
		CylDome hbMesh(c, r, r, 3, 1);
		CylDome vlMesh(c, r, r, 3, 2);
		CylDome htMesh(c, r, r, 3, 3);
		compileMesh(vMesh, MESH_FRAMEV_FILL, MESH_FRAMEV_OUTLINE);
		compileMesh(hMesh, MESH_FRAMEH_FILL, MESH_FRAMEH_OUTLINE);
		compileMesh(ulMesh, MESH_FRAMEUL_FILL, MESH_FRAMEUL_OUTLINE);
		compileMesh(blMesh, MESH_FRAMEBL_FILL, MESH_FRAMEBL_OUTLINE);
		compileMesh(brMesh, MESH_FRAMEBR_FILL, MESH_FRAMEBR_OUTLINE);
		compileMesh(urMesh, MESH_FRAMEUR_FILL, MESH_FRAMEUR_OUTLINE);
		compileMesh(vrMesh, MESH_FRAMEVR_FILL, MESH_FRAMEVR_OUTLINE);
		compileMesh(hbMesh, MESH_FRAMEHB_FILL, MESH_FRAMEHB_OUTLINE);
		compileMesh(vlMesh, MESH_FRAMEVL_FILL, MESH_FRAMEVL_OUTLINE);
		compileMesh(htMesh, MESH_FRAMEHT_FILL, MESH_FRAMEHT_OUTLINE);
	}
	
	void Animation::LoadTexture(char * filename, GLuint texName, GLuint target) {
		glBindTexture(target, texName);
		PngLoader tex(filename);
		gluBuild2DMipmaps(target, GL_RGBA8, tex.getWidth(), tex.getHeight(), 
				  GL_RGBA, GL_UNSIGNED_BYTE, tex.getPixelsRGBA());
	}

	void Animation::LoadTexture(char * filename, GLuint texName) {
		LoadTexture(filename, texName, GL_TEXTURE_2D);
		GLuint target=GL_TEXTURE_2D;
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	void Animation::LoadTextureCube(char * filenamebase, GLuint texName) {
		char filename[strlen(filenamebase)+10];
		sprintf(filename, filenamebase, "XP");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		sprintf(filename, filenamebase, "XN");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		sprintf(filename, filenamebase, "YP");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		sprintf(filename, filenamebase, "YN");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		sprintf(filename, filenamebase, "ZP");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		sprintf(filename, filenamebase, "ZN");
		LoadTexture(filename, texName, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
		GLuint target=GL_TEXTURE_CUBE_MAP;
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}


	void Animation::setAspect(int width, int height) {
		Width=width;
		Height=height;
	}

	Animation::Animation(): worm(8,3, false), ball(30,30), hero(10,1000), board(8.0) {
		accum = 0;
		fixedCam.pos.set(0,40,0);
		fixedCam.vel.set(0,-1,0);
		fixedTarget.pos.set(0,0,0);
		fixedTarget.vel.set(0,-1,0);
		testAngle=0;
		camera = &fixedCam;
		lookAt = &fixedTarget;
		zoom = 1.0;
		cube.defaultVertexAttributes(5);
		cube.cacheTriangleAttributes();
		cube.flagVisibleEdges();
		worm.cacheTriangleAttributes();
		worm.flagVisibleEdges();
		ball.defaultVertexAttributes(3);
		ball.normalsForTrianglesFromVertices();
		ball.cacheTriangleAttributes(); 			
		ball.flagVisibleEdges();
		initSpline();
		wormCount = 5;
		wormScale = 0.2f;
		try {
			head.Load("models/obj_worm.ac", "Head", FFORMAT_AC3D);
			eyeL.Load("models/obj_worm.ac", "Sphere.003", FFORMAT_AC3D);
			eyeR.Load("models/obj_worm.ac", "Sphere.002", FFORMAT_AC3D);
			tail.Load("models/obj_worm.ac", "Cone", FFORMAT_AC3D);
			terrain.Load("models/terrain1.blend.ac","Grid", FFORMAT_AC3D);
			for (AttributeArray::iterator iterrain=terrain.Attributes.begin(); 
			     iterrain!=terrain.Attributes.end(); iterrain++) {
				iterrain->flags |= (ATTRIB_TEXCOORD1|ATTRIB_TEXCOORD2|ATTRIB_TEXCOORD3);
				iterrain->TexCoord[3]=
					iterrain->TexCoord[2]=
					iterrain->TexCoord[1]=iterrain->TexCoord[0];
				iterrain->TexCoord[3].multBy(10);
				iterrain->TexCoord[2].multBy(7);				
				iterrain->TexCoord[1].multBy(30);
			}
			tail.flagVisibleEdges();
			terrain.flagVisibleEdges();
		}
		catch (exception &ex) {
			cerr<<ex.what();
		}
//		head.debugPrint();
		head.flagVisibleEdges();
		board.Setup(&terrain);
	}

	void Animation::testRotation() {
		testAngle=testAngle+1.0;
	}

	void Animation::wormGrow() {
		wormCount++;
	}

	Animation::~Animation() {
		if (vertexProgram) free((void*)vertexProgram);
		if (vertexProgram) free((void*)fragmentProgram);
		glDeleteProgramsARB(4,hwPrograms);
		glDeleteTextures(10,texNames);
	}
}
