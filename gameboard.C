#include "gameboard.h"
#include "model.h"
#include "log.h"
namespace model {
	char *gameboardStrings[] = {
		"/---------T----T----T----T----7",
		"|         |    |    |    |    |",
		"E----+----+----+----+----+----3",
		"|         |    |    |    |    |",
		"|         |    |    |    |    |",
		"L---------+----+----+----+----3",
		"          |    |    |    |    |",
		"          |    |    |    |    |",
		"          |    |    |    |    |",
		"          |    |    |    |    |",
		"     /----'    E----+----+----3",
		"     |         |    |    |    |",
		"     |         |    |    |    |",
		"     |         |    |    |    |",
		"     |         |    |    |    |",
		"/----+----+----+----+----+----3",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"E----+----+----+----+----+----3",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"E----+----+----+----+----+----3",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"|    |    |    |    |    |    |",
		"L----^----^----^----^----^----'"
	};

	BoardCell::BoardCell() {
	};
	BoardCell::~BoardCell() {
	};
	void BoardCell::AddTriangle(Index triangle) {
		triangles.push_back(triangle);
	};
 	void CellMatrix::init(int rows, int cols) {
		Rows=rows;
		Cols=cols;
		clear();
		reserve(Rows);
		for (int i=0; i<Rows; i++) {
			vector<BoardCell> V;
			V.resize(Cols);
			push_back(V);
		};
	}
	GameBoard::GameBoard(FLOAT scale): 
		Scale(scale) {};
	void GameBoard::Setup(Mesh *ground) {
		Ground=ground;
		BuildCellTrianglesList();
	};
	void GameBoard::Load() {
		char item;
		for (int i=0; i<Cells.getRows(); i++) 
			for (int j=0; j<Cells.getCols(); j++) {
				item = gameboardStrings[i][j];
				Cells[i][j].item=item;
			}
	};
	void GameBoard::CoordsToIndex(int &i, int &j, const Vertex &V) {
		FLOAT invScale=1.0/Scale;
		i=(Index)(V.z*invScale);
		j=(Index)(V.x*invScale);
	};
	void GameBoard::IndexToCoords(int i, int j, Vertex &V1, Vertex& V2) {
		V1.x=j*Scale;
		V1.z=i*Scale;
		V2.x=V1.x+Scale;
		V2.z=V1.z+Scale;
		V1.y=V2.y=0;
	};
	unsigned char GameBoard::PlaneBits(Vertex A, V3& P1, V3& P2) {
		unsigned char pBits=0;
		if (A.x<P1.x) pBits|=1;
		if (A.x>P2.x) pBits|=2;
		if (A.z<P1.z) pBits|=4;
		if (A.z>P2.z) pBits|=8;
		return pBits;
	};
	bool GameBoard::IsTriangleOverCell(int i, int j, TriangleGeometry &G) {
		V3 R1, R2;
		IndexToCoords(i,j,R1,R2);
		// clip away "easy" triangles
		return !(PlaneBits(G.V[0],R1,R2) & 
			 PlaneBits(G.V[1],R1,R2) & 
			 PlaneBits(G.V[2],R1,R2));
		/*
		  V3 R3, R4;
		  R3=R1; R1.x+=Scale;
		  R4=R1; R1.y+=Scale;
		  return G.BariCoords(u,v,R1) ||
		  G.BariCoords(u,v,R2) ||
		  G.BariCoords(u,v,R3) ||
		  G.BariCoords(u,v,R4);
		*/
	};
	void GameBoard::BuildCellTrianglesList() {
		TriangleGeometry G;
		Index nTriangles=Ground->getTriangleCount();
		Index i,j,k,l,i0,j0,i1,j1;
		for (k=0; k<nTriangles; k++) {
			Ground->getTriangleGeometry(k,G);
			i0=Cells.getRows();
			j0=Cells.getCols();
			i1=0; 
			j1=0; 
			for (l=0; l<2; l++) {
				CoordsToIndex(i,j,G.V[l]);
				i0=min(i-1,i0);
				j0=min(j-1,j0);
				i1=max(i+1,i1);
				j1=max(j+1,j1);
				log << LLInfo << i0 << "\t" << j0 << "\t" << i1 << "\t" << j1 << "\t" << G.V[l];
			}
			i1=min(i1,Cells.getRows());
			j1=min(j1,Cells.getCols());
			i0=max(0,i0);
			j0=max(0,j0);
			for (i=i0; i<i1;i++) for (j=j0; j<j1; j++)
				if (IsTriangleOverCell(i,j,G)) {
					Cells[i][j].AddTriangle(k);
				}
		}
		for (i=0; i<Cells.getRows();i++) for (j=0; j<Cells.getCols(); j++) {
			BoardCell &bc=Cells[i][j];
			log << LLInfo << "Cell "<<i<<","<<j<<" contains:";
			for (IndexArray::iterator iTriangle=bc.triangles.begin(); iTriangle!=bc.triangles.end(); iTriangle++)
				log << LLInfo << "\t" << *iTriangle;
			log <<"\n";
		}
		
	};
	// Spostare in model.h o vecmath.h
	bool GameBoard::Y(Vertex &Point, Normal &N) {
		FLOAT u,v,y;
		TriangleGeometry G;
		int i,j;
		CoordsToIndex(i,j,Point);
		BoardCell &bc=Cells[i][j];
		for (IndexArray::iterator iTriangle=bc.triangles.begin(); iTriangle!=bc.triangles.end(); iTriangle++) {
			Ground->getTriangleGeometry(*iTriangle,G);
			if (G.BariCoordsXZ(u,v,Point,&y)) {
				Point.y=y;
//				log << LLInfo << "Point is over triangle " << *iTriangle;
				G.normal(N);
				return true;
			}
		}
		log << LLError << "Point is NOT over a triangle\n";
		return false;
	};
};
