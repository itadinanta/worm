#include <math.h>
#include <vector>
#include "vecmath.h"
#include "spline.h"
#include "linearmap.h"
#include <iostream>
namespace spline {
	using namespace std;

	static inline FLOAT divz(FLOAT a, FLOAT b) {
		return (b?(a/b):0.0);
	}
	/**
	 * Cubic
	 */
	
	void Cubic::pos(FLOAT t, V3 &d) const {
		register FLOAT t2=t*t;
		V4 T(t2*t, t2,t,1);
		d.x=T * cx;
		d.y=T * cy;
		d.z=T * cz;
//		printDebug();
	}
	void Cubic::vel(FLOAT t, V3 &d) const {
		register FLOAT _2t=2*t;
		register FLOAT _3t2=3*t*t;
		d.x=cx.x*_3t2+cx.y*_2t+cx.z;
		d.y=cy.x*_3t2+cy.y*_2t+cy.z;
		d.z=cz.x*_3t2+cz.y*_2t+cz.z;
//		printDebug();
	}
	void Cubic::printDebug() const {
		cout << "CUBIC:\n" << cx << cy << cz;	     
	}
	Cubic::Cubic(const Cubic& cb) {
		copyFrom(cb);
	}
	Cubic& Cubic::operator=(const Cubic& cb) {
		copyFrom(cb);
		return *this;
	}
	void Cubic::copyFrom(const Cubic& cb) {
		cx=cb.cx;
		cy=cb.cy;
		cz=cb.cz;
	}
	Cubic::Cubic() {cx.zero(); cy.zero(); cz.zero();}
	Cubic::Cubic(const M44& B,const  V3 &P0,const  V3 &P1,const   V3 &P2,const  V3 &P3) {
		setCoef(B,P0,P1,P2,P3);
	}
	void Cubic::setCoef(const M44& B,const  V3 &P0,const  V3 &P1,const  V3 &P2,const  V3 &P3) {
		V4 Px(P0.x, P1.x, P2.x, P3.x);
		V4 Py(P0.y, P1.y, P2.y, P3.y);
		V4 Pz(P0.z, P1.z, P2.z, P3.z);

		B.mult(cx,Px);
		B.mult(cy,Py);
		B.mult(cz,Pz);
	}

	/**
	 * KnotVector
	 */

	KnotVector::KnotVector() {
		lastKnot = 0;
		firstKnot = 0;
		knotScale =1;
	}

	const M44& KnotVector::blendingFunction(int i) const {
		if (i>=0 && i<blendingFunctions.size())
			return blendingFunctions[i];
		else
			throw exception();
	}
	
	const FLOAT KnotVector::operator[](int i) const {
		return t[i];
	}

	void KnotVector::clear() {
		t.clear();
		blendingFunctions.clear();
	}

	void KnotVector::regenerate() {
		evalBlendingFunctionCoefficients();
		createIndex();
	}
	void KnotVector::evalBlendingFunctionCoefficients() {
		blendingFunctions.clear();
		M44 B;
		FLOAT t1,t2,t3,t4,t5,t6;
		FLOAT t6_5,t6_4,t6_3,t6_2,t6_1;
		FLOAT t5_4,t5_3,t5_2,t5_1;
		FLOAT t4_3,t4_2,t4_1;
		FLOAT t3_2,t3_1;
		FLOAT t2_1;
		FLOAT D1,D2,D3;
		int i;
		int nfunctions = t.size()-2*k+1;
		for (i=0; i<nfunctions; i++) {
			t1=t[i+1]; t2=t[i+2]; t3=t[i+3]; t4=t[i+4]; t5=t[i+5]; t6=t[i+6];
			t6_5=t6-t5; t6_4=t6-t4; t6_3=t6-t3; t6_2=t6-t2; t6_1=t6-t1;
			t5_4=t5-t4; t5_3=t5-t3; t5_2=t5-t2; t5_1=t5-t1;
			t4_3=t4-t3; t4_2=t4-t2; t4_1=t4-t1;
			t3_2=t3-t2; t3_1=t3-t1;
			t2_1=t2-t1;

			// C0
			D1 = t4_1*t4_2*t4_3;

			B[0][0]=divz(-1,D1);
			B[1][0]=divz( 3*t4,D1);
			B[2][0]=divz(-3*t4*t4,D1);
			B[3][0]=divz(   t4*t4*t4,D1);

			
			// C1
			D1 = t5_2*t4_3*t4_2;
			D2 = t5_2*t4_3*t5_3;
			D3 = t4_1*t4_2*t4_3;

			B[0][1]=divz(1,D1)+divz(1,D2)+divz(1,D3);
			B[1][1]=divz(-t4-t5-t2,D1)+divz(-t5-t5-t3,D2)+divz(-t4-t4-t1,D3);
			B[2][1]=divz(t4*t5+t2*t4+t2*t5,D1)+divz(t5*t5+2*t3*t5,D2)+divz(t4*t4+2*t1*t4,D3);
			B[3][1]=divz(-t2*t5*t4,D1)-divz(t3*t5*t5,D2)-divz(t1*t4*t4,D3);			

			// C2
			D1=t4_2*t5_2*t4_3;
			D2=t5_2*t4_3*t5_3;
			D3=t4_3*t5_3*t6_3;

			B[0][2]=divz(-1,D1)+divz(-1,D2)+divz(-1,D3);
			B[1][2]=divz(t4+t2+t2,D1)+divz(t5+t3+t2,D2)+divz(t6+t3+t3,D3);
			B[2][2]=divz(-2*t4*t2-t2*t2,D1)+divz(-t3*t5-t2*t5-t3*t2,D2)+divz(-2*t3*t6-t3*t3,D3);
			B[3][2]=divz(t4*t2*t2,D1)+divz(t2*t3*t5,D2)+divz(t6*t3*t3,D3);

			// C3
			D1 = t6_3*t5_3*t4_3;

			B[0][3]=divz( 1,D1);
			B[1][3]=divz(-3*t3,D1);
			B[2][3]=divz( 3*t3*t3,D1);
			B[3][3]=divz(-1*t3*t3*t3,D1);
			
			blendingFunctions.push_back(B);
		}
	}

