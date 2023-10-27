#include "Definitions.h"

int SquareFiles[64];
int SquareRanks[64];

U64 PieceKeys[12][64];
U64 EnPassantKeys[64];
U64 CastlingKeys[16];
U64 SideKey;

unsigned int state = 1804289383;

unsigned int Rand32() {
	unsigned int number = state;

	number ^= number << 13;
	number ^= number >> 17;
	number ^= number << 5;

	state = number;

	return number;
}

U64 Rand64() {
	U64 n1 = (U64)(Rand32()) & 0xFFFF;
	U64 n2 = (U64)(Rand32()) & 0xFFFF;
	U64 n3 = (U64)(Rand32()) & 0xFFFF;
	U64 n4 = (U64)(Rand32()) & 0xFFFF;

	return (n1) | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

void InitFileRankLookup() {
	for (int square = 0; square < 64; square++) {
		int file = square % 8;
		int rank = (square - file) / 8;

		SquareFiles[square] = file;
		SquareRanks[square] = rank;
	}
}

void InitHashKeys() {
	for (int square = 0; square < 64; square++) {
		for (int piece = wP; piece <= bK; piece++) {
			PieceKeys[piece][square] = Rand64();
		}
		
		EnPassantKeys[square] = Rand64();
	}

	for (int i = 0; i < 16; i++) {
		CastlingKeys[i] = Rand64();
	}

	SideKey = Rand64();
}

void Init() {
	InitFileRankLookup();
	InitHashKeys();
	InitAttackTables();
	InitBitMasks();
	InitPieceSquareTables();
}