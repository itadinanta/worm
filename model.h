#ifndef __MODEL_H
#define __MODEL_H

#include <vector>
#include <map>
#include <bitset>
#include "vecmath.h"
#include "rgba.h"

namespace model {
	using namespace std;
	using namespace spline;

	typedef V3 Vertex;
	typedef V3 Normal;
	typedef int Index;
	typedef unsigned int Flags;


	enum {ATTRIB_COLOR=1, 
	      ATTRIB_SPECULAR=2, 
	      ATTRIB_TEXCOORD0=4, 
	      ATTRIB_TEXCOORD1=8,
	      ATTRIB_TEXCOORD2=16,
	      ATTRIB_TEXCOORD3=32};

	enum {FFORMAT_OFF, FFORMAT_AC3D};

	class Attributes {
	public:
		RGBA Color;
		RGBA Specular;
		V3 TexCoord[4];
		unsigned char flags;
	};
     
	class PointArray: public vector<V3> {
	public:
		PointArray() {};
	};

	class AttributeArray: public vector<Attributes> {
	public:
		AttributeArray() {};
	};

	class IndexArray: public vector<Index> {
	public:
		IndexArray() {};
	};
	
	template <Index Count>
	class IndexContainer {
 	public:
		Index Vertex[Count];
		Index Normal[Count];
		Index Attributes[Count];
		Flags f;
		inline Index count() {return Count;}
		IndexContainer() {}
	};
	
	
	class Segment: public IndexContainer<2> {};
	class Triangle: public IndexContainer<3> {
	public:
		Index TriangleNormal;
		Index TriangleAttributes;
		inline void setup(Index i1, Index i2, Index i3, 
				  Index n1, Index n2, Index n3, 
				  Index a1, Index a2, Index a3, 
				  Index nT, Index aT) {
			Vertex[0]=i1;
			Vertex[1]=i2;
			Vertex[2]=i3;
			Normal[0]=n1;
			Normal[1]=n2;
			Normal[3]=n3;
			Attributes[0]=a1;
			Attributes[1]=a2;
			Attributes[2]=a3;
			TriangleNormal=nT;
			TriangleAttributes=aT;
		};		
	};

	typedef enum  {
		EDGE_LEFT_BIT = 0,
		EDGE_RIGHT_BIT = 1,
		EDGE_LEFT =       0x00000001,
		EDGE_RIGHT =      0x00000002,
		EDGE_SILHOUETTE = 0x00000004,
		EDGE_MATERIAL =   0x00000008,
		EDGE_MODEL =      0x00000080,
		EDGE_CULLED =     0x00000100
	} EdgeFlags;

	class TriangleGeometry {
	public:
		Vertex V[3];
		bool BariCoordsXZ(FLOAT &u, FLOAT &v, const Vertex& Point, FLOAT *y=0) {
			Vertex UU,VV,PP;
			UU.set(V[1].x-V[0].x, 0, V[1].z-V[0].z);
			VV.set(V[2].x-V[0].x, 0, V[2].z-V[0].z);
			PP.set(Point.x-V[0].x, 0, Point.z-V[0].z);

			FLOAT PxUz, PzUx, VxUz, VzUx;
			FLOAT PxVz, PzVx, D;

			PxUz = PP.x*UU.z; PzUx = PP.z*UU.x;
			PxVz = PP.x*VV.z; PzVx = PP.z*VV.x;
			VxUz = VV.x*UU.z; VzUx = VV.z*UU.x;
			D=(VxUz-VzUx);
			if (!D) return false;
			D=1.0f/D;
			u=(PzVx-PxVz)*D;
			v=(PxUz-PzUx)*D;
			if (y) *y=u*V[1].y+v*V[2].y+(1-u-v)*V[0].y;
			return (u>=0) && (v>=0) && (u+v<=1);
		};
		void normal(Normal&N) {
			Vertex B,C;
			B=V[1]; B-=V[0];
			C=V[2]; C-=V[0];
			N.setVec(C,B);
		};
		Normal getNormal() {
			Normal N;
			normal(N);
			return N;
		};
	};