	void KnotVector::createIndex() {
		knotIdx.clear();
		for (FLOATVector::iterator i=t.begin(); i<t.end(); i++)
			knotIdx.add(*i);			
	}

	void KnotVector::chordLengthAutoFill(const V3Vector& data) {
		int npts = data.size();
		if (npts < k-2) return;
		clear();
//		const int k = 4; // Order
		for (int i=0; i<k; i++) t.push_back(0.0);

		int i;
		firstKnot = 0;
		FLOAT sumknot = firstKnot;
		for (i=1; i<npts; i++) {
			sumknot += data[i].distFrom(data[i-1]);
			t.push_back(sumknot);
		}
		lastKnot = sumknot;
		knotScale =1.0/(lastKnot - firstKnot);

		for (i=0; i<k-1; i++)
			t.push_back(lastKnot);
		lastKnot = 1.0;

		for (FLOATVector::iterator i=t.begin(); i<t.end(); i++)
			*i*=knotScale;
	}

	void KnotVector::uniformAutoFill(int nsegments) {
		if (nsegments<1) return;
		clear();
		firstKnot=0;
		lastKnot=nsegments;
		knotScale=1.0/(nsegments);
		int i;
		for (i=0; i<k; i++) t.push_back(0.0);
		for (i=1; i<nsegments; i++) t.push_back((FLOAT)i);
		for (i=0; i<k; i++) t.push_back((FLOAT)nsegments);
	}

	void KnotVector::sequenceAutoFill(FLOAT t0, FLOAT tn, FLOAT step) {
		if (tn-t0 < step) return;
		clear();
		firstKnot=t0;
		lastKnot=tn;
		knotScale=step/(tn-t0);
		for (FLOAT i=t0; i<=tn; i+=step) t.push_back(i);
	}

	void KnotVector::printDebug() {
		for (FLOATVector::iterator i=t.begin(); i<t.end(); i++) {
			printf("knot %i = %f\n", i-t.begin(), *i);
		}
		for (M44Vector::iterator B=blendingFunctions.begin(); B<blendingFunctions.end(); B++) {
			cout << *B;
		}
	}

	FLOAT KnotVector::N(int i, FLOAT u) const {
		return N(i,k,u);
	}

	FLOAT KnotVector::N(int i, int k, FLOAT u) const {
		FLOAT n;
//		printf("%s u=%g k=%d t=(%g, %g)\n","\t\t\t\t\t"+k,u,k,t[0],t[k]);
		if (u>=t[i] && u<t[i+k]) {
			if (k == 1) 
				n=1.0;
			else
				n=divz((u-t[i])*N(i,k-1,u),(t[i+k-1]-t[i]))+
					divz((t[i+k]-u)*N(i+1,k-1,u),(t[i+k]-t[i+1]));
		}
		else
			n = 0.0;
//		printf("%s n=%g\n","\t\t\t\t\t"+k, n);
		return n;		
	}

	FLOAT KnotVector::NB(int i, float u) const {
		FLOAT u2=u*u;
		V4 uPow(u*u2, u2, u, 1);
		int j=getKnotIdx(u);
		int c=i-j;
		printf("B=%i, i=%i, c=%i\n",j,i,c);
		if (c<0 || c>k-1) return 0;		
		const M44 &B=blendingFunctions[j];
		return uPow * B.getC(c);
	}

	size_t KnotVector::getKnotIdx(FLOAT u) const {
		return knotIdx.firstNotLess(u,k-1,t.size()-k)-k+1;
	}

