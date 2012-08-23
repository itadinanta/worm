#include "model.h"
#include <map>
#include <iterator>
#include <fstream>
#include "scanner.h"
#include <exception>
#include "log.h"

namespace model {
	class MeshException: public exception {
		mString message;
	public:
		MeshException(const mString &msg): message(msg) {}
		virtual ~MeshException() throw() {}
		virtual const char* what() const throw() {return message.c_str();}
	};

	class AcMeshLoader: public Scanner {	
		enum tokenMesh {tmagic=1, 
		      tMATERIAL, trgb, tamb, temis, tspec, tshi, ttrans,
		      tOBJECT, tObjWorld,
		      tkids, 
		      tObjPoly,
		      tname, tnumvert, 
		      tnumsurf,
		      tSURF,
		      tmat,
		      trefs,
		      ttexture,
		      ttexrep};
		      
		Mesh *currentMesh;
		Mesh *loadingMesh;
		const char *loadingMeshName;
		istream *iFile;
		vector<RGBA> Colors;
	protected:
		bool ReadNumberSeq(FLOAT * number) {
			switch (GetNextToken()) {
			case TOKEN_INT: 
			case TOKEN_FLOAT:
				return sscanf(GetLastWord().c_str(), "%f", number);
			default:
				PushBackWord();
				return false;
			}
		}

		int ReadIndex() {
			int val;
			switch (GetNextToken()) {
			case TOKEN_INT: 
				sscanf(GetLastWord().c_str(), "%d", &val);
				return val;
				break;
			default:
				return 0;
			}
		}
		FLOAT ReadNumber() {
			FLOAT f;
			ReadNumberSeq(&f);
			return f;
		}
		int ReadNumbers(FLOAT *numbers, int maxNumbers) {
			int count=0;
			for (count=0; count<maxNumbers; count++)
				if (!ReadNumberSeq(numbers++)) break;		
			return count;
		}
		RGBA ReadRGB() {
			FLOAT rgb[3];
			if (ReadNumbers(rgb,3)==3)
				return RGBA((unsigned char)round(rgb[0]*255), 
					    (unsigned char)round(rgb[1]*255), 
					    (unsigned char)round(rgb[2]*255),
					    255);
		}

		V3 ReadVertex() {
			V3 V;
			V.x=ReadNumber();
			V.y=ReadNumber();
			V.z=-ReadNumber();
			return V;
		}		

		void Expect(tokenMesh token) throw(exception) {
			if (GetNextToken()!=token)
				throw MeshException("Expected: "+GetSymbol(token)+", found "+GetLastWord());
		}
		
		void ReadMaterial() {
			Expect((tokenMesh)TOKEN_STRING); // Material name
			Expect(trgb); Colors.push_back(ReadRGB());
			Expect(tamb); ReadRGB();
			Expect(temis); ReadRGB();
			Expect(tspec); ReadRGB();
			Expect(tshi); ReadNumber();
			Expect(ttrans); ReadNumber();
		}

		void ReadPolySurfList() {
			int i,j, iVertex;
			int pCount=ReadIndex();
			int nCount;
			int iMaterial;
			V3 texCoord(0,0,0);
			for (i=0; i<pCount; i++) {
				Expect(tSURF);
				GetNextWord(); // what the hell is this?
				Expect(tmat); iMaterial=ReadIndex();
				Expect(trefs); nCount=ReadIndex();
				Index SurfRefs[nCount];
				Index MatRefs[nCount];
				for (j=0; j<nCount; j++) {
					iVertex = ReadIndex();
					texCoord.x = 1-ReadNumber();
					texCoord.y = 1-ReadNumber();
					if (currentMesh) {
					    SurfRefs[j]=iVertex;
					    MatRefs[j]=currentMesh->addAttributes(Colors[iMaterial], texCoord);
					}
				}
				if (currentMesh)
					currentMesh->addPrimitive(PRIM_FAN, nCount, SurfRefs, 0, MatRefs, 0, 0);
			}
		}

		void ReadPolyVertices() {
			int i;
			int vCount=ReadIndex();
			V3 cVertex;
			for (i=0; i<vCount; i++) {
				cVertex=ReadVertex();
				if (currentMesh) {
					currentMesh->addVertex(cVertex);
//					cerr << cVertex;
				}
			}
		}

		void ReadPoly() {
			mString aName;
			while (!Eof()) {
				switch (GetNextToken()) {
				case tname:
					aName=escape(GetNextWord());
//					cerr << aName << " found\n";
				if (aName==loadingMeshName)
					currentMesh=loadingMesh;
				else
					currentMesh=0;				
				break;
				case ttexture:
					Expect((tokenMesh)TOKEN_STRING);
					break;
				case tnumvert:
					ReadPolyVertices();
					break;
				case tnumsurf:
					ReadPolySurfList();
					break;
				case tkids:
					PushBackWord();
					currentMesh=0;
					return;
				case ttexrep:
					ReadNumber(); // ignore this at the moment
					ReadNumber();
					break;
				default:
					GetNextWord();
					break;
				}
			}
		}

