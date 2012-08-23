#ifndef _VECMATH_H
#define _VECMATH_H
#include <math.h>
#include <iostream>
#define FLOAT float

namespace spline {
	using namespace std;
	class V3 {
	public:
		static const V3 vZero, aX, aY, aZ;
	public:
		FLOAT x,y,z;
		inline V3(const V3 &src) {set(src);}
		inline V3& operator = (const V3& o) {set(o); return *this;}
		inline V3() {}
		inline V3(FLOAT _x, FLOAT _y, FLOAT _z) {set(_x, _y, _z);}
		inline void set(FLOAT _x, FLOAT _y, FLOAT _z) {
			x = _x;
			y = _y;
			z = _z;
		}
		inline void zero() {set(0,0,0);}
		inline void set(const V3& o) {set(o.x, o.y, o.z);}
		inline void setScalar(const V3& o, FLOAT timev) {set(o.x*timev, o.y*timev, o.z*timev);}
		inline void setDiff(const V3& o, const V3& r) {set(r.x - o.x, r.y - o.y, r.z - o.z);}
		inline void setSum(const V3& o, const V3& r) {set(r.x+o.x,r.y+o.y,r.z+o.z);}
		inline void setVec(const V3& a, const V3& s) {set(a.y*s.z-s.y*a.z, a.z*s.x-s.z*a.x, a.x*s.y-s.x*a.y);}
		inline void reflect(V3& d, const V3 &s) const {
			float coef = 2*(x*s.x+y*s.y+z*s.z);
			d.set(s.x-coef*x,s.y-coef*y,s.z-coef*z);
		}
		inline FLOAT norm() const {return x*x+y*y+z*z;}
		inline FLOAT len() const {return sqrtf(norm());}
		inline FLOAT distFrom(V3 other) const {other.orig(*this);return other.len();}
		void normalize() {
			FLOAT l=len();
			if (l) {
				l = (FLOAT)(1.0)/l;
				multBy(l);
			}
		}
		inline V3 operator-() {
			return V3(-x, -y ,-z);
		}
		inline void orig(const V3& o) {
			x-=o.x;
			y-=o.y;
			z-=o.z;
		}
		inline void moveBy(const V3& v, const V3& a, FLOAT timev) {
			float timev2 = 0.5*timev;
			x+=(v.x+a.x*timev2)*timev;
			y+=(v.y+a.y*timev2)*timev;
			z+=(v.z+a.z*timev2)*timev;
		}
		inline void moveBy(const V3& o, FLOAT timev) {
			x+=o.x*timev;
			y+=o.y*timev;
			z+=o.z*timev;
		}
		inline void moveBy(const V3& o) {
			x+=o.x;
			y+=o.y;
			z+=o.z;
		}
		inline V3& operator += (const V3& o) {
			moveBy(o);
			return *this;
		}
		inline V3& operator -= (const V3& o) {
			orig(o);
			return *this;
		}
		inline void multBy(FLOAT scale) {
			x*=scale;
			y*=scale;
			z*=scale;
		}
		inline V3& operator *= (FLOAT scale) {
			multBy(scale);
			return *this;
		}
		inline void divBy(FLOAT scale) {
			if (scale) {
				scale = (FLOAT)(1.0)/scale;
				multBy(scale);
			}
		}
		inline V3&operator /= (FLOAT scale) {
			divBy(scale);
			return *this;
		}
		inline FLOAT dot(const V3& s) const {
			return	x * s.x + 
				y * s.y + 
				z * s.z;
		}
		inline FLOAT operator * (const V3& s) const {
			return dot(s);
		}
		inline void vec(V3& d, const V3& s) const {
			d.x = y*s.z-s.y*z;
			d.y = z*s.x-s.z*x;
			d.z = x*s.y-s.x*y;	     
		}
		inline V3 operator ^ (const V3& s) const {
			V3 d;
			vec(d,s);
			return d;
		}
		const inline FLOAT& operator[](int idx) const {
			return *(((FLOAT*)this)+idx);
		}
		inline FLOAT& operator[](int idx) {
			return *(((FLOAT*)this)+idx);
		}
		V3& normal(V3 &n, const V3 & p1, const V3 &p2) const {
			V3 p1n = p1;
			V3 p2n = p2;
			p1n.orig(*this);
			p2n.orig(*this);
			p1n.vec(n,p2n);
			n.normalize();
			return n;
		}
	};

