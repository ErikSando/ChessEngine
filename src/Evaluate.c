#include <stdio.h>
#include <stdlib.h>
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
/*U64 WhitePawnStorm[8][64]; // indexed by distance (rank) and square
U64 BlackPawnStorm[8][64];*/
U64 CenterMask = 0x0000001818000000ULL;
U64 ExtendedCenterMask = 0x00003C24243C0000ULL;

const int IsolatedPawnPenalty = 10;
const int StackedPawnPenalty = 5;
const int DefendedPawnBonus = 8;

const int RookOpenFileBonus = 12;
const int RookSemiOpenFileBonus = 8;

const int QueenOpenFileBonus = 8;
const int QueenSemiOpenFileBonus = 5;

const int BishopPairBonus = 25;

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

void InitBitMasks() {
	U64 File_A = 0x0101010101010101ULL;
	U64 Rank_1 = 0x00000000000000FFULL;

	for (int i = 0; i < 8; i++) {
		FileMasks[i] = File_A << i;
		RankMasks[i] = Rank_1 << i * 8;
		IsolatedMask[i] = 0ULL;
	}

	for (int rank = Rank8; rank >= Rank1; rank--) {
		for (int file = FileA; file <= FileH; file++) {
			int square = GetSquare(file, rank);

			StackedMask[square] = FileMasks[file];
			ClearBit(StackedMask[square], square);

			WhitePassedMask[square] = 0ULL;
			BlackPassedMask[square] = 0ULL;

			int _rank = rank;

			while (_rank <= Rank7) {
				WhitePassedMask[square] |= RankMasks[++_rank];
			}

			_rank = rank;

			while (_rank >= Rank2) {
				BlackPassedMask[square] |= RankMasks[--_rank];
			}

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

			WhiteClosePawnShield[square] |= PawnCaptures[White][square];
			BlackClosePawnShield[square] |= PawnCaptures[Black][square];

			if (rank < Rank8) {
				SetBit(WhiteClosePawnShield[square], square + 8);

				if (rank < Rank7) {
					SetBit(WhiteFarPawnShield[square], square + 16);

					if (file > FileA) SetBit(WhiteFarPawnShield[square], square + 15);
					if (file < FileH) SetBit(WhiteFarPawnShield[square], square + 17);
				}
			}

			if (rank > Rank1) {
				SetBit(BlackClosePawnShield[square], square - 8);

				if (rank > Rank2) {
					SetBit(BlackFarPawnShield[square], square - 16);
				
					if (file > FileA) SetBit(BlackFarPawnShield[square], square - 17);
					if (file < FileH) SetBit(BlackFarPawnShield[square], square - 15);
				}
			}
		}
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

static inline int ManhattanDistance(int square1, int square2) {
	return abs(GetFile(square2) - GetFile(square1))
		 + abs(GetRank(square2) - GetRank(square1));
}

int Evaluate(const Position* position) {
	if (MaterialDraw(position)) return 0;

	int mgScore = 0;
	int egScore = 0;
	int score = 0;

	int gamePhase = 0;

	int pawns = 0;
	int bishops[2] = { 0, 0 };

	int piece = wP;
	U64 bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		pawns++;

		gamePhase += GamePhaseIncrement[piece];

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		//U64 captures = PawnCaptures[White][square];
		//int centerControl = CountBits(CenterMask & captures);
		//int extendedCenterControl = CountBits(ExtendedCenterMask & captures);

		//score += centerControl * CenterControlBonus;
		//score += extendedCenterControl * ExtendedCenterControlBonus;

		if ((WhitePassedMask[square] & position->bitboards[bP]) == 0) score += PassedPawnValue[GetRank(square)];
		if ((IsolatedMask[GetFile(square)] & position->bitboards[wP]) == 0) score -= IsolatedPawnPenalty;
		if (StackedMask[square] & position->bitboards[wP]) score -= StackedPawnPenalty;
		if (PawnCaptures[Black][square] & position->bitboards[wP]) score += DefendedPawnBonus;
	}

	piece = bP;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		pawns++;

		gamePhase += GamePhaseIncrement[piece];

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		//U64 captures = PawnCaptures[Black][square];
		//int centerControl = CountBits(CenterMask & captures);
		//int extendedCenterControl = CountBits(ExtendedCenterMask & captures);

		//score -= centerControl * CenterControlBonus;
		//score -= extendedCenterControl * ExtendedCenterControlBonus;

		if ((BlackPassedMask[square] & position->bitboards[wP]) == 0) score -= PassedPawnValue[7 - GetRank(square)];
		if ((IsolatedMask[GetFile(square)] & position->bitboards[bP]) == 0) score += IsolatedPawnPenalty;
		if (StackedMask[square] & position->bitboards[bP]) score += StackedPawnPenalty;
		if (PawnCaptures[White][square] & position->bitboards[bP]) score -= DefendedPawnBonus;
	}

	piece = wN;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		gamePhase += GamePhaseIncrement[piece];

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		U64 attacks = KnightAttacks[square] & ~position->occupancy[White];
		int mobility = CountBits(attacks);
		//int centerControl = CountBits(attacks & CenterMask);
		//int extendedCenterControl = CountBits(attacks & ExtendedCenterMask);

		mgScore += mobility * MobilityMgValue[piece];
		egScore += mobility * MobilityEgValue[piece];

		//mgScore += centerControl * CenterControlBonus;
		//mgScore += extendedCenterControl * ExtendedCenterControlBonus;
		//score += centerControl * CenterControlBonus;
		//score += extendedCenterControl * ExtendedCenterControlBonus;
	}

	piece = bN;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		gamePhase += GamePhaseIncrement[piece];
		
		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		U64 attacks = KnightAttacks[square] & ~position->occupancy[Black];
		int mobility = CountBits(attacks);
		//int centerControl = CountBits(attacks & CenterMask);
		//int extendedCenterControl = CountBits(attacks & ExtendedCenterMask);

		mgScore -= mobility * MobilityMgValue[piece];
		egScore -= mobility * MobilityEgValue[piece];

		//mgScore -= centerControl * CenterControlBonus;
		//mgScore -= extendedCenterControl * ExtendedCenterControlBonus;
		//score -= centerControl * CenterControlBonus;
		//score -= extendedCenterControl * ExtendedCenterControlBonus;
	}

	piece = wB;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		bishops[White]++;

		gamePhase += GamePhaseIncrement[piece];

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		U64 attacks = GetBishopAttacks(square, position->occupancy[Both]) & ~position->occupancy[White];
		int mobility = CountBits(attacks);
		//int centerControl = CountBits(attacks & CenterMask);
		//int extendedCenterControl = CountBits(attacks & ExtendedCenterMask);

		mgScore += mobility * MobilityMgValue[piece];
		egScore += mobility * MobilityEgValue[piece];

		//mgScore += centerControl * CenterControlBonus;
		//mgScore += extendedCenterControl * ExtendedCenterControlBonus;
		//score += centerControl * CenterControlBonus;
		//score += extendedCenterControl * ExtendedCenterControlBonus;
	}

	piece = bB;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		bishops[Black]++;

		gamePhase += GamePhaseIncrement[piece];

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		U64 attacks = GetBishopAttacks(square, position->occupancy[Both]) & ~position->occupancy[Black];
		int mobility = CountBits(attacks);
		//int centerControl = CountBits(attacks & CenterMask);
		//int extendedCenterControl = CountBits(attacks & ExtendedCenterMask);

		mgScore -= mobility * MobilityMgValue[piece];
		egScore -= mobility * MobilityEgValue[piece];

		//mgScore -= centerControl * CenterControlBonus;
		//mgScore -= extendedCenterControl * ExtendedCenterControlBonus;
		//score -= centerControl * CenterControlBonus;
		//score -= extendedCenterControl * ExtendedCenterControlBonus;
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

		U64 attacks = GetRookAttacks(square, position->occupancy[Both]) & ~position->occupancy[White];
		int mobility = CountBits(attacks);

		mgScore += mobility * MobilityMgValue[piece];
		egScore += mobility * MobilityEgValue[piece];
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

		U64 attacks = GetRookAttacks(square, position->occupancy[Both]) & ~position->occupancy[Black];
		int mobility = CountBits(attacks);

		mgScore -= mobility * MobilityMgValue[piece];
		egScore -= mobility * MobilityEgValue[piece];
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

		int mobility = CountBits(GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[White]);

		mgScore += mobility * MobilityMgValue[piece];
		egScore += mobility * MobilityEgValue[piece];
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

		int mobility = CountBits(GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[Black]);

		mgScore -= mobility * MobilityMgValue[piece];
		egScore -= mobility * MobilityEgValue[piece];
	}

	int wKingSquare;
	int bKingSquare;

	piece = wK;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		wKingSquare = square;

		mgScore += MgTables[piece][square];
		egScore += EgTables[piece][square];

		mgScore -= KingVirtualAttackWeight * CountBits(GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[Both]);
		mgScore += ClosePawnShieldBonus * CountBits(WhiteClosePawnShield[square] & position->bitboards[wP]);
		mgScore += FarPawnShieldBonus * CountBits(WhiteFarPawnShield[square] & position->bitboards[wP]);

		// I realise this doesnt exclude pawns below the king, but who cares
		if (!(StackedMask[square] & position->bitboards[wP])) mgScore -= KingOpenFilePenalty;
	}

	piece = bK;
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		bKingSquare = square;

		mgScore -= MgTables[piece][square];
		egScore -= EgTables[piece][square];

		mgScore += KingVirtualAttackWeight * CountBits(GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[Both]);
		mgScore -= ClosePawnShieldBonus * CountBits(BlackClosePawnShield[square] & position->bitboards[bP]);
		mgScore -= FarPawnShieldBonus * CountBits(BlackFarPawnShield[square] & position->bitboards[bP]);

		if (!(StackedMask[square] & position->bitboards[bP])) mgScore += KingOpenFilePenalty;
	}

	if (pawns == 0) { // mop up evaluation
		if (egScore > 0) {
			score += 4.7 * CenterManhattanDistance[bKingSquare];
			score += 1.6 * (14 - ManhattanDistance(wKingSquare, bKingSquare));	
		}
		else {
			score -= 4.7 * CenterManhattanDistance[wKingSquare];
			score -= 1.6 * (14 - ManhattanDistance(wKingSquare, bKingSquare));
		}
	}
	
	//if (gamePhase == 0) { // king and pawn endgame

	//}

	if (bishops[White] > 1) score += BishopPairBonus;
	if (bishops[Black] > 1) score -= BishopPairBonus;

	int mgPhase = gamePhase;
	if (mgPhase > 24) mgPhase = 24;
	int egPhase = 24 - mgPhase;

	int evaluation = (mgScore * mgPhase + egScore * egPhase) / 24 + score;

	return position->side == White ? evaluation : -evaluation;
}