		void ReadObjectTree() {
			switch (GetNextToken()) {
			case tObjWorld:
				break;
			case tObjPoly:
				ReadPoly();
				break;
			}
			Expect(tkids);
			int kidCount=ReadIndex();
			int i;
			for (i=0; i<kidCount;i++) {
				Expect(tOBJECT);
				ReadObjectTree();
			}
		}
	public:
		void ExtractMesh(const char * meshName, Mesh* destination) {
			currentMesh = 0;
			loadingMesh = destination;
			loadingMeshName = meshName;
			istreamParserStream F(iFile);
			BeginParsing(&F);
			Expect(tmagic);
			while (!Eof()) {
				switch(GetNextToken()) {
				case TOKEN_EOF: 
					break;
				case tMATERIAL: ReadMaterial();
					break;
				case tOBJECT: ReadObjectTree();
					break;
				}
			}
			EndParsing();
		}
		AcMeshLoader(istream * input) {
			iFile=input;
			AddBlank(' ');
			AddBlank('\t');
			AddBlank('\x0d');
			AddBlank('\x0a');
			
//	AddSeparator('|', '|');
//	AddSeparator('&');
			AddToken("AC3Db", tmagic);
			AddToken("MATERIAL", tMATERIAL);
			AddToken("OBJECT", tOBJECT);
			AddToken("SURF", tSURF);
			AddToken("rgb", trgb);
			AddToken("amb", tamb);
			AddToken("emis",temis);
			AddToken("spec",tspec);
			AddToken("shi", tshi);
			AddToken("trans", ttrans);
			AddToken("world", tObjWorld);
			AddToken("kids", tkids);
			AddToken("poly", tObjPoly);
			AddToken("name", tname);
			AddToken("numvert", tnumvert);
			AddToken("numsurf", tnumsurf);
			AddToken("mat", tmat);			
			AddToken("refs", trefs);
			AddToken("texture", ttexture);
			AddToken("texrep", ttexrep);

		};
	};
		
		
	/* ------------------------------------------------------------------------- */
	
	Primitive::Primitive(int _baseIndex, PrimType _primType, int _vertexCount):
		baseIndex(_baseIndex), 
		primType(_primType),
		vertexCount(_vertexCount) {
		vertices = 0;
		vertexNormals = 0;
		triangleNormals = 0;
		vertexAttributes = 0;
		triangleAttributes = 0;
		defaultAttribute= -1;
		defaultNormal = -1;
	}
	Primitive::~Primitive() {
		freeIndices(vertices);
		freeIndices(vertexAttributes);
		freeIndices(triangleAttributes);
		freeIndices(vertexNormals);
		freeIndices(triangleNormals);
	}
	void Primitive::Fill(Index *v, Index value, Index count) {
		count=translateCount(count);
		if (v) while (count--) *v++=value;
	}
	void Primitive::Fill(Index *v, Index* values, Index count) {
		count=translateCount(count);
		if (v && values) memcpy(v, values, count * sizeof(Index));
	}
	void Primitive::Fill(Index *v, Index * values, Index defaultValue, Index count) {
		if (!v) return;
		if (values) Fill(v, values, count); 
		else Fill(v, defaultValue, count);
	}
	void Primitive::getTriangle(Index rIndex, Triangle *T) {
		getTriangleRelative(rIndex - getTriangleFirst(), T);
	}      
	void Primitive::getTriangleRelative(Index i, Triangle *T) {
		if (!T) return;
		int i1, i2, i3, n1, n2, n3, a1, a2, a3, ap, np;
		switch (primType) {
		case PRIM_FAN:
			i1=0; i2=i+1; i3=i+2;
			break;
		case PRIM_STRIP:	
			i3=i+2;
			if (i & 1) { i1=i+1; i2=i;} 
			else { i1=i;i2=i+1; }
			break;
		default:
			i1=i; i2=i+1; i3=i+2;
		}
		if (vertexAttributes) {
			a1=vertexAttributes[i1];
			a2=vertexAttributes[i2];
			a3=vertexAttributes[i3];
		}
		else {
			a1=a2=a3=defaultAttribute;
		}
		if (vertexNormals) {
			n1=vertexNormals[i1];
			n2=vertexNormals[i2];
			n3=vertexNormals[i3];
		}
		else {
			n1=n2=n3=defaultNormal;
		}
		ap=(triangleAttributes)
			? triangleAttributes[i]
			: defaultAttribute;
		np=(triangleNormals)
			? triangleNormals[i]
			: defaultNormal;
		T->setup(vertices[i1], vertices[i2], vertices[i3],n1,n2,n3,a1,a2,a3,np,ap);
	};	
	Index Primitive::getVertexCount() {
		return vertexCount;
	};
	Index Primitive::getTriangleCount() {
		return vertexCount - 2;
	};
	Index Primitive::getTriangleFirst() {
		return baseIndex;
	};
	Index Primitive::getTriangleLast() {
		return baseIndex+getTriangleCount()-1;
	};
	void Primitive::setVertexList(Index * bufVertices) {
		if (bufVertices) {
			if (!vertices) vertices=allocIndices(COUNT_VERTICES);
			Fill(vertices, bufVertices, COUNT_VERTICES);
		};
	};
	void Primitive::setNormalList(Index * bufVertexNormals, Index* bufTriangleNormals, Index primNormal) {
		if (bufVertexNormals) {
			if (!vertexNormals) vertexNormals=allocIndices(COUNT_VERTICES);
			Fill(vertexNormals, bufVertexNormals, COUNT_VERTICES);
		};

		if (bufTriangleNormals) {
			if (!triangleNormals) triangleNormals=allocIndices(COUNT_TRIANGLES);
			Fill(triangleNormals, bufTriangleNormals, COUNT_TRIANGLES);
		};
		setDefaultNormal(primNormal);
	};
	void Primitive::setAttributesList(Index * bufVertexAttributes, Index* bufTriangleAttributes, Index primAttributes) {
		if (bufVertexAttributes) {
			if (!vertexAttributes) vertexAttributes=allocIndices(COUNT_VERTICES);
			Fill(vertexAttributes, bufVertexAttributes, COUNT_VERTICES);
		};
		if (bufTriangleAttributes) {
			if (!triangleAttributes) triangleAttributes=allocIndices(COUNT_TRIANGLES);
			Fill(triangleAttributes, bufTriangleAttributes, COUNT_TRIANGLES);
		};
		setDefaultAttribute(primAttributes);
	};

