#include <stdlib.h>
#include "Definitions.h"

#define MirrorSquare(square) ((square) ^ 56)

U64 FileMasks[8];
U64 RankMasks[8];
U64 WhitePassedMask[64];
U64 BlackPassedMask[64];
U64 IsolatedMask[8];
U64 StackedMask[64];

const int IsolatedPawnPenalty = 10;
const int StackedPawnPenalty = 5;

const int RookOpenFileBonus = 12;
const int RookSemiOpenFileBonus = 8;

const int QueenOpenFileBonus = 8;
const int QueenSemiOpenFileBonus = 5;

const int BishopPairBonus = 30;

const int KingCenterPawnBonus = 25;
const int KingSidePawnBonus = 20;

const int WeakKingNoCastlingPenalty = 30;

const int PassedPawnValue[8] = { 0, 20, 30, 40, 55, 75, 100, 0 };

const int PieceMgValue[6] = { 100, 320, 330, 500, 950, 0 };
const int PieceEgValue[6] = { 110, 290, 330, 550, 900, 0 };

const int MobilityMgValue[12] = { 0, 3, 3, 1, 2, 0, 0, 3, 3, 1, 2, 0 };
const int MobilityEgValue[12] = { 0, 1, 2, 3, 2, 0, 0, 1, 2, 3, 2, 0 };

// Piece tables taken from https://www.chessprogramming.org/Simplified_Evaluation_Function
// (with some small changes)
// King tables made by me

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
	-40, -30, -20, -20, -20, -20, -30, -40,
	-30, -20,   0,   0,   0,   0, -20, -30,
	-20,   0,  10,  15,  15,  10,   0, -20,
	-20,   5,  15,  20,  20,  15,   5, -20,
	-20,   0,  15,  20,  20,  15,   0, -20,
	-20,   5,  10,  15,  15,  10,   5, -20,
	-30, -20,   0,   5,   5,   0, -20, -30,
	-40, -30, -20, -20, -20, -20, -35, -40
};

const int BishopTable[64] = {
	-20, -10, -10, -10, -10, -10, -10, -20,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   5,   5,  10,  10,   5,   5, -10,
	-10,   0,  10,  10,  10,  10,   0, -10,
	-10,  10,  10,  10,  10,  10,  10, -10,
	-10,   5,   0,   0,   0,   0,   5, -10,
	-20, -10, -10, -10, -10, -10, -10, -20
};

const int RookTable[64] = {
	  0,   0,   0,   5,   5,   0,   0,   0,
	  5,  15,  15,  15,  15,  15,  15,   5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	 -5,   0,   0,   5,   5,   0,   0,  -5,
	  0,   0,   5,  10,  10,   5,   0,  0
};

const int QueenTable[64] = {
	-20, -10, -10,  -5,  -5, -10, -10, -20,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	 -5,   0,   5,   5,   5,   5,   0,  -5,
	  0,   0,   5,   5,   5,   5,   0,  -5,
	-10,   5,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,   0,   0,   0,   0, -10,
	-20, -10, -10,  -5,  -5, -10, -10, -20
};

