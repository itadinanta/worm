#ifndef __GAMEBOARD_H
#define __GAMEBOARD_H

#include "model.h"
#include "vecmath.h"
#include <vector>
#include <bitset>

namespace model {
	class BoardCell {
	public:
		char item;
		bitset<8> flags;
		IndexArray triangles;
		BoardCell();
		~BoardCell();
		void AddTriangle(Index triangle);
	};
	
	class CellMatrix: private vector< vector<BoardCell> > {
		int Rows, Cols;
	public:
		CellMatrix(int rows=31, int cols=31) {
			init(rows, cols);
		}
		void init(int rows, int cols);
		
		inline vector<BoardCell> & operator[](int i) {
			return vector< vector<BoardCell> >::operator[](i);
		};
		inline int getRows() {return Rows;}
		inline int getCols() {return Cols;}
		
	};

	class GameBoard {
		FLOAT Scale;
		Mesh *Ground;
		void Load();
		unsigned char PlaneBits(Vertex A, V3& P1, V3&P2);
		bool IsTriangleOverCell(int i, int j, TriangleGeometry &G);
		void BuildCellTrianglesList();
		bool BariCoords(FLOAT &u, FLOAT &v, const Vertex& Point, const Vertex&A, Vertex B, Vertex C);
	public:
		CellMatrix Cells;
		GameBoard(FLOAT scale=4.0);
		void Setup(Mesh *ground);
		void CoordsToIndex(int &i, int& j, const Vertex &V1);
		void IndexToCoords(int i, int j, Vertex &V1, Vertex& V2);
		bool Y(Vertex& Point, Normal &N);
	};
};


#endif