	void Primitive::getVertexList(Index * bufVertices) {
		if (bufVertices && vertices)
			Fill(bufVertices, vertices, -1, COUNT_VERTICES);
	};

	void Primitive::getNormalList(Index * bufVertexNormals, Index * bufTriangleNormals) {
		if (bufVertexNormals) 
			Fill(bufVertexNormals, vertexNormals, defaultNormal, COUNT_VERTICES);
		if (bufTriangleNormals) 
			Fill(bufTriangleNormals, triangleNormals, defaultNormal, COUNT_TRIANGLES);
	};

	void Primitive::getAttributesList(Index * bufVertexAttributes, Index * bufTriangleAttributes) {
		if (bufVertexAttributes) 
			Fill(bufVertexAttributes, vertexAttributes, defaultAttribute, COUNT_VERTICES);
		if (bufTriangleAttributes) 
			Fill(bufTriangleAttributes, triangleAttributes, defaultAttribute, COUNT_TRIANGLES);
	};


	Index Primitive::translateCount(Index count) {
		switch (count) {
		case COUNT_VERTICES: count = getVertexCount();
			break;
		case COUNT_TRIANGLES: count = getTriangleCount();
			break;
		};
		return count;	
	};

	Index *Primitive::allocIndices(Index count) {
		count=translateCount(count);
		log << LLDebug << "Allocating "<< count << " Index items\n";
		return (Index*)calloc(count, sizeof(Index));
	};

	void Primitive::freeIndices(Index * buf) {
		if (buf) free(buf);
	};
/* ------------------------------------------------------------------------- */

	PrimitiveArray::~PrimitiveArray() {
		for (iterator i=begin(); i!=end(); i++)
			delete (*i);		
	}

/* ------------------------------------------------------------------------- */

	Mesh::Mesh() {
	}
	void Mesh::ClearVNA() {
		Vertices.clear();
		Normals.clear();
		Attributes.clear();
	}
	Index Mesh::addVertex(const Vertex& V) {
		log << LLDebug << "Vertex " <<Vertices.size()<< " = " << V;
		Vertices.push_back(V);
		return Vertices.size()-1;
	}
	Index Mesh::addNormal(const Vertex& N) {
		log << LLDebug << "Vertex " <<Vertices.size()<< " = " << N;
		Normals.push_back(N);
		return Normals.size()-1;
	}
	inline Index Mesh::addAttributes() {
		V3 T(0,0,0);
		return addAttributes(0, RGBA(0,0,0,255),T,T,T,T);
	}
	inline Index Mesh::addAttributes(const RGBA& Color) {
		V3 T(0,0,0);
		return addAttributes(ATTRIB_COLOR, Color,T,T,T,T);
	}
	inline Index Mesh::addAttributes(const RGBA& Color, const Vertex &TexCoord0) {
		return addAttributes(ATTRIB_COLOR|ATTRIB_TEXCOORD0,
				     Color,TexCoord0,TexCoord0,TexCoord0,TexCoord0);
	}
	inline Index Mesh::addAttributes(const RGBA& Color, 
				   const Vertex &TexCoord0, 
				   const Vertex &TexCoord1) {
		return addAttributes(ATTRIB_COLOR|ATTRIB_TEXCOORD0|ATTRIB_TEXCOORD1,
				     Color,TexCoord0,TexCoord1,TexCoord1,TexCoord1);
	}
	inline Index Mesh::addAttributes(
		const RGBA& Color, 
		const Vertex &TexCoord0, 
		const Vertex &TexCoord1, 
		const Vertex &TexCoord2) {
		return addAttributes(
			ATTRIB_COLOR|ATTRIB_TEXCOORD0|ATTRIB_TEXCOORD1|ATTRIB_TEXCOORD2,
			Color,TexCoord0,TexCoord1,TexCoord2,TexCoord2);
	};
	Index Mesh::addAttributes(
		const unsigned char bits,
		const RGBA& Color, 
		const Vertex &TexCoord0, 
		const Vertex &TexCoord1, 
		const Vertex &TexCoord2, 
		const Vertex &TexCoord3) {
		model::Attributes a;
		a.flags=bits;
		a.TexCoord[0]=TexCoord0;
		a.TexCoord[1]=TexCoord1;
		a.TexCoord[2]=TexCoord2;
		a.TexCoord[3]=TexCoord3; 	
		a.Color=Color;
		Attributes.push_back(a);
		return Attributes.size()-1;
	};
	Index Mesh::getTriangleCount() const {
		return TriangleIndex.size();
	};
	Primitive* Mesh::CreatePrimitive(PrimType primType, Index VertexCount) {
		return new Primitive(getTriangleCount(), primType, VertexCount);
	};
	Primitive* Mesh::addPrimitive(PrimType primType, int VertexCount, Index * v,
				      Index * vertexNormals,
				      Index * vertexAttributes,
				      Index * triangleNormals,
				      Index * triangleAttributes, 
				      Index primNormal,
				      Index primAttributes) { // Index* va, Index * ta, Index defAttribute) {
		Primitive * p=addPrimitive(primType,VertexCount,v);
		if (p) {
			p->setNormalList(vertexNormals, triangleNormals, primNormal);
			p->setAttributesList(vertexAttributes, triangleAttributes, primAttributes);
		}
		return p;		
	};

