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
	// could use this and restore board to cut down the lines of code but doesnt seem necessary
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

	position->history[position->historyPly].positionKey = position->positionKey;
	position->history[position->historyPly].enPassant = position->enPassant;
	position->history[position->historyPly].castling = position->castling;
	position->history[position->historyPly].fiftyMoveRule = position->fiftyMoveRule;
	position->history[position->historyPly].move = move;

	position->ply++;
	position->historyPly++;

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

		if (IsBigPiece(captured)) position->bigPieces[enemy]--;
	}
	else if (IsEnPassant(move)) {
		if (side == White) {
			ClearBit(position->bitboards[bP], toSquare - 8);
			ClearBit(position->occupancy[Black], toSquare - 8);
		}
		else {
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

	position->side ^= 1;
	HashSide;

	int kingSquare = GLS1BI(position->bitboards[side == White ? wK : bK]);

	if (SquareAttacked(kingSquare, enemy, position)) {
		TakeMove(position);
		return False;
	}

	return True;
}

void TakeMove(Position* position) {
	position->ply--;
	position->historyPly--;

	position->positionKey = position->history[position->historyPly].positionKey;
	position->enPassant = position->history[position->historyPly].enPassant;
	position->castling = position->history[position->historyPly].castling;
	position->fiftyMoveRule = position->history[position->historyPly].fiftyMoveRule;

	position->side ^= 1;

	int move = position->history[position->historyPly].move;

	int fromSquare = FromSquare(move);
	int toSquare = ToSquare(move);
	int piece = MovedPiece(move);
	int captured = CapturedPiece(move);
	int promoted = PromotedPiece(move);
	int side = position->side;
	int enemy = side ^ 1;

	ClearBit(position->occupancy[side], toSquare);

	if (promoted) ClearBit(position->bitboards[promoted], toSquare);
	else ClearBit(position->bitboards[piece], toSquare);

	SetBit(position->bitboards[piece], fromSquare);
	SetBit(position->occupancy[side], fromSquare);
	
	if (IsCapture(move)) {
		SetBit(position->bitboards[captured], toSquare);
		SetBit(position->occupancy[enemy], toSquare);

		if (IsBigPiece(captured)) position->bigPieces[enemy]++;
	}
	else if (IsEnPassant(move)) {
		if (side == White) {
			SetBit(position->bitboards[bP], toSquare - 8);
			SetBit(position->occupancy[Black], toSquare - 8);
		}
		else {
			SetBit(position->bitboards[wP], toSquare + 8);
			SetBit(position->occupancy[White], toSquare + 8);
		}
	}
	else if (IsCastling(move)) {
		switch (toSquare) {
			case G1:
				ClearBit(position->bitboards[wR], F1);
				ClearBit(position->occupancy[White], F1);
				SetBit(position->bitboards[wR], H1);
				SetBit(position->occupancy[White], H1);
				break;

			case C1:
				ClearBit(position->bitboards[wR], D1);
				ClearBit(position->occupancy[White], D1);
				SetBit(position->bitboards[wR], A1);
				SetBit(position->occupancy[White], A1);
				break;

			case G8:
				ClearBit(position->bitboards[bR], F8);
				ClearBit(position->occupancy[Black], F8);
				SetBit(position->bitboards[bR], H8);
				SetBit(position->occupancy[Black], H8);
				break;

			case C8:
				ClearBit(position->bitboards[bR], D8);
				ClearBit(position->occupancy[Black], D8);
				SetBit(position->bitboards[bR], A8);
				SetBit(position->occupancy[Black], A8);
				break;
		}
	}

	position->occupancy[Both] = position->occupancy[White] | position->occupancy[Black];
}

void MakeNullMove(Position* position) {
	position->history[position->historyPly].positionKey = position->positionKey;
	position->history[position->historyPly].enPassant = position->enPassant;
	position->history[position->historyPly].castling = position->castling;
	position->history[position->historyPly].fiftyMoveRule = position->fiftyMoveRule;

	position->side ^= 1;
	HashSide;

	if (position->enPassant != NoSquare) {
		HashEnPassant;
		position->enPassant = NoSquare;
	}

	position->ply++;
	position->historyPly++;
}

void TakeNullMove(Position* position) {
	position->ply--;
	position->historyPly--;

	position->positionKey = position->history[position->historyPly].positionKey;
	position->enPassant = position->history[position->historyPly].enPassant;
	position->castling = position->history[position->historyPly].castling;
	position->fiftyMoveRule = position->history[position->historyPly].fiftyMoveRule;

	position->side ^= 1;
}