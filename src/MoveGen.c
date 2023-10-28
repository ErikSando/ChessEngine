#include <stdio.h>
#include "Definitions.h"

const int MvvLvaScore[12][12] = {
	{ 105, 104, 103, 102, 101, 100, 105, 104, 103, 102, 101, 100 },
	{ 205, 204, 203, 202, 201, 200, 205, 204, 203, 202, 201, 200 },
	{ 305, 304, 303, 302, 301, 300, 305, 304, 303, 302, 301, 300 },
	{ 405, 404, 403, 402, 401, 400, 405, 404, 403, 402, 401, 400 },
	{ 505, 504, 503, 502, 501, 500, 505, 504, 503, 502, 501, 500 },
	{ 605, 604, 603, 602, 601, 600, 605, 604, 603, 602, 601, 600 },
	{ 105, 104, 103, 102, 101, 100, 105, 104, 103, 102, 101, 100 },
	{ 205, 204, 203, 202, 201, 200, 205, 204, 203, 202, 201, 200 },
	{ 305, 304, 303, 302, 301, 300, 305, 304, 303, 302, 301, 300 },
	{ 405, 404, 403, 402, 401, 400, 405, 404, 403, 402, 401, 400 },
	{ 505, 504, 503, 502, 501, 500, 505, 504, 503, 502, 501, 500 },
	{ 605, 604, 603, 602, 601, 600, 605, 604, 603, 602, 601, 600 }
};

#define Move(from, to, piece, captured, promoted, flag) \
	((from) | ((to) << 6) | ((piece) << 12) | ((captured) << 16) | (promoted << 20) | (flag))

static inline void AddMove(int move, int score, MoveList* list) {
	list->moves[list->count].move = move;
	list->moves[list->count].score = score;
	list->count++;
}

int MoveExists(int move, Position* position) {
	MoveList list[1];
	GenerateMoves(position, list);

	for (int i = 0; i < list->count; i++) {
		if (list->moves[i].move != move || !MakeMove(list->moves[i].move, position)) continue;
		TakeMove(position);
		return True;
	}

	return False;
}