	Primitive* Mesh::addPrimitive(PrimType primType, int VertexCount, Index * v, Index primAttributes, Index primNormal) {
		if (VertexCount>2) {
			Primitive * primSource = CreatePrimitive(primType, VertexCount);
			int nTri = primSource->getTriangleCount();
			// to find a triangle quickly
			register const int nMax=Primitives.size();
			for (int i=0; i<nTri; i++) {
//			DEBUG("TriangleIndex[" << TriangleIndex.size() << "] = "<<nMax<<'\n');
				TriangleIndex.push_back(nMax); 
			}
			
			Primitives.push_back(primSource);
//		DEBUG("Primitive ("<<Primitives.size()<<")\n");
			primSource->setVertexList(v);
			primSource->setNormalList(0,0,primNormal);
			primSource->setAttributesList(0,0,primAttributes);
			return primSource;
		}
		else return 0;
	};

	void Mesh::addEdge(Index i1, Index i2, Index t, Index i3) {
		int e1, e2;
		EdgeId eid(i1, i2);
		int eIndex;
		EdgeMap::iterator edgeItem = revEdgeMap.find(eid);
		if (edgeItem!=revEdgeMap.end()) { // new edge (maybe...)
			eid=edgeItem->first;
			eIndex=edgeItem->second;
			log << LLDebug<< "\tEdge found ";
		}
		else {
			eid=EdgeId(i1,i2);
			eIndex=Edges.size();
			revEdgeMap[eid] = eIndex;
			Edges.push_back(Edge(i1,i2));
			log << LLDebug << "\tNew edge ";
		}
		log << LLDebug << i1 << ", " << i2 << " of triangle " << t << "\n";
		Edge &ed=Edges[eIndex];
		ed.setTriangle(i1,i2,t,i3);
	}

	void Mesh::getTriangle(Index i, Triangle *t) {
		Primitive *p=Primitives[TriangleIndex[i]];
		if (p) p->getTriangle(i, t);
	}
	void Mesh::getTriangleGeometry(Index i, TriangleGeometry &G) {
		Triangle T;
		getTriangle(i, &T);
		G.V[0]=Vertices[T.Vertex[0]];
		G.V[1]=Vertices[T.Vertex[1]];
		G.V[2]=Vertices[T.Vertex[2]];
	}
	void Mesh::Transform(const M44 & transform) {
		V3 N;
		V4 V1, V2;

		for (PointArray::iterator iV=Vertices.begin(); iV!=Vertices.end(); iV++) {
			V2=*iV;
			transform.mult(V1, V2);
			*iV=V1;
		}
		for (PointArray::iterator iA=Normals.begin(); iA!=Normals.end(); iA++) {
			transform.mult3(N, *iA);
			*iA=N;
		}
	}
	