	/**
	 * BSpline
	 */
	
#define MBD(i) ((FLOAT)i/6.0)
	
	const M44 Cubic::MBSpline(MBD(-1), MBD( 3), MBD(-3), MBD( 1),
				  MBD( 3), MBD(-6), MBD( 3), 0,
				  MBD(-3), 0,       MBD( 3), 0,
				  MBD( 1), MBD( 4), MBD( 1), 0);
	
	const M44 Cubic::MBHermite( 2,-2, 1, 1,
				   -3, 3,-2,-1,
				    0, 0, 1, 0,
				    1, 0, 0, 0);

	const M44 Cubic::MBBezier(-1, 3,-3, 1,
				   3,-6, 3, 0,
				  -3, 3, 0, 0,
				   1, 0, 0, 0);

	V3 BSpline::evalSplineMatrix(FLOAT u) const {
		int i=knots.getKnotIdx(u);
		return evalSplineMatrix(u, i);
	}
	
	V3 BSpline::evalSplineMatrix(FLOAT u, int i) const {
		if (i<0)
			return control[0];
		else if (i<segments.size()) {
			const Cubic &b=segments[i];
			V3 result;
			b.pos(u,result);
			return result;
		}
		else return control[control.size()-1];
	}

	V3 BSpline::evalSplineDerivativeMatrix(FLOAT u, int knotIndex) const {
//		if (!u) 
		if (knotIndex < segments.size()) 
			u+=1e-6; // Discontinuity in u=0, shift by eps
		else {
			knotIndex--;
			u-=1e-4; // Discontinuity in u=1
//			cout << "Knot: "<<knotIndex<<" u="<<u<<'\n';
		}
		const Cubic &b=segments[knotIndex];
		V3 result;
		b.vel(u,result);
		return result;
	}

	V3 BSpline::evalSplineRecursively(FLOAT u) const {
		int i,npts=control.size();
		static const int k=4;
		V3 S(0.0,0.0,0.0),K;
		if (u>=knots.LastKnot())
			S=control[npts-1]; // Estremo destro
		else
			for (i = 0; i < npts; i++) {
				K = control[i];
				K *= knots.N(i, k, u);
				S += K;
			}
		return S;
	}
	const V3& BSpline::getControlPoint(int i) {
		return control[i];
	}
	BSpline::BSpline(const KnotVector &Knots):
		knots(Knots) {}

	BSpline::BSpline(const KnotVector &Knots, const V3Vector &controlPoints):
		knots(Knots), control(controlPoints.begin(), controlPoints.end()) {}

	void BSpline::regenerate() {
		evalBlendingMatrices();
	}
	void BSpline::evalBlendingMatrices() {
		segments.clear();
		M44 b;
		int i;
		for (i=0; i<control.size()-(k-1); i++) {
			Cubic C(knots.blendingFunction(i),
				control[i],
				control[i+1],
				control[i+2],
				control[i+3]);
			segments.push_back(C);		       
			const Cubic &b=segments[segments.size()-1];
		}
	}
	size_t BSpline::size() {
		return knots.size()-2*k-2;
	}
	const V3& BSpline::eval(FLOAT u, V3* result) const {
		return *result=evalSplineRecursively(u);
	}
	const V3& BSpline::eval(FLOAT u, int i, V3* result) const {
		return *result=evalSplineMatrix(u, i);
	}
	const V3& BSpline::evalDerivative(FLOAT u, int i, V3* result) const {
		return *result=evalSplineDerivativeMatrix(u, i);
	}
	FLOAT BSpline::blend(FLOAT u, int segment) {
		
	}
	void BSpline::clear() {
	}
	BSpline::~BSpline() {
	}

	/**
	 * InterpolatedBSpline
	 */