	class Edge: public IndexContainer<2> {
	public:
		unsigned int flags;
		Index Triangles[2];
		Index CompVertex[2];	
		Edge(Index i1, Index i2) {
			Vertex[0]=i1;
			Vertex[1]=i2;
			Attributes[0]=Attributes[1]=
				Normal[0]=Normal[1]=				
				Triangles[0]=Triangles[1]=
				CompVertex[0]=CompVertex[1]=-1;
			flags=0;
		}
		inline bool direction(int i1, int i2) {
			return !(Vertex[0]==i1 && Vertex[1]==i2); // returns zero if edge equals (i1,i2)
		}
		void setTriangle(Index i1, Index i2, Index t, Index i3) {
			Index side = (i1==Vertex[0])?EDGE_RIGHT_BIT:EDGE_LEFT_BIT;
			if (flags & (1<<side)) {
				cerr << "\tDuplicate edge for triangle " << Attributes[side] << ": \t" 
				     << i1 << ", " << i2 << "\n";
				side = 1-side;
			}
			flags|=(1<<side);
			Triangles[side]=t;
			CompVertex[side]=i3;
		}
		void setAttributes(Index n1, Index n2, Index a1, Index a2) {
			Normal[0]=n1;
			Normal[1]=n2;
			Attributes[0]=a1;
			Attributes[1]=a2;
		}
		inline bool isVisible() {
			// Marks always visible edges
			return (flags & (EDGE_SILHOUETTE|EDGE_MATERIAL|EDGE_MODEL));
		}
		inline bool isCulled() {
			// Mark never visible edges
			return flags & (EDGE_CULLED);
		}
	};

	class EdgeId {
		Index _i1, _i2;
	public:
		inline EdgeId(Index _I1, Index _I2) {
			if (_I1<=_I2) {
				_i1=_I1;
				_i2=_I2;
			} else {
				_i1=_I2;
				_i2=_I1;
			}
		}
		inline EdgeId(const EdgeId &id) {
			_i1=id._i1;
			_i2=id._i2;
		}
		inline EdgeId& operator=(const EdgeId &id) {
			_i1=id._i1;
			_i2=id._i2;
			return *this;
		}
		inline bool operator==(const EdgeId &id) const {
			return _i1 == id._i1 && _i2 == id._i2;
		}
		inline bool operator<(const EdgeId &id) const {
			return (_i1<id._i1) || (_i1==id._i1 && _i2<id._i2); // too long.
		}
		inline Index i1() {return _i1;}
		inline Index i2() {return _i2;}
	};

	typedef enum {PRIM_POLY, PRIM_STRIP, PRIM_FAN} PrimType;
	const Index COUNT_VERTICES=-1;
	const Index COUNT_TRIANGLES=-2;
	class Primitive {	
		Index baseIndex;
		PrimType primType;
		Index *vertices;
		Index *vertexAttributes;
		Index *vertexNormals;
		Index *triangleNormals;
		Index *triangleAttributes;
		Index defaultNormal;
		Index defaultAttribute;
		Index vertexCount;
		void Fill(Index *v, Index value, Index count);
		void Fill(Index *v, Index * values, Index count);
		void Fill(Index *v, Index * values, Index defaultValue, Index count);
	public:
		Primitive(int baseIndex, PrimType primType, int vertexCount);
		~Primitive();
		void getTriangle(Index i, Triangle *T);
		void getTriangleRelative(Index tIndex, Triangle *T);
		Index *allocIndices(Index count);
		void freeIndices(Index * buf);
		PrimType getPrimType() {return primType;}
		Index translateCount(Index count);
		void setVertexList(Index * bufVertices);
		void setNormalList(Index * bufVertexNormals, Index* bufTriangleNormals=0, Index primNormal=-1);
		void setAttributesList(Index * bufVertexAttributes, Index* bufTriangleAttributes = 0, Index primAttributes=-1);
		void getVertexList(Index * bufVertices);
		void getNormalList(Index * bufVertexNormals, Index * bufTriangleNormals=0);
		void getAttributesList(Index *bufVertexAttributes, Index * bufTriangleAttributes = 0);
		inline void setDefaultAttribute(Index v) {defaultAttribute = v;};
		inline Index getDefaultAttribute() {return defaultAttribute;}
		inline void setDefaultNormal(Index v) {defaultNormal = v;};
		inline Index getDefaultNormal() {return defaultNormal;}
		Index getVertexCount();
		Index getTriangleCount();
		Index getTriangleFirst();
		Index getTriangleLast();
	};

	class PrimitiveArray: public vector<Primitive*> {
	public:
		~PrimitiveArray();
	};
	class EdgeArray: public vector<Edge> {
	};
	