	void Mesh::approxNormalsXYZ(int count, V3* points, V3* normals, V3 up) {
		V3 d;
		V3 r;
		int i;
		for (i=0; i<count; i++) {
			if (i<count-1) d=points[i+1]; else d=points[i];
			if (i) d-=points[i-1]; else d-=points[i];
			r.setVec(d,up);
			normals[i].setVec(r,d);
			normals[i].normalize();
		}
	}
	void Mesh::normalsForVerticesFromVertices() {
		normalsForTrianglesFromVertices();
		normalsForVerticesFromTriangles();
	}
	void Mesh::createEmptyNormals() {
		Index i;
		Index firstVertexNormal=Normals.size();
// adds an empty normal for each vertex
		for (i=0; i< Vertices.size(); i++) addNormal(V3(0,0,0)); 
		Primitive *p;
// sets a fresh list of normals for each primitive
		for (PrimitiveArray::iterator iP=Primitives.begin(); iP!=Primitives.end(); iP++) {
			p=(*iP);
			Index *bufVertices=p->allocIndices(COUNT_VERTICES);
			p->getVertexList(bufVertices);
			for (i=0; i<p->getVertexCount(); i++) 
				bufVertices[i]+=firstVertexNormal;
			p->setNormalList(bufVertices);
			p->freeIndices(bufVertices);
		}
	}
	void Mesh::normalsForVerticesFromTriangles() {
		Index i,iT, nT=getTriangleCount();
		Primitive *p;
		for (iT=0; iT<getTriangleCount(); iT++) {
			Triangle t;
			getTriangle(iT, &t);
			if (t.TriangleNormal>=0) {
				V3 &N=Normals[t.TriangleNormal];
				if (t.Normal[0]>=0)
					Normals[t.Normal[0]]+=N;
				if (t.Normal[1]>=0)
					Normals[t.Normal[1]]+=N;
				if (t.Normal[2]>=0 && t.Normal[2]<Normals.size())
					Normals[t.Normal[2]]+=N;
			}
			
		}
		for (PrimitiveArray::iterator iP=Primitives.begin(); iP!=Primitives.end(); iP++) {
			p=(*iP);
			Index bufNormals[p->getVertexCount()];
			p->getNormalList(bufNormals);
			for (i=0; i< p->getVertexCount(); i++)
				Normals[bufNormals[i]].normalize();
		}
	}

	void Mesh::normalsForTrianglesFromVertices() {
		Primitive *p;
		for (PrimitiveArray::iterator iP=Primitives.begin(); iP!=Primitives.end(); iP++) {
			p=(*iP);
			Index iT, nT=p->getTriangleCount();
			Index *tmpIdx=p->allocIndices(nT);
			Triangle t;
			V3 A, B, C;
			for (iT=0; iT<nT; iT++) {
				p->getTriangleRelative(iT, &t);
				A=Vertices[t.Vertex[0]];
				B=Vertices[t.Vertex[1]];
				C=Vertices[t.Vertex[2]];
				C-=A;
				B-=A;
				A.setVec(C,B); // triangles are stored clockwise
				A.normalize();
				tmpIdx[iT]=addNormal(A);
			}
			p->setNormalList(0, tmpIdx, p->getDefaultNormal());
			p->freeIndices(tmpIdx);
		}
	}

	void Mesh::buildEdgeTopology () {
		revEdgeMap.clear();
		Edges.clear();
		Index n=getTriangleCount();
		Triangle t;
		for (Index i=0; i<n; i++) {
			getTriangle(i, &t);
			log << LLDebug << "Triangle " 
			     << i << ":\t"
			     << t.Vertex[0] << ", " 
			     << t.Vertex[1] << ", " 
			     << t.Vertex[2] << "\n";
			addEdge(t.Vertex[0], t.Vertex[1], i, t.Vertex[2]);
			addEdge(t.Vertex[1], t.Vertex[2], i, t.Vertex[0]);
			addEdge(t.Vertex[2], t.Vertex[0], i, t.Vertex[1]);
		}
		revEdgeMap.clear(); // frees memory
	}

	void Mesh::cacheTriangleAttributes() {
		Index t1,t2,n1,n2,a1,a2;
		Triangle t;
		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {
			t1=edge->Triangles[0];
			if (t1>=0) {
				getTriangle(t1, &t);
				a1=t.TriangleAttributes;
				n1=t.TriangleNormal;
			} else {
				a1=-1;
				n1=-1;
			}
			t2=edge->Triangles[1];
			if (t2>=0) {
				getTriangle(t2, &t);
				a2=t.TriangleAttributes;
				n2=t.TriangleNormal;
			} else {
				a2=-1;
				n2=-1;
			}
			edge->setAttributes(n1,n2,a1,a2);
		}
	}
	
	void Mesh::flagVisibleEdges() {
 		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {			
			if ((edge->CompVertex[0]<0 || edge->CompVertex[1]<0))
				edge->flags |= EDGE_MODEL;
		}
	}
	
	void Mesh::flagSilhouetteEdges(const V3& viewPoint) {
		Triangle t;
		Index n=getTriangleCount();
		char* orientation=(char*)calloc(n,1);
		V3 viewVector;
		Index tNormal;
		for (Index i=0; i<n; i++) {
			getTriangle(i, &t);
			viewVector=viewPoint;
			viewVector-=Vertices[t.Vertex[0]]; // the first vertex should be fine
			tNormal=t.TriangleNormal;
			if (tNormal>=0) {
				orientation[i]=viewVector.dot(Normals[tNormal])>=0;
				log << LLDebug << "Triangle " << i << "\t"
				    << "viewVector =\t" << viewVector 
				    << "\t\tnormal =\t" << Normals[tNormal]
				    << "\t\torientation =\t" << (int)orientation[i] << "\n";
			}
		}
		Index t1, t2, a1, a2;
		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {
			t1=edge->Triangles[0];
			t2=edge->Triangles[1];
			bool silhouette = (orientation[t1] != orientation[t2]);
			log << LLDebug << "Edge " << (edge - Edges.begin()) << ": \t"
			    << edge->Vertex[0] << " -> "
			    << edge->Vertex[1] << "\t"
			    << t1 << " | "
			    << t2 << "\t visibility: "
			    << silhouette << "\n";
			if (silhouette) edge->flags |= EDGE_SILHOUETTE;
			else edge->flags &= ~EDGE_SILHOUETTE;
		}
		free((void*)orientation);	
	}

