#include "vecmath.h"

namespace spline {	
	const V3 V3::vZero(0,0,0);
	const V3 V3::aX(1,0,0);
	const V3 V3::aY(0,1,0);
	const V3 V3::aZ(0,0,1);
	const V4 V4::iX(1,0,0,0);
	const V4 V4::iY(0,1,0,0);
	const V4 V4::iZ(0,0,1,0);
	const V4 V4::iW(0,0,0,1);
	const V4 V4::up(0,1,0,1);

	ostream& operator << (ostream& o, const V3 & v) {
		return o << v.x << '\t' << v.y << '\t'  << v.z << '\n';
	}

	ostream& operator << (ostream& o, const V4 & v) {
		return o << v.x << '\t' << v.y << '\t' << v.z << '\t' << v.w << '\n';
	}
	
	ostream& operator << (ostream& o, const M44 & m) {
		return o << m.r1 << m.r2 << m.r3 << m.r4 << '\n';
	}
}