void GenerateMoves(const Position* position, MoveList* list) {
	list->count = 0;

	int side = position->side;
	int enemy = side ^ 1;
	int piece = side == White ? wP : bP;
	int captureStart = side == White ? bP : wP;
	int direction = side == White ? 8 : -8;
	int promotionRank = side == White ? Rank8 : Rank1;

	U64 bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 moves = PawnMoves[side][square] & ~position->occupancy[Both];
		U64 captures = PawnCaptures[side][square] & position->occupancy[enemy];

		while (moves) {
			int toSquare = GLS1BI(moves);
			ClearBit(moves, toSquare);

			int flag = NoFlag;

			if (toSquare - square == direction * 2) {
				if (GetBit(position->occupancy[Both], square + direction)) continue;

				flag = PawnStartFlag;
			}
			else if (GetRank(toSquare) == promotionRank) {
				AddMove(Move(square, toSquare, piece, 0, piece + 4, NoFlag), 500, list); // queen
				AddMove(Move(square, toSquare, piece, 0, piece + 3, NoFlag), 400, list); // rook
				AddMove(Move(square, toSquare, piece, 0, piece + 2, NoFlag), 210, list); // bishop
				AddMove(Move(square, toSquare, piece, 0, piece + 1, NoFlag), 200, list); // knight
				continue;
			}

			int move = Move(square, toSquare, piece, 0, 0, flag);
			int score = 0;

			if (position->killerMoves[0][position->ply] == move) score = 900;
			else if (position->killerMoves[1][position->ply] == move) score = 800;
			else score = position->historyMoves[piece][toSquare];

			AddMove(move, score, list);
		}

		while (captures) {
			int toSquare = GLS1BI(captures);
			ClearBit(captures, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int score = MvvLvaScore[captured][piece];

			if (GetRank(toSquare) == promotionRank) {
				AddMove(Move(square, toSquare, piece, captured, piece + 4, CaptureFlag), score + 40, list); // queen
				AddMove(Move(square, toSquare, piece, captured, piece + 3, CaptureFlag), score + 30, list); // rook
				AddMove(Move(square, toSquare, piece, captured, piece + 2, CaptureFlag), score + 20, list); // bishop
				AddMove(Move(square, toSquare, piece, captured, piece + 1, CaptureFlag), score + 10, list); // knight
				continue;
			}

			AddMove(Move(square, toSquare, piece, captured, 0, CaptureFlag), score, list);
		}

		if (position->enPassant != NoSquare) {
			if (PawnCaptures[side][square] & (1ULL << position->enPassant)) {
				AddMove(Move(square, position->enPassant, piece, 0, 0, EnPassantFlag), 105, list);
			}
		}
	}

	if (side == White) {
		if ((position->castling & WKC) &&
			!GetBit(position->occupancy[Both], F1) &&
			!GetBit(position->occupancy[Both], G1) &&
			!SquareAttacked(E1, Black, position) &&
			!SquareAttacked(F1, Black, position)
		) {
			int move = Move(E1, G1, wK, 0, 0, CastlingFlag);
			int score = 0;

			if (position->killerMoves[0][position->ply] == move) score = 900;
			else if (position->killerMoves[1][position->ply] == move) score = 800;
			else score = position->historyMoves[wK][G1];

			AddMove(move, score, list);
		}

		if ((position->castling & WQC) &&
			!GetBit(position->occupancy[Both], D1) &&
			!GetBit(position->occupancy[Both], C1) &&
			!GetBit(position->occupancy[Both], B1) &&
			!SquareAttacked(E1, Black, position) &&
			!SquareAttacked(D1, Black, position)
		) {
			int move = Move(E1, C1, wK, 0, 0, CastlingFlag);
			int score = 0;

			if (position->killerMoves[0][position->ply] == move) score = 900;
			else if (position->killerMoves[1][position->ply] == move) score = 800;
			else score = position->historyMoves[wK][C1];

			AddMove(move, score, list);
		}
	}
	else {
		if ((position->castling & BKC) &&
			!GetBit(position->occupancy[Both], F8) &&
			!GetBit(position->occupancy[Both], G8) &&
			!SquareAttacked(E8, White, position) &&
			!SquareAttacked(F8, White, position)
		) {
			int move = Move(E8, G8, bK, 0, 0, CastlingFlag);
			int score = 0;

			if (position->killerMoves[0][position->ply] == move) score = 900;
			else if (position->killerMoves[1][position->ply] == move) score = 800;
			else score = position->historyMoves[bK][G8];

			AddMove(move, score, list);
		}

		if ((position->castling & BQC) &&
			!GetBit(position->occupancy[Both], D8) &&
			!GetBit(position->occupancy[Both], C8) &&
			!GetBit(position->occupancy[Both], B8) &&
			!SquareAttacked(E8, White, position) &&
			!SquareAttacked(D8, White, position)
		) {
			int move = Move(E8, C8, bK, 0, 0, CastlingFlag);
			int score = 0;

			if (position->killerMoves[0][position->ply] == move) score = 900;
			else if (position->killerMoves[1][position->ply] == move) score = 800;
			else score = position->historyMoves[bK][C8];

			AddMove(move, score, list);
		}
	}

	piece++; // knights
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = KnightAttacks[square] & ~position->occupancy[side];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int move;
			int score = 0;

			if (GetBit(position->occupancy[enemy], toSquare)) {
				int captured;

				for (captured = captureStart; captured <= captureStart + 6; captured++) {
					if (GetBit(position->bitboards[captured], toSquare)) break;
				}

				move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
				score = MvvLvaScore[captured][piece];
			}
			else {
				move = Move(square, toSquare, piece, 0, 0, NoFlag);

				if (position->killerMoves[0][position->ply] == move) score = 900;
				else if (position->killerMoves[1][position->ply] == move) score = 800;
				else score = position->historyMoves[piece][toSquare];
			}

			AddMove(move, score, list);
		}
	}

	piece++; // bishops
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetBishopAttacks(square, position->occupancy[Both]) & ~position->occupancy[side];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int move;
			int score = 0;

			if (GetBit(position->occupancy[enemy], toSquare)) {
				int captured;

				for (captured = captureStart; captured <= captureStart + 6; captured++) {
					if (GetBit(position->bitboards[captured], toSquare)) break;
				}

				move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
				score = MvvLvaScore[captured][piece];
			}
			else {
				move = Move(square, toSquare, piece, 0, 0, NoFlag);

				if (position->killerMoves[0][position->ply] == move) score = 900;
				else if (position->killerMoves[1][position->ply] == move) score = 800;
				else score = position->historyMoves[piece][toSquare];
			}

			AddMove(move, score, list);
		}
	}

	piece++; // rooks
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetRookAttacks(square, position->occupancy[Both]) & ~position->occupancy[side];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int move;
			int score = 0;

			if (GetBit(position->occupancy[enemy], toSquare)) {
				int captured;

				for (captured = captureStart; captured <= captureStart + 6; captured++) {
					if (GetBit(position->bitboards[captured], toSquare)) break;
				}

				move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
				score = MvvLvaScore[captured][piece];
			}
			else {
				move = Move(square, toSquare, piece, 0, 0, NoFlag);

				if (position->killerMoves[0][position->ply] == move) score = 900;
				else if (position->killerMoves[1][position->ply] == move) score = 800;
				else score = position->historyMoves[piece][toSquare];
			}

			AddMove(move, score, list);
		}
	}

	piece++; // queens
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[side];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int move;
			int score = 0;

			if (GetBit(position->occupancy[enemy], toSquare)) {
				int captured;

				for (captured = captureStart; captured <= captureStart + 6; captured++) {
					if (GetBit(position->bitboards[captured], toSquare)) break;
				}

				move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
				score = MvvLvaScore[captured][piece];
			}
			else {
				move = Move(square, toSquare, piece, 0, 0, NoFlag);

				if (position->killerMoves[0][position->ply] == move) score = 900;
				else if (position->killerMoves[1][position->ply] == move) score = 800;
				else score = position->historyMoves[piece][toSquare];
			}

			AddMove(move, score, list);
		}
	}

	piece++; // kings
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = KingAttacks[square] & ~position->occupancy[side];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int move;
			int score = 0;

			if (GetBit(position->occupancy[enemy], toSquare)) {
				int captured;

				for (captured = captureStart; captured <= captureStart + 6; captured++) {
					if (GetBit(position->bitboards[captured], toSquare)) break;
				}

				move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
				score = MvvLvaScore[captured][piece];
			}
			else {
				move = Move(square, toSquare, piece, 0, 0, NoFlag);

				if (position->killerMoves[0][position->ply] == move) score = 900;
				else if (position->killerMoves[1][position->ply] == move) score = 800;
				else score = position->historyMoves[piece][toSquare];
			}

			AddMove(move, score, list);
		}
	}
}