	void Mesh::Load(const char * filename, const char * key, int format) {	
		ifstream f(filename);
		Index nVertices, nPrims, nAttribs;
		Index nVPrim, i, j, idx;
		V3 V;
		if (f) {
			switch (format) {
			case FFORMAT_OFF: {
				f.ignore(1024,'\n'); // skips first line
				f >> nVertices >> nPrims >> nAttribs;
				Vertices.reserve(nVertices);
				Primitives.reserve(nPrims);
				for (i=0; i<nVertices; i++) {
					f >> V.x >> V.y >> V.z;
					addVertex(V);
				}
				
				for (i=0; i<nPrims; i++) {
					f >> nVPrim;
					Index primTmp[nVPrim];
					for (j=0; j<nVPrim; j++) {
						f >> idx;
						primTmp[j] = idx;
					}
					addPrimitive(PRIM_FAN, nVPrim, primTmp);
				}
				for (i=0; i<nAttribs; i++) {
				}
			}
			case FFORMAT_AC3D: {
				AcMeshLoader Loader(&f);
				Loader.ExtractMesh(key, this);
			}
			}
			buildEdgeTopology();
			createEmptyNormals();
			normalsForTrianglesFromVertices();
			normalsForVerticesFromTriangles();
			cacheTriangleAttributes();				
		}
	};

	void Mesh::defaultTopology() {
	}

	void Mesh::debugPrint() {
		PrimitiveArray::iterator pIter;
		for (pIter=Primitives.begin(); pIter!=Primitives.end(); pIter++) {
			Primitive *p=(*pIter);
			log << LLInfo << "Primitive: " << (pIter-Primitives.begin())<<"\n";
			Index vCount=p->getVertexCount();
			Index *vertexList = p->allocIndices(vCount);
			Index *normalList = p->allocIndices(vCount);
			Index *attributesList = p->allocIndices(vCount);
			p->getVertexList(vertexList);
			p->getNormalList(normalList);
			p->getAttributesList(attributesList);
			switch (p->getPrimType()) {
			case PRIM_POLY: log << LLInfo << "PRIM_POLY\n";
				break;
			case PRIM_STRIP: log << LLInfo << "PRIM_STRIP\n";
				break;
			case PRIM_FAN: log << LLInfo << "PRIM_FAN\n";
				break;
			default: log << LLInfo << "PRIM_UNKNOWN\n";
				break;
			}			
			for (int i=0; i<vCount; i++) {
				if (!i || normalList[i]!=normalList[i-1]) {
					if (normalList[i]>=0) {
						model::Normal &n=this->Normals[normalList[i]];
						log << LLInfo << "Normal[" <<normalList[i] <<"] = " << n;
					}
				}
				if (!i || attributesList[i]!=attributesList[i-1]) {
					if (attributesList[i]>=0) {
						model::Attributes &a=this->Attributes[attributesList[i]];
						log << LLInfo << "Color "<<(int)a.Color.r<<", "<<(int)a.Color.g<<", "<<(int)a.Color.b<<", "<<(int)a.Color.a<<"\t"<<
							"Att[" << attributesList[i] << "] = " << a.TexCoord[0];
					}
				};
				if (vertexList[i]>=0) {
					V3 &V=Vertices[vertexList[i]];
					log << LLInfo << "Vertex["<<vertexList[i]<<"] ="
					       <<V;
				};
			}
			p->freeIndices(normalList);
			p->freeIndices(attributesList);
			p->freeIndices(vertexList);
			log << LLInfo << "\n\n";
		}	
		log << LLInfo << "Ok\n\n";
	}

/* ------------------------------------------------------------------------- */
	
	Cube::Cube() {
		defaultTopology();
	}
	void Cube::defaultTopology() {
		int faces[][4]={ {0, 1, 2, 3}, // Z+
				 {4, 5, 6, 7}, // Z-
				 {2, 5, 4, 3}, // X+
				 {0, 7, 6, 1}, // X-			 			 
				 {1, 6, 5, 2}, // Y+
				 {3, 4, 7, 0} }; // Y-

		int attr[]={0,1,2,3};

		for (int i=0; i<6; i++) // a cube has 6 faces!
			addPrimitive(PRIM_FAN, 4, faces[i], 0, attr, 0, 0, i, -1);
		Vertices.reserve(12);
		Normals.reserve(6);
		Attributes.reserve(6);
		
		buildEdgeTopology();
	};

	void Cube::flagVisibleEdges() {
 		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {	
			if (edge->Normal[0]!=edge->Normal[1]) {
				edge->flags |= EDGE_MODEL;
			}
		}
	}

