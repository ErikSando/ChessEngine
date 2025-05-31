#ifndef Evaluate_h
#define Evaluate_h

#include "Definitions.h"

#define MirrorSquare(square) ((square) ^ 56)

U64 FileMasks[8];
U64 RankMasks[8];
U64 WhitePassedMask[64];
U64 BlackPassedMask[64];
U64 IsolatedMask[8];
U64 StackedMask[64];
U64 WhiteClosePawnShield[64]; // pawn shield in front of king
U64 WhiteFarPawnShield[64]; // pawn shield two squares in front of king
U64 BlackClosePawnShield[64];
U64 BlackFarPawnShield[64];
U64 CenterMask = 0x0000001818000000ULL;
U64 ExtendedCenterMask = 0x00003C24243C0000ULL;

const int IsolatedPawnPenalty = 10;
const int StackedPawnPenalty = 5;
const int DefendedPawnBonus = 8;

const int RookOpenFileBonus = 12;
const int RookSemiOpenFileBonus = 8;

const int QueenOpenFileBonus = 8;
const int QueenSemiOpenFileBonus = 5;

const int BishopPairBonus = 30;

const int KingVirtualAttackWeight = 2;
const int ClosePawnShieldBonus = 10;
const int FarPawnShieldBonus = 3;
const int KingOpenFilePenalty = 15;

// setting these to zero drops the nodes searched significantly
const int CenterControlBonus = 5; // low because it will be added to both middle game score and full game score to make it less valuable in the endgame
const int ExtendedCenterControlBonus = 3;

const int PassedPawnValue[8] = { 0, 20, 30, 40, 55, 75, 100, 0 };

const int PieceMgValue[6] = { 100, 320, 330, 500, 950, 0 };
const int PieceEgValue[6] = { 110, 290, 330, 550, 900, 0 };

const int MobilityMgValue[12] = { 0, 4, 3, 1, 1, 0, 0, 4, 3, 1, 1, 0 };
const int MobilityEgValue[12] = { 0, 2, 3, 3, 1, 0, 0, 2, 3, 3, 1, 0 };

// Piece square tables based on https://www.chessprogramming.org/Simplified_Evaluation_Function
// (with some small changes)
// King tables made by me
// Values have been roughly halved for each square to compensate for mobility evaluation (excepet pawns and king)

const int PawnMgTable[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 50,  50,  50,  50,  50,  50,  50,  50,
	 10,  10,  20,  30,  30,  20,  10,  10,
	  5,   5,  10,  20,  20,  10,   5,   5,
	  0,   0,   0,  25,  25,   0,   0,   0,
	  5,  -5, -10,   0,   0, -10,  -5,   5,
	  5,  10,  10, -25, -30,  10,  10,   5,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int PawnEgTable[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 50,  50,  50,  50,  50,  50,  50,  50,
	 35,  35,  35,  35,  35,  35,  35,  35,
	 20,  20,  20,  20,  20,  20,  20,  20,
	  5,   5,   5,   5,   5,   5,   5,   5,
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10, -10, -10, -10, -10, -10, -10, -10,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int KnightTable[64] = {
	-20, -15, -10, -10, -10, -10, -15, -20,
	-15, -10,   0,   0,   0,   0, -10, -15,
	-10,   0,   3,   8,   8,   3,   0, -10,
	-10,   3,   8,  10,  10,   8,   3, -10,
	-10,   0,   8,  10,  10,   8,   0, -10,
	-10,   3,   5,   8,   8,   5,   3, -10,
	-15, -10,   0,   3,   3,   0, -10, -15,
	-20, -15, -10, -10, -10, -10, -18, -20
};

const int BishopTable[64] = {
	-10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,   0,   3,   5,   5,   3,   0,  -5,
	 -5,   3,   3,   5,   5,   3,   3,  -5,
	 -5,   0,   5,   5,   5,   5,   0,  -5,
	 -5,   5,   5,   5,   5,   5,   5,  -5,
	 -5,   3,   0,   0,   0,   0,   3,  -5,
	-10,  -5,  -5,  -5, - 5,  -5,  -5, -10
};

const int RookTable[64] = {
	  0,   0,   0,   5,   5,   0,   0,   0,
	  5,  10,  10,  10,  10,  10,  10,   5,
	 -4,   0,   0,   4,   4,   0,   0,  -4,
	 -4,   0,   0,   4,   4,   0,   0,  -4,
	 -4,   0,   0,   4,   4,   0,   0,  -4,
	 -4,   0,   0,   4,   4,   0,   0,  -4,
	 -4,   0,   0,   4,   4,   0,   0,  -4,
	  0,   0,   3,   6,   6,   4,   0,   0
};

const int QueenTable[64] = {
	 -5,  -5,  -5,  -3,  -3,  -5,  -5, -10,
	 -5,   0,   0,   0,   0,   0,   0,  -5,
	 -5,   0,   3,   3,   3,   3,   0,  -5,
	 -3,   0,   3,   3,   3,   3,   0,  -3,
	  0,   0,   3,   3,   3,   3,   0,  -3,
	 -5,   3,   3,   3,   3,   3,   0,  -5,
	 -5,   0,   3,   0,   0,   0,   0,  -5,
	-10,  -5,  -5,  -3,  -3,  -5,  -5, -10
};

const int KingMgTable[64] = {
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-40, -40, -40, -40, -40, -40, -40, -40,
	 20,  40,  30, -30,   0, -30,  40,  20
};

const int KingEgTable[64] = {
	-30, -20, -10,   0,   0, -10, -20, -30,
	-20, -10,   0,  10,  10,   0, -10, -20,
	-20,   0,  10,  20,  20,  10,   0, -20,
	-20,   0,  10,  20,  20,  10,   0, -20,
	-20,   0,  10,  20,  20,  10,   0, -20,
	-20, -10,   0,  10,  10,   0, -10, -20,
	-30, -20, -10,   0,   0, -10, -20, -30,
	-40, -30, -20, -10, -10, -20, -30, -40
};

const int CenterManhattanDistance[64] = {
	6, 5, 4, 3, 3, 4, 5, 6,
	5, 4, 3, 2, 2, 3, 4, 5,
	4, 3, 2, 1, 1, 2, 3, 4,
	3, 2, 1, 0, 0, 1, 2, 3,
	3, 2, 1, 0, 0, 1, 2, 3,
	4, 3, 2, 1, 1, 2, 3, 4,
	5, 4, 3, 2, 2, 3, 4, 5,
	6, 5, 4, 3, 3, 4, 5, 6
};

const int* PieceMgTables[6] = {
	PawnMgTable,
	KnightTable,
	BishopTable,
	RookTable,
	QueenTable,
	KingMgTable
};

const int* PieceEgTables[6] = {
	PawnEgTable,
	KnightTable,
	BishopTable,
	RookTable,
	QueenTable,
	KingEgTable
};

const int GamePhaseIncrement[12] = { 0, 1, 1, 2, 4, 0, 1, 1, 2, 4, 0 };

int MgTables[12][64];
int EgTables[12][64];

#endif