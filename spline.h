#ifndef _SPLINE_H
#define _SPLINE_H

#include "vecmath.h"
#include "linearmap.h"
#include <vector>

namespace spline {
	using namespace std;

	static const int k=4;

	class Cubic {
	private:
		void copyFrom(const Cubic & cp);
	protected:
		V4 cx,cy,cz;
		void setCoef(const M44& B,const  V3 &P0,const  V3 &P1,const  V3 &P2,const  V3 &P3);
	public:
		static const M44 MBBezier;
		static const M44 MBSpline;
		static const M44 MBHermite;
		void printDebug() const;
		inline V3 operator() (FLOAT t) const {
			V3 r; 
			pos(t, r);
			return r;
		}
		Cubic();
		Cubic(const Cubic &);
		Cubic& operator=(const Cubic &);
		Cubic(const M44& B,const  V3 &P0,const  V3 &P1,const   V3 &P2,const  V3 &P3);
		void pos(FLOAT t, V3 &d) const;
		void vel(FLOAT t, V3 &d) const;
	};

	typedef vector<V3> V3Vector;
	typedef vector<FLOAT> FLOATVector;
	typedef vector<M44> M44Vector;
	
	class KnotVector {
	private:
		FLOAT lastKnot;
		FLOAT firstKnot;
		FLOAT knotScale;
		M44Vector blendingFunctions;
		FLOATVector t;
		LinearMap<FLOAT,FLOAT> knotIdx;
		void evalBlendingFunctionCoefficients();
		void createIndex();
	public:
		static const int k=4;
		KnotVector();
		void add(FLOAT u);
		void clear();
		const FLOAT operator[](int i) const;
		FLOAT operator()(FLOAT u) const;
		FLOAT N(int i, int k, FLOAT u) const;
		FLOAT NB(int i, float u) const;
		FLOAT N(int i, float u) const;
		inline size_t size() const {return t.size();}
		void regenerate();
		size_t getKnotIdx(FLOAT u) const;
		void chordLengthAutoFill(const V3Vector& data);
		void uniformAutoFill(int nsegments);
		void sequenceAutoFill(FLOAT t0, FLOAT tn, FLOAT step);
		const M44& blendingFunction(int i) const;
		inline FLOAT LastKnot() const {return lastKnot;}
		inline FLOAT FirstKnot() const {return firstKnot;}
		void printDebug();
	};

	class BSpline {
	protected:
		const KnotVector &knots;
		V3Vector control;
		vector<Cubic> segments;
		void evalBlendingMatrices();
		FLOAT blend(FLOAT u, int segment);
		V3 evalSplineMatrix(FLOAT u) const;
		V3 evalSplineMatrix(FLOAT u, int knotIndex) const;
		V3 evalSplineDerivativeMatrix(FLOAT u, int knotIndex) const;
	public:
		static const int k=KnotVector::k;
		BSpline(const KnotVector &Knots);
		BSpline(const KnotVector &Knots, const V3Vector &controlPoints);
		virtual ~BSpline();
		size_t size();
		const V3& eval(FLOAT u, V3* result) const;
		const V3& eval(FLOAT u, int i, V3* result) const;
		const V3& evalDerivative(FLOAT u, int i, V3* result) const;
		inline V3 operator() (FLOAT u) {V3 point; return eval(u, &point);};
		virtual void regenerate();
		void clear();
		const V3& getControlPoint(int i);
		V3 evalSplineRecursively(FLOAT u) const;
	};

	class InterpolatedBSpline: public BSpline {
	protected:
		void interpolate();
	private:
		const V3Vector& data;
	public:
		virtual void regenerate();		
		InterpolatedBSpline(const KnotVector &Knots, const V3Vector &dataPoints);
		virtual ~InterpolatedBSpline() {};		
	};

	class BSplineStrip {
	private:
		KnotVector knots;
		V3Vector origin, up;
		InterpolatedBSpline originSpline, upSpline;
		LinearMap<FLOAT, FLOAT> lenIdx;
		
		bool isValid;
		FLOAT offset;
	public:
		BSplineStrip();
		void regenerate();
		void add(const V3 &Origin, const V3 &Up);
		int eval(FLOAT u, V3 *uOrigin, V3 *uUp, V3 *uDirection) const;
		bool eval(FLOAT u, int i, V3 *uOrigin, V3 *uUp, V3 *uDirection) const;
		void clear();
		void shift(const V3 &Origin, const V3 &up);
		V3 evalRecursively(float u) const;
	};
};

#endif