	void Cube::defaultVertexAttributes(FLOAT Side) {
		side=Side*0.5;
		ClearVNA();
		addVertex(V3(-side, -side,  side));//0
		addVertex(V3(-side,  side,  side));//1
		addVertex(V3( side,  side,  side));//2
		addVertex(V3( side, -side,  side));//3
		addVertex(V3( side, -side, -side));//4
		addVertex(V3( side,  side, -side));//5
		addVertex(V3(-side,  side, -side));//6
		addVertex(V3(-side, -side, -side));//7
		
		addAttributes(RGBA(255,100,120,255),V3(0,1,0));
		addAttributes(RGBA(255,100,120,255),V3(0,0,0));
		addAttributes(RGBA(255,100,120,255),V3(1,0,0));
		addAttributes(RGBA(255,100,120,255),V3(1,1,0));

		addNormal(V3( 0, 0,+1)); //);
		addNormal(V3( 0, 0,-1)); // RGBA(0xff, 0xff, 0x80, 0xff));
		addNormal(V3(+1, 0, 0)); //,RGBA(0x80, 0xff, 0x80, 0xff));
		addNormal(V3(-1, 0, 0)); // RGBA(0x80, 0xff, 0xff, 0xff));
		addNormal(V3( 0,+1, 0)); //RGBA(0x80, 0x80, 0xff, 0xff));
		addNormal(V3( 0,-1, 0)); //RGBA(0xff, 0x80, 0xff, 0xff));
	}

/* ------------------------------------------------------------------------- */
 	Cylinder::Cylinder(Index Sectors, Index Sections, bool HasCaps) {
		sectors=Sectors;
		sections=Sections;
		hasCaps=HasCaps;
		Vertices.reserve(getMaxVertexCount());
		Normals.reserve(getMaxAttributesCount());
		Attributes.reserve(getMaxAttributesCount());
		defaultTopology();
	}

	void Cylinder::defaultTopology() {
		const Index secPoints = sectors * 2 + 2;
		Index strip[secPoints], i, step;
		for (i=0; i<=sectors; i++) {
			strip[2*i+1]=i % sectors;
			strip[2*i]=strip[2*i+1] + sectors;
		}
		for (step=1; step<=sections; step++) {
			addPrimitive(PRIM_STRIP, secPoints, strip, strip, 0, 0, 0, -1, 1);
			for (i=0; i<secPoints; i++) strip[i] += sectors;
		}
		if (hasCaps) {
			Index aCount=getMaxAttributesCount();
			Index vCount=getMaxVertexCount();
			// adds first cap
			for (i=0; i<sectors; i++) strip[i+1]=sectors-i-1;
			strip[0] = vCount-2; strip[sectors+1]=strip[1];
			addPrimitive(PRIM_FAN, sectors+2, strip, 0, 0, 0, 0, vCount-2, 0);
			// adds last cap
			for (i=0; i<sectors; i++) strip[i+1]=sectors*sections+i;
			strip[0]++; strip[sectors+1]=strip[1];
			addPrimitive(PRIM_FAN, sectors+2, strip, 0, 0, 0, 0, vCount-1, 0);
		}
		buildEdgeTopology();
	}

	void Cylinder::flagVisibleEdges() {
		Index vCount=getMaxVertexCount();
 		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {			
			if (hasCaps) {
				Index diff=edge->Vertex[0]-edge->Vertex[1];
				if ((edge->CompVertex[0]>=(vCount-2)) || (edge->CompVertex[1]>=(vCount-2)))
					edge->flags |= EDGE_MODEL;
			} else {
				if ((edge->CompVertex[0]<0 || edge->CompVertex[1]<0) && 
				    (edge->Vertex[1]<sectors))
					edge->flags |= EDGE_MODEL;
			}
		}
	}

	void Cylinder::defaultVertexAttributes(FLOAT Length, FLOAT Radius) { // ?? virtualize these ones? {
		ClearVNA();
	}

/* ------------------------------------------------------------------------- */

	Sphere::Sphere(Index Sectors, Index Sections): Cylinder(Sectors,Sections) {
		
	}
	
	void Sphere::getSphericalCoords(Index i, Index j, V3& pt) {
		FLOAT a=0.025+0.95*i*M_PI/sections+(-M_PI/2);
		FLOAT g=j*M_PI*2/sectors;
		FLOAT r1=cos(a);
		pt.y=sin(a);
		pt.x=r1*cos(g);
		pt.z=r1*sin(g);
	}

	void Sphere::defaultVertexAttributes(FLOAT Radius) {
		radius=Radius;
		ClearVNA();
		V3 V;
		RGBA col[]={RGBA(255,0,255,255), RGBA(0,255,0,255)};
		for (Index i=0; i<=sections; i++) {
			for (Index j=0; j<sectors; j++) {
				getSphericalCoords(i,j,V);
				addNormal(V);
				V*=radius;
				addVertex(V);
			}			
		}
		addNormal(V3(0,-1,0));
		addNormal(V3(0,1,0));
		addAttributes(col[0]);
		addAttributes(col[1]);
		addVertex(V3(0,-radius,0));
		addVertex(V3(0, radius,0));
	}

