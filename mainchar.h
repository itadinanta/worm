#ifndef __MAINCHAR_H
#define __MAINCHAR_H

#include <deque>
#include "vecmath.h"

namespace model {
	using namespace spline;

	class PositionChainPos {
	public:
		V3 pos;
		V3 vel;
		V3 up;
		V3 normal;
		FLOAT dist;
		FLOAT mileage;
		FLOAT invDist;
		M44 matrix;
		static void buildRefMatrix(M44 &Mdef, V3 evpt, V3 uppt, V3 dirpt);
		void reassignRefMatrix();
	};

	class PositionChain: public deque<PositionChainPos> {
	private:
		unsigned int maxLen, minLen;
		FLOAT length;
	public:
		static void blendMatrices(M44 &M, FLOAT t, const M44 &M1, const M44 &M2);
		static V3 blendVectors(FLOAT t, const V3 &V1, const V3 &V2);
		static FLOAT blend(FLOAT t, FLOAT a, FLOAT b) {return a+t*(b-a);}
		PositionChain(unsigned int MinLen, unsigned int MaxLen);
		void Move(V3 newPos, V3 newVel, V3 newUp);
		inline void Move(V3 newPos, V3 newVel) {
			V3 newUp=newPos; 
			newUp.y+=1.0f;
			Move(newPos,newVel,newUp);
		};
		inline FLOAT getLength() {return length;}
		inline FLOAT getMileage() {return size()?front().mileage:0;}
		void extend(int amount);
		void eval(FLOAT position, V3 &evpt, V3 &uppt, V3 &dirpt);
		void getMatrix(FLOAT position, M44 &mOut);
		void searchElements(FLOAT position, FLOAT &t, iterator &I1, iterator &I2);
	};

	class PositionChainCursor {
	private:
		PositionChain* chain;
		FLOAT tChain;
		FLOAT tPos;
	public:
		inline FLOAT getPos() {return tPos;}
		FLOAT seekDist(FLOAT newDist);
		FLOAT seekPos(FLOAT newPos);
		void reset();
		PositionChainCursor(PositionChain *aChain): 
			chain(aChain) {
			tChain=0;
			tPos=0;
		}
	};
}

#endif
