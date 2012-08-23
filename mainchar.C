#include "mainchar.h"
namespace model {
	PositionChain::PositionChain(unsigned int MinLen, unsigned int MaxLen): 
		maxLen(MaxLen), minLen(MinLen) {
		PositionChainPos temp;
		temp.pos=V3::vZero;
		temp.vel=V3::vZero; 		
		length=0;
		for (int i=0; i<minLen; i++)
			push_front(temp);
	}

	void PositionChain::searchElements(FLOAT position, FLOAT &t, iterator &I1, iterator &I2) {
		if (empty()) {
			t=0;
			I1=I2=end();
		}
		int i=(int)position;
		t=position-floor(position);
		if (i<0) 
			i=0;
		else if (i>size()-1) 
			i=size()-1;
		I1=begin()+i;
		if (t<0.0001) {
			I2=I1;
			t=0.0f;
		} else {
			I2=I1+1;
		}
	}

	void PositionChain::eval(FLOAT position, V3 &evpt, V3 &uppt, V3 &dirpt) {
		iterator I1, I2;
		FLOAT t;
		searchElements(position, t, I1, I2);
		if (I1==end()) {
			evpt=V3::vZero;
			dirpt=V3::aX;
			uppt=V3::aY;
		} else {
			evpt=blendVectors(t, I1->pos, I2->pos);
			uppt=blendVectors(t, I1->up, I2->up);
			dirpt=blendVectors(t, I1->normal, I2->normal);
		}
	}

	void PositionChain::getMatrix(FLOAT position, M44 &M) {
		iterator I1, I2;
		FLOAT t;
		searchElements(position, t, I1, I2);
		blendMatrices(M, t, I1->matrix, I2->matrix);
	}
	

	void PositionChainPos::reassignRefMatrix() {
		buildRefMatrix(matrix, pos, up, normal);
	}

	
	void PositionChainPos::buildRefMatrix(M44 &Mdef, V3 evpt, V3 uppt, V3 dirpt) {
		V3 leftpt;
		uppt-=evpt;
		uppt.vec(leftpt, dirpt);
		dirpt.vec(uppt, leftpt);
		
		uppt.normalize();
		dirpt.normalize();
		leftpt.normalize();
		
		Mdef.identity();
		Mdef.setC(0,leftpt); // x to left (right?) TODO: check
		Mdef.setC(1,uppt);  // Y to up
		Mdef.setC(2,dirpt); // Z to derivative
		Mdef.setC(3,evpt);  // translate
	}

	void PositionChain::blendMatrices(M44 &M, FLOAT t, const M44 &M1, const M44 &M2) {
		if (!t)
			M=M1;
		else {
			FLOAT t1=1-t;
			for (register int i=0; i<16; i++) 
				((FLOAT*)(&M))[i]=t1 * ((FLOAT*)(&M1))[i]+t*((FLOAT*)(&M2))[i];
		}
	}

	V3 PositionChain::blendVectors(FLOAT t, const V3 &V1, const V3 &V2) {
		if (!t) 
			return V1;
		else {
			V3 V;
			FLOAT t1=1-t;		
			V.x=t1*V1.x+t*V2.x;
			V.y=t1*V1.y+t*V2.y;
			V.z=t1*V1.z+t*V2.z;
			return V;
		}
	}
				
	void PositionChain::Move(V3 pos, V3 vel, V3 up) {
		PositionChainPos newPos;	
		newPos.pos=pos;
		newPos.vel=vel;
		newPos.up=up;
		
		if (!size()) {
			newPos.dist=0;
			newPos.mileage=0.0f;
			newPos.invDist=0.0f;
			newPos.normal=vel;
			newPos.normal.normalize();
		} 
		else {
			PositionChainPos &oldPos1 = front();
			oldPos1.normal=pos;
			if (size()>1) {
				PositionChainPos &oldPos2 = *(begin()+1);
				oldPos1.normal-=oldPos2.pos;
				oldPos1.normal.normalize();
				newPos.normal=pos;
				newPos.normal-=oldPos1.pos;
				newPos.normal.normalize();				
			}
			else {
				oldPos1.normal-=oldPos1.pos;
				oldPos1.normal.normalize();			
				newPos.normal=oldPos1.normal;
			}
			oldPos1.reassignRefMatrix();
			newPos.dist=newPos.pos.distFrom(oldPos1.pos);
			newPos.invDist=(newPos.dist)?(1.0f/newPos.dist):0.0f;
			newPos.mileage=oldPos1.mileage+newPos.dist;
		}
		newPos.reassignRefMatrix();
		push_front(newPos);		
		while (size()>maxLen) pop_back();
		length=front().mileage-back().mileage;
	};
	
	void PositionChain::extend(int amount) {
		if (amount>0) 
			maxLen+=amount;
		else 
			while ((amount++) && size()<minLen) {
				pop_back();
			}
	}

	void PositionChainCursor::reset() {
		tChain=0;
		tPos=0;
	}

	FLOAT PositionChainCursor::seekDist(FLOAT newDist) {
		if (newDist>tChain) {
			FLOAT t, segment;
			FLOAT d1, d2;
			PositionChain::iterator I1, I2;
			FLOAT mileage=chain->getMileage();
			chain->searchElements(tPos, t, I1, I2);
			while (I2<chain->end()) {
				d1=mileage-I1->mileage;
				d2=mileage-I2->mileage;
//				cout << mileage << '\t' 
//				     << d1 << '\t'
//				     << d2 << '\n';
				if ((newDist>=d1) && ((newDist<d2) || (d1==d2))) {
					segment=(I1-chain->begin());
					tPos=segment+I1->invDist*(newDist-d1);
					tChain=newDist;
					return tPos;
				}
				++I1;
				++I2;
			}
			tChain=chain->getLength();
			tPos=chain->size();
		}
		return tPos;
	}

	FLOAT PositionChainCursor::seekPos(FLOAT newPos) {
		FLOAT t;
		PositionChain::iterator I1, I2;
		tPos=newPos;
		chain->searchElements(tPos, t, I1, I2);
		tChain=chain->getMileage()-chain->blend(t, I1->mileage, I2->mileage);
		return tChain;
	}
};
		