	InterpolatedBSpline::InterpolatedBSpline(const KnotVector &Knots, const V3Vector &dataPoints):
		BSpline(Knots), data(dataPoints)  {
		}
	void InterpolatedBSpline::regenerate() {
		interpolate();
		BSpline::regenerate();
	}
	void InterpolatedBSpline::interpolate() {
		int i,npts=data.size();
 		FLOAT a, b, c, A, B, C, gamma_1, beta;
		FLOAT t4_2,t4_3,t4_1,t3_2,t3_1,t2_1,t5_4,t5_3,t5_2,t6_5,t6_4,t6_3;
		V3 delta_1, tmp;
		FLOATVector gamma(npts);
		V3Vector delta(npts);
		const KnotVector &t = knots;
		t5_4=t[5]-t[4]; t5_3=t[5]-t[3]; t5_2=t[5]-t[2];
		t4_3=t[4]-t[3];	t4_2=t[4]-t[2];	t4_1=t[4]-t[1];
		t3_2=t[3]-t[2];	t3_1=t[3]-t[1];

		gamma_1 = 0;
		delta_1 = V4::vZero;
		for (i = 0; i < npts; i++) {
			if (i == 0 || i==npts-1) {
				a=c=0;
				b=1.0;
				gamma[i] = 0.0;
				delta[i] = data[i];
				beta = 1;
			}
			else {			
     				t5_4=t[i+5]-t[i+4];
				t5_3=t[i+5]-t[i+3];
				t5_2=t[i+5]-t[i+2];

				a = (t4_3*t4_3 / t4_1 ) / t4_2;
				b = (t3_1*t4_3 / t4_1 + t5_3*t3_2/t5_2) / t4_2;
				c = (t3_2*t3_2 / t5_2) / t4_2;
#ifdef TEST
				A = t.NB(i,t[i+3]);
				B = t.NB(i+1,t[i+3]);
				C = t.NB(i+2,t[i+3]);
#define DELTA(x,y) (fabs((x)-(y))>1e-6)
				if (DELTA(a,A)) {
					printf("Errore su a[%d], %f, %f\n",i,a,A);
					printf("%f %f %f %f %f %f\n", t[i+0],t[i+1],t[i+2],t[i+3],t[i+4],t[i+5]);
				}
				if (DELTA(b,B)) printf("Errore su b[%d], %f, %f\n",i,b,B);
				if (DELTA(c,C)) {
					printf("Errore su c[%d] %f, %f, %f\n",i,t[i+3],c,C);
					printf("%f\t%f\t%f\t%f\t%f\t%f\n", t[i+0],t[i+1],t[i+2],t[i+3],t[i+4],t[i+5]);
				}
#endif			
				beta = b - a * gamma_1;
				gamma[i] = c / beta;
				delta[i] = data[i];
				tmp = delta_1;
				tmp *= -a;
				delta[i] += tmp;
				delta[i] /= beta;

			}
			
//			printf("%i)\ta=%f\tb=%f\tc=%f\td=%f\tbeta=%f\tgamma_1=%f\tgamma=%f\tdelta=%f\n",i,a,b,c,data[i].x,beta,gamma_1,gamma[i],delta[i].x);
			delta_1 = delta[i];
			gamma_1 = gamma[i];
			
			t3_2=t4_3;
			t3_1=t4_2;		  
			t4_3=t5_4;
			t4_2=t5_3;
			t4_1=t5_2;
		}

		// Final sweep
		// control++;
		control.resize(npts+2);
		control[npts] = delta_1;
		V3 control_1 = control[npts];
		for (i=npts-2; i>=0; i--) {
			// control[i] = delta[i] - gamma[i] * control[i+1]
			control[i+1] = delta[i];
			tmp = control_1;
			tmp *= -gamma[i];
			control[i+1] += tmp;
			control_1 = control[i+1];
		}

		control[0]=control[1];
		control[npts+1]=control[npts];
	}

					


	/**
	 * BSplineStrip
	 */

	BSplineStrip::BSplineStrip(): 
		offset(0), 
		originSpline(knots, origin),
		upSpline(knots, up),
		isValid(false) {}
	void BSplineStrip::regenerate() {
		if (origin.size()>2 && !isValid) {
			knots.chordLengthAutoFill(origin);
			knots.regenerate();
			originSpline.regenerate();
			upSpline.regenerate();
			isValid = true;
		}
	}
	void BSplineStrip::add(const V3 &sampleOrigin, const V3 &sampleUp) {
		origin.push_back(sampleOrigin);
		up.push_back(sampleUp);
		isValid=false;
	}
	V3 BSplineStrip::evalRecursively(float u) const {
		return originSpline.evalSplineRecursively(u);
	}
	int BSplineStrip::eval(FLOAT u, V3 *uOrigin, V3 *uUp, V3 *uDirection) const {
		int i=knots.getKnotIdx(u);
		return (eval(u,i,uOrigin,uUp,uDirection))
			?i
			:-1;
	}
	bool BSplineStrip::eval(FLOAT u, int i, V3 *uOrigin, V3 *uUp, V3 *uDirection) const {
		if (!isValid) return false;
		originSpline.eval(u, i, uOrigin);
		originSpline.evalDerivative(u, i, uDirection);
		upSpline.eval(u, i, uUp);
		return true;
	}
	void BSplineStrip::clear() {
		knots.clear();
		origin.clear();
		up.clear();
		lenIdx.clear();
		isValid=false;
	}
	void BSplineStrip::shift(const V3 &sampleOrigin, const V3 &sampleUp) {
		origin.erase(origin.begin());
		up.erase(up.begin());
		add(sampleOrigin, sampleUp);
		regenerate();
	}
	/*
	void BSplineStrip::evalByDist(FLOAT l, V3& result) {
		eval(uByDist(l),result);
	}
	*/
}