	void Sphere::flagVisibleEdges() {
		Cylinder::flagVisibleEdges();
	}	

/* ------------------------------------------------------------------------- */
	CylDome::CylDome(Index sectors, FLOAT height, FLOAT radius, int nCyl, int orient) {
		this->sectors=sectors;
		this->height=height;
		this->radius=radius;
		this->nCyl=nCyl;
		this->orient=orient;
		defaultTopology();
		defaultVertexAttributes();
	}

	void CylDome::defaultVertexAttributes() {
		arch(-1, 1, 1, 1, sectors);
		FLOAT s2=sqrt(2.0)*0.5;
		switch (nCyl) {
 		case 1: arch(-1,-1, 1,-1, sectors);
			break;
		case 2: arch( 1,-1, 1, 1, sectors);
			arch(-1,-1, 1, 1, sectors);
			break;
		case 3: arch( 1,-1, 1, 1, sectors); //pts
			arch(-1,-1, 1,-1, sectors); //2*pts
			arch(-1,-1, 1, 1, sectors); //3*pts
			arch(-1, 1, 1,-1, sectors); //4*pts
			break;
		case 4: arch( 1,-1, 1, 1, sectors); //pts
			arch(-1,-1, 1,-1, sectors); //2*pts
			arch(-1,-1, 1,-1, sectors);
			arch(-1, 1, 1, 1, sectors);
			arch( 1, 1, 1,-1, sectors);
			break;
		}
	}

	void CylDome::flagVisibleEdges() {

		int elimit;
		switch(nCyl) {
		case 1: elimit=2*(sectors+1); break;
		case 2: elimit=2*(sectors+1); break;
		case 3: elimit=3*(sectors+1); break;
		case 4: elimit=4*(sectors+1); break;
		}
 		for (EdgeArray::iterator edge=Edges.begin(); edge!=Edges.end(); edge++) {	
			if ((edge->Vertex[0]>=elimit) && (edge->Vertex[1]>=elimit))
				edge->flags |= EDGE_MODEL;
		}
	}

	void CylDome::defaultTopology() {
		int pts=sectors+1;
		int hpts=(sectors/2) + 1;
		switch (nCyl) {
		case 1: surf(0, 1, pts, 1, pts, 0, 1);
			break;
		case 2: surf(0, 1, 2*pts, 1, pts, 0, 1);
			surf(2*pts, 1, pts, 1, pts,  pts,1);
			break;
		case 3: surf(0, 1, 2*pts, 1, hpts,0,1);
			surf(hpts-1, 1, 3*pts+hpts-1, 1, hpts,hpts-1,1);
			surf(3*pts+hpts-1, 1, pts+hpts-1, 1, hpts, pts+hpts-1, 1);
			surf(pts+hpts-1,-1, 4*pts+hpts-1, 1, hpts, pts+hpts-1, -1);
			surf(4*pts+hpts-1, 1,2*pts+hpts-1,1, hpts, 2*pts+hpts-1,1);
			break;
		case 4:
			break;
		}
		buildEdgeTopology();
		flagVisibleEdges();
	}

	void CylDome::surf(Index aStart, Index aStride, Index bStart, Index bStride, int count, int nStart, int nStride) {
		Index strip[count*2];
		Index stripn[count*2];
		for (int i=0; i<count; i++) {
			strip[i*2]=aStart+((i*aStride) % (2*count));
			strip[i*2+1]=bStart+((i*bStride) % (2*count));
			stripn[i*2]=nStart+(i*nStride);
			stripn[i*2+1]=stripn[i*2];
		};
		addPrimitive(PRIM_STRIP, count*2, strip, stripn, strip, 0, 0);
	};

	void CylDome::arch(FLOAT x0, FLOAT z0, FLOAT x1, FLOAT z1, int count) {
		FLOAT oSin[]={0,1,0,-1};
		FLOAT oCos[]={1,0,-1,0};
		FLOAT t,tq,x,z,xm,zm,alpha;
		orient %= 4;
		V3 V,D;
		xm=(x0+x1)*0.5;
		zm=(z0+z1)*0.5;
		V3 vert[count+1];
		V3 norm[count+1];
		for (int i=0; i<=count; i++) {
			t=i/(FLOAT)count;
			tq=2*t-1;
			x=x0+(x1-x0)*t;
			z=z0+(z1-z0)*t;
			V.x=x*radius;
			V.y=sqrt(1-tq*tq)*height;
			V.z=z*radius;
			D.x= V.x*oCos[orient] - V.z*oSin[orient];
			D.y= V.y;
			D.z= V.x*oSin[orient] + V.z*oCos[orient];			
			log << LLDebug << "x: "<<x<<", z: "<<z<<" ->"<<D;
			vert[i]=D;
			
		}		
		approxNormalsXYZ(count+1, vert, norm, V3::aY);
		for (int i=0; i<=count; i++) {
			addVertex(vert[i]);
			addNormal(norm[i]);
			addAttributes();
		}
	}

}
