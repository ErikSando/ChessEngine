#include <stdio.h>
#include <string.h>
#include "Definitions.h"
#include "Attacks.h"

U64 GeneratePawnMoves(int square, int side) {
	U64 moves = 0ULL;
	U64 bitboard = 0ULL;

	SetBit(bitboard, square);

	if (side == White) {
		moves |=
			(bitboard << 8) |
			((SquareRanks[square] == Rank2) ? bitboard << 16 : 0ULL);
	}
	else {
		moves |=
			(bitboard >> 8) |
			((SquareRanks[square] == Rank7) ? bitboard >> 16 : 0ULL);
	}

	return moves;
}

U64 GeneratePawnCaptures(int square, int side) {
	U64 captures = 0ULL;
	U64 bitboard = 0ULL;

	SetBit(bitboard, square);

	if (side == White) {
		captures |=
			((bitboard << 9) & NotFileA) |
			((bitboard << 7) & NotFileH);
	}
	else {
		captures |=
			((bitboard >> 9) & NotFileH) |
			((bitboard >> 7) & NotFileA);
	}

	return captures;
}

U64 GenerateKnightAttacks(int square) {
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;

	SetBit(bitboard, square);

	attacks |=
		((bitboard << 17) & NotFileA)  |
		((bitboard << 15) & NotFileH)  |
		((bitboard << 10) & NotFileAB) |
		((bitboard <<  6) & NotFileHG) |
		((bitboard >> 17) & NotFileH)  |
		((bitboard >> 15) & NotFileA)  |
		((bitboard >> 10) & NotFileHG) |
		((bitboard >>  6) & NotFileAB);

	return attacks;
}

U64 GenerateKingAttacks(int square) {
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;

	SetBit(bitboard, square);

	attacks |=
		((bitboard << 1) & NotFileA) |
		((bitboard << 9) & NotFileA) |
		((bitboard << 7) & NotFileH) |
		((bitboard << 8))            |
		((bitboard >> 1) & NotFileH) |
		((bitboard >> 9) & NotFileH) |
		((bitboard >> 7) & NotFileA) |
		((bitboard >> 8));

	return attacks;
}

U64 GenerateUnblockedBishopAttacks(int square) {
	U64 attacks = 0ULL;

	int file = GetFile(square);
	int rank = GetRank(square);

	for (int targetFile = file - 1, targetRank = rank + 1; targetFile > FileA && targetRank < Rank8; targetFile--, targetRank++) {
		attacks |= (1ULL << GetSquare(targetFile, targetRank));
	}

	for (int targetFile = file + 1, targetRank = rank + 1; targetFile < FileH && targetRank < Rank8; targetFile++, targetRank++) {
		attacks |= (1ULL << GetSquare(targetFile, targetRank));
	}

	for (int targetFile = file + 1, targetRank = rank - 1; targetFile < FileH && targetRank > Rank1; targetFile++, targetRank--) {
		attacks |= (1ULL << GetSquare(targetFile, targetRank));
	}

	for (int targetFile = file - 1, targetRank = rank - 1; targetFile > FileA && targetRank > Rank1; targetFile--, targetRank--) {
		attacks |= (1ULL << GetSquare(targetFile, targetRank));
	}

	return attacks;
}

U64 GenerateUnblockedRookAttacks(int square) {
	U64 attacks = 0ULL;

	int file = GetFile(square);
	int rank = GetRank(square);

	for (int targetFile = file - 1; targetFile > FileA; targetFile--) {
		attacks |= (1ULL << GetSquare(targetFile, rank));
	}

	for (int targetRank = rank + 1; targetRank < Rank8; targetRank++) {
		attacks |= (1ULL << GetSquare(file, targetRank));
	}

	for (int targetFile = file + 1; targetFile < FileH; targetFile++) {
		attacks |= (1ULL << GetSquare(targetFile, rank));
	}

	for (int targetRank = rank - 1; targetRank > Rank1; targetRank--) {
		attacks |= (1ULL << GetSquare(file, targetRank));
	}

	return attacks;
}

