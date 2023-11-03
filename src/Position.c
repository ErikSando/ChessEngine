#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Definitions.h"

void ResetBoard(Position* position) {
	memset(position->bitboards, 0ULL, sizeof(position->bitboards));
	memset(position->occupancy, 0ULL, sizeof(position->occupancy));
	memset(position->bigPieces, 0, sizeof(position->bigPieces));
	memset(position->history, 0, sizeof(position->history));

	position->positionKey = 0ULL;
	position->side = Both;
	position->enPassant = NoSquare;
	position->castling = 0;
	position->fiftyMoveRule = 0;
	position->ply = 0;
	position->historyPly = 0;
}

U64 GeneratePositionKey(Position* position) {
	U64 key = 0ULL;

	for (int piece = wP; piece <= bK; piece++) {
		U64 bitboard = position->bitboards[piece];

		while (bitboard) {
			int square = GLS1BI(bitboard);
			ClearBit(bitboard, square);

			key ^= PieceKeys[piece][square];
		}
	}

	if (position->enPassant != NoSquare) key ^= EnPassantKeys[position->enPassant];
	if (position->side == White) key ^= SideKey;

	key ^= CastlingKeys[position->castling];

	return key;
}

void ParseFEN(char* fen, Position* position) {
	ResetBoard(position);

	int rank = Rank8;
	int file = FileA;
	int piece = -1;
	int count = 0;

	while ((rank >= Rank1) && *fen) {
		count = 1;

		switch (*fen) {
		case 'p':
		case 'r':
		case 'n':
		case 'b':
		case 'q':
		case 'k':
		case 'P':
		case 'R':
		case 'N':
		case 'B':
		case 'Q':
		case 'K':
			piece = CharPiece[*fen];
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			piece = -1;
			count = *fen - '0';
			break;

		case '/':
		case ' ':
			rank--;
			file = FileA;
			fen++;
			continue;

		default:
			printf("Invalid FEN\n");
			ResetBoard(position);
			return;
		}

		for (int i = 0; i < count; i++) {
			if (piece != -1) {
				int square = GetSquare(file, rank);
				SetBit(position->bitboards[piece], square);

				int colour = PieceColour[piece];
				SetBit(position->occupancy[colour], square);
				SetBit(position->occupancy[Both], square);

				if (IsBigPiece(piece)) position->bigPieces[colour]++;
			}

			file++;
		}

		fen++;
	}

	position->side = *fen == 'w' ? White : Black;
	fen += 2;

	for (int i = 0; i < 4; i++) {
		if (*fen == ' ') break;

		switch (*fen) {
			case 'K': position->castling |= WKC; break;
			case 'Q': position->castling |= WQC; break;
			case 'k': position->castling |= BKC; break;
			case 'q': position->castling |= BQC; break;
			default: break;
		}

		fen++;
	}

	fen++;

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		position->enPassant = GetSquare(file, rank);

		fen++;
	}

	position->fiftyMoveRule = atoi(fen + 2);

	position->positionKey = GeneratePositionKey(position);
}

int SquareAttacked(int square, int side, const Position* position) {
	if ((side == White) && (PawnCaptures[Black][square] & position->bitboards[wP])) return True;
	else if ((side == Black) && (PawnCaptures[White][square] & position->bitboards[bP])) return True;

	if ((GetBishopAttacks(square, position->occupancy[Both])
			& (side == White ? position->bitboards[wB] | position->bitboards[wQ] : position->bitboards[bB] | position->bitboards[bQ])) ||
		(GetRookAttacks(square, position->occupancy[Both])
			& (side == White ? position->bitboards[wR] | position->bitboards[wQ] : position->bitboards[bR] | position->bitboards[bQ])) ||
		(KnightAttacks[square]
			& (side == White ? position->bitboards[wN] : position->bitboards[bN])) ||
		(KingAttacks[square]
			& (side == White ? position->bitboards[wK] : position->bitboards[bK]))
	) {

		return True;
	}

	return False;
}