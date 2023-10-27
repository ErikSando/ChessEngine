#include <stdio.h>
#include <string.h>
#include "Definitions.h"

#define HashPiece(piece, square) (position->positionKey ^= PieceKeys[(piece)][(square)])
#define HashEnPassant (position->positionKey ^= EnPassantKeys[position->enPassant])
#define HashCastling (position->positionKey ^= CastlingKeys[(position->castling)])
#define HashSide (position->positionKey ^= SideKey)

const int CastlingUpdate[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11
};

static inline void SaveBoard(Position* position) {

}

static inline void RestoreBoard(Position* position) {

}

int MakeMove(int move, Position* position) {
	int fromSquare = FromSquare(move);
	int toSquare = ToSquare(move);
	int piece = MovedPiece(move);
	int promoted = PromotedPiece(move);
	int side = position->side;
	int enemy = side ^ 1;

	memcpy(position->history[position->ply].bitboards, position->bitboards, sizeof(position->bitboards));
	memcpy(position->history[position->ply].occupancy, position->occupancy, sizeof(position->occupancy));

	position->history[position->ply].positionKey = position->positionKey;
	position->history[position->ply].side = position->side;
	position->history[position->ply].enPassant = position->enPassant;
	position->history[position->ply].castling = position->castling;
	position->history[position->ply].fiftyMoveRule = position->fiftyMoveRule;

	position->ply++;
	
	ClearBit(position->bitboards[piece], fromSquare);
	ClearBit(position->occupancy[side], fromSquare);
	SetBit(position->occupancy[side], toSquare);

	HashPiece(piece, fromSquare);

	if (promoted) {
		HashPiece(promoted, toSquare);
		SetBit(position->bitboards[promoted], toSquare);
	}
	else {
		HashPiece(piece, toSquare);
		SetBit(position->bitboards[piece], toSquare);
	}

	if (position->enPassant != NoSquare) {
		HashEnPassant;
		position->enPassant = NoSquare;
	}

	HashCastling;
	position->castling &= CastlingUpdate[fromSquare];
	position->castling &= CastlingUpdate[toSquare];
	HashCastling;

	position->fiftyMoveRule++;

	if (IsPawn(piece)) {
		position->fiftyMoveRule = 0;

		if (IsPawnStart(move)) {
			position->enPassant = toSquare - (side == White ? 8 : -8);
			HashEnPassant;
		}
	}

	if (IsCapture(move)) {
		position->fiftyMoveRule = 0;

		int captured = CapturedPiece(move);

		ClearBit(position->bitboards[captured], toSquare);
		ClearBit(position->occupancy[enemy], toSquare);
		HashPiece(captured, toSquare);
	}
	else if (IsEnPassant(move)) {
		if (side == White) {
			ClearBit(position->bitboards[bP], toSquare - 8);
			ClearBit(position->occupancy[Black], toSquare - 8);
		} else {
			ClearBit(position->bitboards[wP], toSquare + 8);
			ClearBit(position->occupancy[White], toSquare + 8);
		}
	}
	else if (IsCastling(move)) {
		switch (toSquare) {
			case G1:
				ClearBit(position->bitboards[wR], H1);
				ClearBit(position->occupancy[White], H1);
				SetBit(position->bitboards[wR], F1);
				SetBit(position->occupancy[White], F1);
				break;

			case C1:
				ClearBit(position->bitboards[wR], A1);
				ClearBit(position->occupancy[White], A1);
				SetBit(position->bitboards[wR], D1);
				SetBit(position->occupancy[White], D1);
				break;

			case G8:
				ClearBit(position->bitboards[bR], H8);
				ClearBit(position->occupancy[Black], H8);
				SetBit(position->bitboards[bR], F8);
				SetBit(position->occupancy[Black], F8);
				break;

			case C8:
				ClearBit(position->bitboards[bR], A8);
				ClearBit(position->occupancy[Black], A8);
				SetBit(position->bitboards[bR], D8);
				SetBit(position->occupancy[Black], D8);
				break;
		}
	}

	position->occupancy[Both] = position->occupancy[White] | position->occupancy[Black];

	int kingSquare = GLS1BI(position->bitboards[side == White ? wK : bK]);

	if (SquareAttacked(kingSquare, enemy, position)) {
		TakeMove(position);
		return False;
	}

	position->side ^= 1;
	HashSide;

	return True;
}

void TakeMove(Position* position) {
	position->ply--;

	memcpy(position->bitboards, position->history[position->ply].bitboards, sizeof(position->history[position->ply].bitboards));
	memcpy(position->occupancy, position->history[position->ply].occupancy, sizeof(position->history[position->ply].occupancy));

	position->positionKey = position->history[position->ply].positionKey;
	position->side = position->history[position->ply].side;
	position->enPassant = position->history[position->ply].enPassant;
	position->castling = position->history[position->ply].castling;
	position->fiftyMoveRule = position->history[position->ply].fiftyMoveRule;
}

void MakeNullMove(Position* position) {
	memcpy(position->history[position->ply].bitboards, position->bitboards, sizeof(position->bitboards));
	memcpy(position->history[position->ply].occupancy, position->occupancy, sizeof(position->occupancy));

	position->history[position->ply].positionKey = position->positionKey;
	position->history[position->ply].side = position->side;
	position->history[position->ply].enPassant = position->enPassant;
	position->history[position->ply].castling = position->castling;
	position->history[position->ply].fiftyMoveRule = position->fiftyMoveRule;

	position->ply++;
}

void TakeNullMove(Position* position) {
	position->ply--;

	memcpy(position->bitboards, position->history[position->ply].bitboards, sizeof(position->history[position->ply].bitboards));
	memcpy(position->occupancy, position->history[position->ply].occupancy, sizeof(position->history[position->ply].occupancy));

	position->positionKey = position->history[position->ply].positionKey;
	position->side = position->history[position->ply].side;
	position->enPassant = position->history[position->ply].enPassant;
	position->castling = position->history[position->ply].castling;
	position->fiftyMoveRule = position->history[position->ply].fiftyMoveRule;
}