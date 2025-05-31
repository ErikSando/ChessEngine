#include <stdio.h>
#include "Definitions.h"

char* SquareString(int square) {
	static char string[3];

	int file = GetFile(square);
	int rank = GetRank(square);

	sprintf(string, "%c%c", ('a' + file), ('1' + rank));

	return string;
}

char* MoveString(int move) {
	static char string[5];
	static char pString[6]; // in case of promotion

	int from = FromSquare(move);
	int to = ToSquare(move);

	int fromFile = 'a' + GetFile(from);
	int fromRank = '1' + GetRank(from);
	int toFile = 'a' + GetFile(to);
	int toRank = '1' + GetRank(to);

	int promoted = PromotedPiece(move);

	if (promoted) {
		sprintf(pString, "%c%c%c%c%c", fromFile, fromRank, toFile, toRank, PromotedChar[promoted]);
		return pString;
	}
	else {
		sprintf(string, "%c%c%c%c", fromFile, fromRank, toFile, toRank);
		return string;
	}
}

int ParseMove(char* move, const Position* position) {
	MoveList list[1];
	GenerateMoves(position, list);

	int fromSquare = GetSquare((move[0] - 'a'), (move[1] - '1'));
	int toSquare = GetSquare((move[2] - 'a'), (move[3] - '1'));

	for (int i = 0; i < list->count; i++) {
		int _move = list->moves[i].move;

		if (FromSquare(_move) == fromSquare && ToSquare(_move) == toSquare) {
			int promoted = PromotedPiece(_move);

			if (promoted) {
				switch (move[4]) {
					case 'q': if (IsQueen(promoted)) return _move; break;
					case 'r': if (IsRook(promoted)) return _move; break;
					case 'b': if (IsBishop(promoted)) return _move; break;
					case 'n': if (IsKnight(promoted)) return _move; break;
				}

				continue;
			}

			return _move;
		}
	}

	return False;
}

void PrintBoard(const Position* position) {
	for (int rank = Rank8; rank >= Rank1; rank--) {
		printf(" %d  ", rank + 1);

		for (int file = FileA; file <= FileH; file++) {
			int square = GetSquare(file, rank);
			int piece = -1;

			for (int _piece = wP; _piece <= bK; _piece++) {
				if (GetBit(position->bitboards[_piece], square)) {
					piece = _piece;
					break;
				}
			}

			printf(" %c", (piece != -1) ? PieceChar[piece] : '.');
		}

		printf("\n");
	}

	printf("\n     A B C D E F G H\n\n");

	char side = '-';
	if (position->side == White) side = 'w';
	else if (position->side == Black) side = 'b';

	printf("Side:         %c\n", side);
	printf("En Passant:   %s\n", (position->enPassant != NoSquare ? SquareString(position->enPassant) : "none"));
	printf("Castling:     %c%c%c%c\n\n",
		(position->castling & WKC ? 'K' : '-'),
		(position->castling & WQC ? 'Q' : '-'),
		(position->castling & BKC ? 'k' : '-'),
		(position->castling & BQC ? 'q' : '-'));
	printf("Position key: %llx\n\n", position->positionKey);
}

void PrintBitboard(U64 bitboard) {
	for (int rank = Rank8; rank >= Rank1; rank--) {
		printf(" %d ", rank + 1);

		for (int file = FileA; file <= FileH; file++) {
			int square = GetSquare(file, rank);

			GetBit(bitboard, square) ? printf(" 1") : printf(" 0");
		}
		
		printf("\n");
	}

	printf("\n    A B C D E F G H\n\n");
}