	typedef map<EdgeId, Index> EdgeMap;

	class Mesh {
	private:
		EdgeMap revEdgeMap;
	protected:
		Primitive* CreatePrimitive(PrimType primType, Index VertexCount);
		void addEdge(int i, int j, int t, int k);
	public:
		Mesh();
		PointArray Vertices;
		PointArray Normals;
		AttributeArray Attributes;
		PrimitiveArray Primitives;
		EdgeArray Edges;
		IndexArray TriangleIndex;
		void createEmptyNormals();
		void normalsForTrianglesFromVertices();
		void normalsForVerticesFromTriangles();
		void normalsForVerticesFromVertices();
		static void approxNormalsXYZ(int count, V3* points, V3* normals, V3 up);
		Index getTriangleCount() const;
		Index addVertex(const Vertex& V);
		Index addNormal(const Normal& V);
		Index addAttributes();
		Index addAttributes(const RGBA& Color);
		Index addAttributes(const RGBA& Color, const Vertex &TexCoord);
		Index addAttributes(const RGBA& Color, const Vertex &TexCoord1, const Vertex &TexCoord2);
		Index addAttributes(const RGBA& Color, const Vertex &TexCoord1, const Vertex &TexCoord2, const Vertex &TexCoord3);
		Index addAttributes(const unsigned char bits, const RGBA& Color, const Vertex &TexCoord1, const Vertex &TexCoord2, const Vertex &TexCoord3, const Vertex &TexCoord3);
		virtual void flagSilhouetteEdges(const V3& viewPoint);
		virtual void flagVisibleEdges();
		void ClearVNA();			
		void buildNormalsFromVertices();
		void debugPrint();
		void getTriangle(Index i, Triangle *t);
		void getTriangleGeometry(Index i, TriangleGeometry &G);
		void Transform(const M44 & transform);
		void cacheTriangleAttributes();
		void Load(const char * fileName, const char * key, int format);
		Primitive* addPrimitive(PrimType primType, Index VertexCount, Index * Vertices, Index primNormal=-1, Index primAttributes=-1); 
		Primitive *addPrimitive(PrimType primType, Index VertexCount, Index * Vertices,
					Index * vertexNormals,
					Index * vertexAttributes,
					Index * triangleNormals,
					Index * triangleAttributes, 
					Index defaultNormal = -1,
					Index defaultAttributes = -1);
		virtual void defaultTopology();
		void buildEdgeTopology();
		virtual ~Mesh() {};
	};

	class Cube: public Mesh { // just to test
		FLOAT side;
	public:
		Cube();
		virtual void defaultTopology();
		void defaultVertexAttributes(FLOAT Side);
		virtual void flagVisibleEdges();
	};

	class Cylinder: public Mesh {
	protected:
		Index sections;
		Index sectors;
		FLOAT length;
		FLOAT radius;
		bool hasCaps;
	public:
		Cylinder(Index Sectors, Index Sections, bool HasCaps=true);
		virtual void defaultTopology();
		void defaultVertexAttributes(FLOAT Length, FLOAT Radius); // ?? virtualize these ones?
		virtual void flagVisibleEdges();
 		inline Index getSections() {return sections;}
		inline Index getSectors() {return sectors;}
		inline Index getMaxVertexCount() {return sectors*(sections+1)+2;}
		inline Index getMaxAttributesCount() {return getMaxVertexCount()+sectors*sections;}
	};

	class Sphere: public Cylinder {
	protected:
		void getSphericalCoords(Index I, Index j, V3& pt);
	public:
		Sphere(Index Sectors, Index Sections);		
		void defaultVertexAttributes(FLOAT Radius);
		virtual void flagVisibleEdges();
	};
	
	class CylDome: public Mesh {
	private:
		void surf(Index aStart, Index aStride, Index bStart, Index bStride, int count, int nStart, int nStride);
		void arch(FLOAT x0, FLOAT z0, FLOAT x1, FLOAT z1, int count);
		Index sectors;
		FLOAT height;
		FLOAT radius;
		int nCyl;
		int orient;
	public:		
		void defaultVertexAttributes();
		virtual void flagVisibleEdges();
		CylDome(Index sectors, FLOAT height, FLOAT radius, int nCyl, int orient);
		virtual void defaultTopology();
	};
};

#endif