void GenerateCaptures(const Position* position, MoveList* list) {
	list->count = 0;

	int side = position->side;
	int enemy = side ^ 1;
	int piece = side == White ? wP : bP;
	int captureStart = side == White ? bP : wP;
	int direction = side == White ? 8 : -8;
	int promotionRank = side == White ? Rank8 : Rank1;

	U64 bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 captures = PawnCaptures[side][square] & position->occupancy[enemy];

		while (captures) {
			int toSquare = GLS1BI(captures);
			ClearBit(captures, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int score = MvvLvaScore[captured][piece];

			if (GetRank(toSquare) == promotionRank) {
				AddMove(Move(square, toSquare, piece, captured, piece + 4, CaptureFlag), score + 40, list); // queen
				AddMove(Move(square, toSquare, piece, captured, piece + 3, CaptureFlag), score + 30, list); // rook
				AddMove(Move(square, toSquare, piece, captured, piece + 2, CaptureFlag), score + 20, list); // bishop
				AddMove(Move(square, toSquare, piece, captured, piece + 1, CaptureFlag), score + 10, list); // knight
				continue;
			}

			AddMove(Move(square, toSquare, piece, captured, 0, CaptureFlag), score, list);
		}

		if (position->enPassant != NoSquare) {
			if (PawnCaptures[side][square] & (1ULL << position->enPassant)) {
				AddMove(Move(square, position->enPassant, piece, 0, 0, EnPassantFlag), 105, list);
			}
		}
	}

	piece++; // knights
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = KnightAttacks[square] & ~position->occupancy[side] & position->occupancy[enemy];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
			int score = MvvLvaScore[captured][piece];

			AddMove(move, score, list);
		}
	}

	piece++; // bishops
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetBishopAttacks(square, position->occupancy[Both]) & ~position->occupancy[side] & position->occupancy[enemy];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
			int score = MvvLvaScore[captured][piece];

			AddMove(move, score, list);
		}
	}

	piece++; // rooks
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetRookAttacks(square, position->occupancy[Both]) & ~position->occupancy[side] & position->occupancy[enemy];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
			int score = MvvLvaScore[captured][piece];

			AddMove(move, score, list);
		}
	}

	piece++; // queens
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = GetQueenAttacks(square, position->occupancy[Both]) & ~position->occupancy[side] & position->occupancy[enemy];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
			int score = MvvLvaScore[captured][piece];

			AddMove(move, score, list);
		}
	}

	piece++; // kings
	bitboard = position->bitboards[piece];

	while (bitboard) {
		int square = GLS1BI(bitboard);
		ClearBit(bitboard, square);

		U64 attacks = KingAttacks[square] & ~position->occupancy[side] & position->occupancy[enemy];

		while (attacks) {
			int toSquare = GLS1BI(attacks);
			ClearBit(attacks, toSquare);

			int captured;

			for (captured = captureStart; captured <= captureStart + 6; captured++) {
				if (GetBit(position->bitboards[captured], toSquare)) break;
			}

			int move = Move(square, toSquare, piece, captured, 0, CaptureFlag);
			int score = MvvLvaScore[captured][piece];

			AddMove(move, score, list);
		}
	}
}