const int KingMgTable[64] = {
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -60, -60, -60, -60, -60,
	-60, -60, -60, -40, -40, -40, -60, -60,
	 20,  40,  30, -30,   0, -30,  40,  20,
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

void InitBitMasks() {
	for (int i = 0; i < 8; i++) {
		FileMasks[i] = 0ULL;
		RankMasks[i] = 0ULL;
		IsolatedMask[i] = 0ULL;
	}

	for (int rank = Rank8; rank >= Rank1; rank--) {
		for (int file = FileA; file <= FileH; file++) {
			int square = GetSquare(file, rank);

			SetBit(FileMasks[file], square);
			SetBit(RankMasks[rank], square);

			StackedMask[square] = 0ULL;
		}
	}

	for (int square = 0; square < 64; square++) {
		WhitePassedMask[square] = 0ULL;
		BlackPassedMask[square] = 0ULL;

		int rank = GetRank(square);
		int file = GetFile(square);

		while (rank <= Rank7) {
			rank++;
			WhitePassedMask[square] |= RankMasks[rank];
		}

		rank = GetRank(square);

		while (rank >= Rank2) {
			rank--;
			BlackPassedMask[square] |= RankMasks[rank];
		}

		StackedMask[square] |= FileMasks[file];
		ClearBit(StackedMask[square], square);

		U64 WhiteFilesMask = FileMasks[file];
		U64 BlackFilesMask = FileMasks[file];

		if (file > FileA) {
			WhiteFilesMask |= FileMasks[file - 1];
			BlackFilesMask |= FileMasks[file - 1];

			IsolatedMask[file] |= FileMasks[file - 1];
		}

		if (file < FileH) {
			WhiteFilesMask |= FileMasks[file + 1];
			BlackFilesMask |= FileMasks[file + 1];

			IsolatedMask[file] |= FileMasks[file + 1];
		}

		WhitePassedMask[square] &= WhiteFilesMask;
		BlackPassedMask[square] &= BlackFilesMask;
	}
}

void InitPieceSquareTables() {
	for (int piece = wP; piece <= wK; piece++) {
		for (int square = 0; square < 64; square++) {
			MgTables[piece][square] = PieceMgValue[piece] + PieceMgTables[piece][MirrorSquare(square)];
			EgTables[piece][square] = PieceEgValue[piece] + PieceEgTables[piece][MirrorSquare(square)];
		}
	}

	for (int piece = bP; piece <= bK; piece++) {
		for (int square = 0; square < 64; square++) {
			MgTables[piece][square] = PieceMgValue[piece - 6] + PieceMgTables[piece - 6][square];
			EgTables[piece][square] = PieceEgValue[piece - 6] + PieceEgTables[piece - 6][square];
		}
	}
}

static inline int MaterialDraw(const Position* position) {
	int pawns[2] = { CountBits(position->bitboards[wP]), CountBits(position->bitboards[bP]) };

	if (pawns[White] || pawns[Black]) return False;

	int knights[2] = { CountBits(position->bitboards[wN]), CountBits(position->bitboards[bN]) };
	int bishops[2] = { CountBits(position->bitboards[wB]), CountBits(position->bitboards[bB]) };
	int rooks[2] = { CountBits(position->bitboards[wR]), CountBits(position->bitboards[bR]) };
	int queens[2] = { CountBits(position->bitboards[wQ]), CountBits(position->bitboards[bQ]) };

	if (!rooks[White] && !rooks[Black] && !queens[White] && !queens[Black]) {
		if (!bishops[White] && !bishops[Black]) {
			if (knights[White] < 3 && knights[Black] < 3) return True;
		}
		else if (!knights[White] && !knights[Black]) {
			if (abs(bishops[White] - bishops[Black]) < 2) return True;
		}
		else if ((knights[White] < 3 && !bishops[White]) || (bishops[Black] == 1 && !knights[Black])) {
			if ((knights[Black] < 3 && !bishops[Black]) || (bishops[Black] == 1 && !knights[Black])) return True;
		}
	}
	else if (!queens[White] && !queens[Black]) {
		if (rooks[White] == 1 && rooks[Black] == 1) {
			if ((knights[White] + bishops[White] < 2) && (knights[Black] + bishops[Black] < 2)) return True;
		}
		else if (rooks[White] == 1 && !rooks[Black]) {
			if ((knights[White] + bishops[White] == 0) && ((knights[Black] + bishops[Black] == 1) || (knights[Black] + bishops[Black] == 2))) return True;
		}
		else if (rooks[Black] == 1 && !rooks[White]) {
			if ((knights[Black] + bishops[Black] == 0) && ((knights[White] + bishops[White] == 1) || (knights[White] + bishops[White] == 2))) return True;
		}
	}

	return False;
}

static int ManhattanDistance(int square1, int square2) {
	int file1 = GetFile(square1);
	int file2 = GetFile(square2);
	int rank1 = GetRank(square1);
	int rank2 = GetRank(square2);

	int fileDistance = abs(file2 - file1);
	int rankDistance = abs(rank2 - rank1);

	return fileDistance + rankDistance;
}

int Evaluate(const Position* position) {
	if (MaterialDraw(position)) return 0;

	int mgScore = 0;
	int egScore = 0;
	int score = 0;

	int gamePhase = 0;

	int piece = wP;
	U64 bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((WhitePassedMask[square] & position->bitboards[bP]) == 0) {
			score += PassedPawnValue[GetRank(square)];
		}

		if ((IsolatedMask[GetFile(square)] & position->bitboards[wP]) == 0) {
			score -= IsolatedPawnPenalty;
		}

		if (StackedMask[square] & position->bitboards[wP]) {
			score -= StackedPawnPenalty;
		}
	}

	piece = bP;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((BlackPassedMask[square] & position->bitboards[wP]) == 0) {
			score -= PassedPawnValue[7 - GetRank(square)];
		}

		if ((IsolatedMask[GetFile(square)] & position->bitboards[bP]) == 0) {
			score += IsolatedPawnPenalty;
		}

		if (StackedMask[square] & position->bitboards[bP]) {
			score += StackedPawnPenalty;
		}
	}

	piece = wN;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];
	}

	piece = bN;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];
	}

	piece = wB;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];
	}

	piece = bB;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];
	}

	piece = wR;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((StackedMask[square] & (position->bitboards[wP] | position->bitboards[bP])) == 0) {
			score += RookOpenFileBonus;
		}
		else if ((StackedMask[square] & position->bitboards[wP]) == 0) {
			score += RookSemiOpenFileBonus;
		}
	}

	piece = bR;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((StackedMask[square] & (position->bitboards[bP] | position->bitboards[wP])) == 0) {
			score -= RookOpenFileBonus;
		}
		else if ((StackedMask[square] & position->bitboards[bP]) == 0) {
			score -= RookSemiOpenFileBonus;
		}
	}

	piece = wQ;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((StackedMask[square] & (position->bitboards[wP] | position->bitboards[bP])) == 0) {
			score += QueenOpenFileBonus;
		}
		else if ((StackedMask[square] & position->bitboards[wP]) == 0) {
			score += QueenSemiOpenFileBonus;
		}
	}

	piece = bQ;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		gamePhase += GamePhaseIncrement[piece];

		if ((StackedMask[square] & (position->bitboards[bP] | position->bitboards[wP])) == 0) {
			score -= QueenOpenFileBonus;
		}
		else if ((StackedMask[square] & position->bitboards[bP]) == 0) {
			score -= QueenSemiOpenFileBonus;
		}
	}

	piece = wK;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		if (mgScore <= 0) {
			if ((position->castling & WKC) == 0) {
				mgScore -= WeakKingNoCastlingPenalty;
			}

			if ((position->castling & WQC) == 0) {
				mgScore -= WeakKingNoCastlingPenalty;
			}
		}
	}

	piece = bK;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		if (mgScore <= 0) {
			if ((position->castling & BKC) == 0) {
				mgScore += WeakKingNoCastlingPenalty;
			}

			if ((position->castling & BQC) == 0) {
				mgScore += WeakKingNoCastlingPenalty;
			}
		}
	}

	if (CountBits(position->bitboards[wP]) == 0 && CountBits(position->bitboards[bP]) == 0) {
		int wKingSquare = GLS1BI(position->bitboards[wK]);
		int bKingSquare = GLS1BI(position->bitboards[bK]);

		if (egScore > 0) {
			score += 4.7 * CenterManhattanDistance[bKingSquare];
			score += 1.6 * (14 - ManhattanDistance(wKingSquare, bKingSquare));
		}
		else if (egScore < 0) {
			score -= 4.7 * CenterManhattanDistance[wKingSquare];
			score -= 1.6 * (14 - ManhattanDistance(wKingSquare, bKingSquare));
		}
	}

	if (CountBits(position->bitboards[wB]) > 1) score += BishopPairBonus;
	if (CountBits(position->bitboards[bB]) > 1) score -= BishopPairBonus;

	int mgPhase = gamePhase <= 24 ? gamePhase : 24;
	int egPhase = 24 - mgPhase;

	int evaluation = (mgScore * mgPhase + egScore * egPhase) / 24 + score;

	return position->side == White ? evaluation : -evaluation;
}