#include <stdio.h>
#include <string.h>
#include "Definitions.h"

U64 NotFileA = 0xFEFEFEFEFEFEFEFEULL;
U64 NotFileAB = 0xFCFCFCFCFCFCFCFCULL;

U64 NotFileH = 0x7F7F7F7F7F7F7F7FULL;
U64 NotFileHG = 0x3F3F3F3F3F3F3F3FULL;

U64 PawnMoves[2][64]; // double pawn push moves need to be eliminated by the move gen if there is a piece one square in front
U64 PawnCaptures[2][64];
U64 KnightAttacks[64];
U64 KingAttacks[64];

U64 BishopAttackMasks[64];
U64 RookAttackMasks[64];

U64 BishopAttacks[64][512];
U64 RookAttacks[64][4096];

const int BishopRelevantBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

const int RookRelevantBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

const U64 BishopMagics[64] = {
	0x40040822862081ULL, 0x40810a4108000ULL, 0x2008008400920040ULL, 0x61050104000008ULL,
	0x8282021010016100ULL, 0x41008210400a0001ULL, 0x3004202104050c0ULL, 0x22010108410402ULL,
	0x60400862888605ULL, 0x6311401040228ULL, 0x80801082000ULL, 0x802a082080240100ULL,
	0x1860061210016800ULL, 0x401016010a810ULL, 0x1000060545201005ULL, 0x21000c2098280819ULL,
	0x2020004242020200ULL, 0x4102100490040101ULL, 0x114012208001500ULL, 0x108000682004460ULL,
	0x7809000490401000ULL, 0x420b001601052912ULL, 0x408c8206100300ULL, 0x2231001041180110ULL,
	0x8010102008a02100ULL, 0x204201004080084ULL, 0x410500058008811ULL, 0x480a040008010820ULL,
	0x2194082044002002ULL, 0x2008a20001004200ULL, 0x40908041041004ULL, 0x881002200540404ULL,
	0x4001082002082101ULL, 0x8110408880880ULL, 0x8000404040080200ULL, 0x200020082180080ULL,
	0x1184440400114100ULL, 0xc220008020110412ULL, 0x4088084040090100ULL, 0x8822104100121080ULL,
	0x100111884008200aULL, 0x2844040288820200ULL, 0x90901088003010ULL, 0x1000a218000400ULL,
	0x1102010420204ULL, 0x8414a3483000200ULL, 0x6410849901420400ULL, 0x201080200901040ULL,
	0x204880808050002ULL, 0x1001008201210000ULL, 0x16a6300a890040aULL, 0x8049000441108600ULL,
	0x2212002060410044ULL, 0x100086308020020ULL, 0x484241408020421ULL, 0x105084028429c085ULL,
	0x4282480801080cULL, 0x81c098488088240ULL, 0x1400000090480820ULL, 0x4444000030208810ULL,
	0x1020142010820200ULL, 0x2234802004018200ULL, 0xc2040450820a00ULL, 0x2101021090020ULL,
};

const U64 RookMagics[64] = {
	0xa080041440042080ULL, 0xa840200410004001ULL, 0xc800c1000200081ULL, 0x100081001000420ULL,
	0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
	0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
	0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
	0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
	0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
	0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
	0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
	0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
	0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
	0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
	0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
	0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
	0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
	0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
	0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL,
};

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

U64 SetOccupancy(int index, int numBits, U64 moveMask) {
	U64 occupancy = 0ULL;

	for (int i = 0; i < numBits; i++) {
		int square = GLS1BI(moveMask);
		ClearBit(moveMask, square);

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