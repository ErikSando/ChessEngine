#include <stdio.h>
#include <stdlib.h>
#include "Definitions.h"
#include "Evaluate.h"

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