	class V4: public V3 {
	public:
		static const V4 iX;
		static const V4 iY;
		static const V4 iZ;
		static const V4 iW;
		static const V4 up;
	public:
		FLOAT w;
		V4() {}
		inline V4(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _w): V3(_x,_y,_z) {w = _w;}
			inline V4(FLOAT _x, FLOAT _y, FLOAT _z): V3(_x,_y,_z) {w = (FLOAT)1.0;}
		inline V4(const V4 &s) {set(s);}
		inline V4(const V3 &s) {set(s);}
		inline void set(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _w) {
			V3::set(_x,_y,_z);
			w = _w;
		}
		inline void zero() {set(0,0,0,0);}
		inline void set(const V3& s) {set(s.x, s.y, s.z, (FLOAT)1.0);}
		inline V4& operator = (const V3& s) {
			set(s);
			return *this;
		}
		inline V4& operator = (const V4& s) {
			set(s);
			return *this;
		}
		inline void set(const V4& s) {
			V3::set(s);
			w = s.w;
		}
		inline FLOAT dot(const V4& s) const {
			return  x * s.x + 
				y * s.y + 
				z * s.z + 
				w * s.w;
		}
		inline FLOAT operator * (const V4 &s) const {return dot(s);}
		inline FLOAT dot(const V3& s) const {
			return  x * s.x + 
				y * s.y + 
				z * s.z + 
				w;
		}
		inline FLOAT dot3(const V3& s) const {
			return  x * s.x + 
				y * s.y + 
				z * s.z;				
		}
		inline FLOAT operator * (const V3 &s) const {return dot(s);}
	};

	class M44 {
	public:
		V4 r1, r2, r3, r4;
		M44() {}
		inline M44(const V4& a, const V4 &b, const V4 &c, const V4 &d):
		r1(a), r2(b), r3(c), r4(d) {};
		inline M44(const V3& a, const V3 &b, const V3 &c, const V3 &d):
		r1(a), r2(b), r3(c), r4(d) {};
		inline M44(FLOAT a11, FLOAT a12, FLOAT a13, FLOAT a14,
			   FLOAT a21, FLOAT a22, FLOAT a23, FLOAT a24,
			   FLOAT a31, FLOAT a32, FLOAT a33, FLOAT a34,
			   FLOAT a41, FLOAT a42, FLOAT a43, FLOAT a44):
		r1(a11,a12,a13,a14),r2(a21,a22,a23,a24),r3(a31,a32,a33,a34),r4(a41,a42,a43,a44) {} 			
		inline void mult(V4& d, const V4& s) const {
			d.x=r1.dot(s);
			d.y=r2.dot(s);
			d.z=r3.dot(s);
			d.w=r4.dot(s);
		}
		inline void mult(V4& d, const V3& s) const {
			d.x=r1.dot(s);
			d.y=r2.dot(s);
			d.z=r3.dot(s);
			d.w=r4.dot(s);
		}
		inline void mult3(V3& d, const V3& s) const {
			d.x=r1.dot3(s);
			d.y=r2.dot3(s);
			d.z=r3.dot3(s);
		}
		inline void setR(int r, const V4& s) {
			(*this)[r].set(s);
 		}
		inline void setC(int col, const V3&s) {
			r1[col]=s.x;
			r2[col]=s.y;
			r3[col]=s.z;
		}
		inline void setC(int col, const V4&s) {
			r1[col]=s.x;
			r2[col]=s.y;
			r3[col]=s.z;
			r4[col]=s.w;
		}
		inline V4 getC(int col) const {
			V4 r;
			r.x=r1[col];
			r.y=r2[col];
			r.z=r3[col];
			r.w=r4[col];
			return r;
		}
		M44& mult(M44&dest, const M44& B) {
			mult(dest.r1, B.getC(0));
			mult(dest.r2, B.getC(1));
			mult(dest.r3, B.getC(2));
			mult(dest.r4, B.getC(3));
			dest.transpose();
			return dest;
		}
		const inline V4& operator[](int c) const {
			return *(((V4*)this)+c);
		}
		inline V4& operator[](int c) {
			return *(((V4*)this)+c);
		}
		inline M44 T() const {
			M44 t = *this;
			t.transpose();
			return t;
		}
		inline void transpose() {
			swap(r1.y,r2.x);
			swap(r1.z,r3.x);
			swap(r1.w,r4.x);
			swap(r2.z,r3.y);
			swap(r2.w,r4.y);
			swap(r3.w,r4.z);
		}
		inline void identity() {
			r1.set(V4::iX);
			r2.set(V4::iY);
			r3.set(V4::iZ);
			r4.set(V4::iW);
		}
		inline void rotY(FLOAT angle) {
			float sa = sin(angle);
			float ca = cos(angle);
			identity();
			r1.x=ca;
			r1.z=sa;
			r3.x=-sa;
			r3.z=ca;
		}
	private:
		float sw;
		inline void swap(FLOAT& a, FLOAT& b) {
			sw=a; a=b; b=sw;
		}
	};

	class ROT {
	public:
		V3 n;
		FLOAT disp;
		inline ROT(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _disp): n(_x,_y,_z) {
			disp = _disp;
		}
		inline void set(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _disp) {
			n.set(_x, _y, _z);
			disp = _disp;
		}
	};

	class QUAT {
	public:
		FLOAT s;
		V3 n;
	};

	class LOC {
	public:
		V4 pos;
		ROT dir;
		inline LOC(FLOAT _px, FLOAT _py, FLOAT _pz, FLOAT _dx, FLOAT _dy, FLOAT _dz, FLOAT _disp):
		pos(_px, _py, _pz), dir(_dx, _dy, _dz, _disp) {};
	};

	ostream& operator << (ostream& o, const V3 & v);
	ostream& operator << (ostream& o, const V4 & v);
	ostream& operator << (ostream& o, const M44 & m);	
}
#endif
