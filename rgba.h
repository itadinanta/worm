#ifndef __RGBA_H
#define __RGBA_H
namespace model {
	class RGB {
	public:
		unsigned char r,g,b;
		RGB() {}
		RGB(unsigned char R, unsigned char G, unsigned char B) {
			r=R;
			g=G;
			b=B;
		}
		inline void set(unsigned char R, unsigned char G, unsigned char B) {	      
			r=R;
			g=G;
			b=B;
		}
	};
	
	class RGBA: public RGB {
	public:
		unsigned char a;
		RGBA() {}
		RGBA(unsigned char R, unsigned char G, unsigned char B, unsigned char A): RGB(R,G,B),a(A) {		
		}
			inline void set(unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
				RGB::set(R,G,B);
				a=A;
			}
	};
};
#endif
