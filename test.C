#include <stdio.h>
#include "spline.h"
#include "linearmap.h"
#include "model.h"
#include "texture_storage.h"

using namespace std;
using namespace spline;
using namespace model;
using namespace texture;

void testLinearMap() {
	LinearMap<float, float> lmap;
	int nval=100;
	for (int i=0; i<=sqrtf(nval); i++) {
		lmap.add(i*i-4, i);
	}
	for (int i=0; i<=nval+1; i++) {
		float f = i;
		int j=lmap.firstNotLess(f);
		float fcmp=f;
		if (fcmp<0) fcmp=0;
		if (fcmp>(nval-1)*(nval-1)) fcmp=(nval-1)*(nval-1);
		int jj=j;
		if (jj!=j) {
			printf("Test failed!");
			printf("%f -> %f %f %d-%d against %d-%d %s\n",f,(float)lmap.eval(f,j), sqrtf(fcmp), j,j+1, jj,jj+1,(jj==j)?"pass!":"NOOOO!");
			exit(0);
		}
		else
			printf("%d, %f\n",j, (float)lmap.eval(f,j));
	}
}

void testKnotVector() {
	V3Vector v;
	v.push_back(V3(0,0,0));
	v.push_back(V3(1,0,0));
	v.push_back(V3(1,1,0));
	v.push_back(V3(0,1,0));
	KnotVector kv;

	kv.uniformAutoFill(1);
	kv.regenerate();
	kv.printDebug();
	cout << Cubic::MBBezier;
	int i;
	for (i=0; i<4; i++) cout << kv.N(i,0.5) << '\t' << kv.NB(i,0.5) <<'\n';

	kv.sequenceAutoFill(-3.0, 4.0, 1.0);
	kv.regenerate();
	kv.printDebug();
	cout << Cubic::MBSpline;
	for (i=0; i<4; i++) cout << kv.N(i,0.75) << '\t' << kv.NB(i,0.75) <<'\n';

	kv.chordLengthAutoFill(v);
	kv.regenerate();
	kv.printDebug();
	
}

static void testInterpolate() {
/*
	const int npts = 20;
	const int k = 4; // Order
	FLOAT knots[npts + 2*k - 2];
	V3 data[npts];
	V3 control[npts + 2];
	int i;
	FLOAT sumrand = 0;
	srand(getpid());
	for (i=0; i<npts; i++) {
		data[i].set(i, sin(i/10.0), cos(i/10.0));
		knots[i+k-1] = sumrand; // Uniform knot vector;
		sumrand += (rand()%1000)*0.001;
	}
	for (i=0; i<k-1; i++) {
		knots[i]=knots[k-1];
		knots[npts+k-1+i]=knots[npts+k-2];
	}
	interpolateSpline(npts, data, control, knots);
	int j;
	V3 evpt;
	for (i=0; i<npts+2*k-2; i++) {
		printf("%d\t%g\t",i,knots[i]);
		j=i-k+1;
			
		if (j<npts && j>=0)
			printf("%g\t%g\t%g\t",data[j].x, data[j].y, data[j].z);
		else
			printf("-\t-\t-\t");
		if (j<npts && j>=0) {
			evalSpline(knots[j+k-1], npts, &evpt, control, knots);
			printf("%g\t=>\t%g\t%g\t%g\t",knots[j+k-1], evpt.x, evpt.y, evpt.z);
		}
		else
			printf("-\t-\t-\t");
		j=i-k+2;
		if (j<=npts+1 && j>=0)
			printf("%g\t%g\t%g\t",control[j].x, control[j].y, control[j].z);
		else
			printf("-\t-\t-\t");
		printf("\n");
	}
	FLOAT step;
	for (step=1; step<=5; step+=0.2) {
		evalSpline(step, npts, &evpt, control, knots);
		printf("%g\t=>\t%g\t%g\t%g\n", step, evpt.x, evpt.y, evpt.z);
	}
*/
}

void testSpline() {
	V3Vector control;
	control.push_back(V3(0,0,0));
	control.push_back(V3(1,0,0));
	control.push_back(V3(1,1,0));
	control.push_back(V3(0,1,0));

	KnotVector kv;

	kv.uniformAutoFill(1);
	kv.regenerate();
	kv.printDebug();
	
	BSpline sp(kv, control);
	sp.regenerate();

	FLOAT step, t;
	V3 OR, OM;
	int nsteps=13;
	for (step=0; step<nsteps; step+=1) {
		t=1.0/(nsteps-1)*step;
		sp.eval(t, &OR);
		sp.eval(t,0,&OM);
		printf("%f\t=>\t%f\t%f\t%f\n", t, OR.x, OR.y, OR.z);
		printf("%f\t=>\t%f\t%f\t%f\n", t, OM.x, OM.y, OM.z);
	}
}

void testSplineStrip() {
	BSplineStrip sp;
	const int npts = 5;
	sp.add(V3(0,0,2),V3(0,0,1));
	sp.add(V3(1,1,3),V3(1,0,1));
	sp.add(V3(0,1,4),V3(1,1,1));
	sp.add(V3(0,0,5),V3(0,1,1));
	sp.regenerate();
	FLOAT step, t;
	V3 o,u,d;
	int nsteps=50;
	for (step=0; step<nsteps; step+=1) {
		t=1.0/(nsteps-1)*step;
		int iknot=sp.eval(t, &o, &u, &d); d.normalize();
		printf("%i\t%f\t=>\t%f\t%f\t%f ", iknot, t, o.x, o.y, o.z);
		printf(" - \t%f\t%f\t%f\n", d.x, d.y, d.z);

		o=sp.evalRecursively(t);
		printf("\t%g=>\t%g\t%g\t%g\n", t, o.x, o.y, o.z);

	}
}

int testEdges() {
	Cube cube;
	cube.defaultVertexAttributes(1);
	cube.cacheTriangleAttributes();
	cube.flagEdges(V3(0,0,0));
	cube.debugPrint();
}

int testPngTexture() { 
	PngLoader tex("test1.png");
	RGBA* pixels=tex.getPixelsRGBA();
	int i,j;
	for (i=0; i<tex.getHeight(); i++) {
		for (j=0; j<tex.getWidth(); j++) {
			RGBA* pixel=pixels+i*tex.getWidth()+j;
			printf("%02x%02x%02x%02x ",pixel->r,pixel->g,pixel->b,pixel->a);
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
//	testLinearMap();
//	testKnotVector();
//	testSpline();
//	testSplineStrip();
//	testEdges();
	testPngTexture();
	// splineRunTest();
	printf("Test passed!");
	return 0;
}