U64 GenerateBishopAttacks(int square, U64 occupancy) {
	U64 attacks = 0ULL;

	int file = GetFile(square);
	int rank = GetRank(square);

	for (int targetFile = file - 1, targetRank = rank + 1; targetFile >= FileA && targetRank <= Rank8; targetFile--, targetRank++) {
		int square = GetSquare(targetFile, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetFile = file + 1, targetRank = rank + 1; targetFile <= FileH && targetRank <= Rank8; targetFile++, targetRank++) {
		int square = GetSquare(targetFile, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetFile = file + 1, targetRank = rank - 1; targetFile <= FileH && targetRank >= Rank1; targetFile++, targetRank--) {
		int square = GetSquare(targetFile, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetFile = file - 1, targetRank = rank - 1; targetFile >= FileA && targetRank >= Rank1; targetFile--, targetRank--) {
		int square = GetSquare(targetFile, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	return attacks;
}

U64 GenerateRookAttacks(int square, U64 occupancy) {
	U64 attacks = 0ULL;

	int file = GetFile(square);
	int rank = GetRank(square);

	for (int targetFile = file - 1; targetFile >= FileA; targetFile--) {
		int square = GetSquare(targetFile, rank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetRank = rank + 1; targetRank <= Rank8; targetRank++) {
		int square = GetSquare(file, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetFile = file + 1; targetFile <= FileH; targetFile++) {
		int square = GetSquare(targetFile, rank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	for (int targetRank = rank - 1; targetRank >= Rank1; targetRank--) {
		int square = GetSquare(file, targetRank);
		attacks |= (1ULL << square);
		if (GetBit(occupancy, square)) break;
	}

	return attacks;
}

U64 SetOccupancy(int index, int numBits, U64 attackMask) {
	U64 occupancy = 0ULL;

	for (int i = 0; i < numBits; i++) {
		int square = GLS1BI(attackMask);
		ClearBit(attackMask, square);

		if (index & (1 << i)) SetBit(occupancy, square);
	}

	return occupancy;
}

void InitNonSliderAttackTables() {
	for (int square = 0; square < 64; square++) {
		PawnMoves[White][square] = GeneratePawnMoves(square, White);
		PawnMoves[Black][square] = GeneratePawnMoves(square, Black);
		PawnCaptures[White][square] = GeneratePawnCaptures(square, White);
		PawnCaptures[Black][square] = GeneratePawnCaptures(square, Black);
		KnightAttacks[square] = GenerateKnightAttacks(square);
		KingAttacks[square] = GenerateKingAttacks(square);
	}
}

void InitSliderAttackTables() {
	for (int square = 0; square < 64; square++) {
		U64 attackMask = BishopAttackMasks[square] = GenerateUnblockedBishopAttacks(square);

		int relevantBits = BishopRelevantBits[square];
		int occupancyIndices = 1 << relevantBits;

		for (int i = 0; i < occupancyIndices; i++) {
			U64 occupancy = SetOccupancy(i, relevantBits, attackMask);
			int magicIndex = (occupancy * BishopMagics[square]) >> (64 - relevantBits);
			BishopAttacks[square][magicIndex] = GenerateBishopAttacks(square, occupancy);
		}
	}

	for (int square = 0; square < 64; square++) {
		U64 attackMask = RookAttackMasks[square] = GenerateUnblockedRookAttacks(square);

		int relevantBits = RookRelevantBits[square];
		int occupancyIndices = 1 << relevantBits;

		for (int i = 0; i < occupancyIndices; i++) {
			U64 occupancy = SetOccupancy(i, relevantBits, attackMask);
			int magicIndex = (occupancy * RookMagics[square]) >> (64 - relevantBits);
			RookAttacks[square][magicIndex] = GenerateRookAttacks(square, occupancy);
		}
	}
}

void InitAttackTables() {
	InitNonSliderAttackTables();
	InitSliderAttackTables();
}

U64 GetBishopAttacks(int square, U64 occupancy) {
	occupancy &= BishopAttackMasks[square];
	occupancy *= BishopMagics[square];
	occupancy >>= 64 - BishopRelevantBits[square];

	return BishopAttacks[square][occupancy];
}

U64 GetRookAttacks(int square, U64 occupancy) {
	occupancy &= RookAttackMasks[square];
	occupancy *= RookMagics[square];
	occupancy >>= 64 - RookRelevantBits[square];

	return RookAttacks[square][occupancy];
}

U64 GetQueenAttacks(int square, U64 occupancy) {
	U64 bishopOccupancy = occupancy;
	U64 rookOccupancy = occupancy;

	bishopOccupancy &= BishopAttackMasks[square];
	bishopOccupancy *= BishopMagics[square];
	bishopOccupancy >>= 64 - BishopRelevantBits[square];

	rookOccupancy &= RookAttackMasks[square];
	rookOccupancy *= RookMagics[square];
	rookOccupancy >>= 64 - RookRelevantBits[square];

	return BishopAttacks[square][bishopOccupancy] | RookAttacks[square][rookOccupancy];
}