#include <stdio.h>
#include <string.h>
#include "Definitions.h"

#define Window 50
#define NullMoveReduction 2
#define FullDepthMoves 4
#define ReductionLimit 3

int bestMove;

void CheckTimeUp(SearchInfo* info) {
	if (info->timeSet && (GetTimeMS() > info->stopTime)) info->stopped = True;

	ReadInput(info);
}

void ResetSearchInfo(Position* position, SearchInfo* info) {
	memset(position->killerMoves, 0, sizeof(position->killerMoves));
	memset(position->historyMoves, 0, sizeof(position->historyMoves));
	memset(position->pvLength, 0, sizeof(position->pvLength));
	memset(position->pvTable, 0, sizeof(position->pvTable));

	position->ply = 0;

	info->stopped = False;
	info->nodes = 0;
}

static inline int IsRepitition(Position* position) {
	for (int i = position->ply - position->fiftyMoveRule; i < position->ply - 1; i++) {
		if (position->positionKey == position->history[i].positionKey) return True;
	}

	return False;
}

static inline void OrderMove(int index, MoveList* list) {
	Move move;
	int bestScore = list->moves[index].score;
	int bestIndex = index;

	for (int i = index + 1; i < list->count; ++i) {
		if (list->moves[i].score > bestScore) {
			bestScore = list->moves[i].score;
			bestIndex = i;
		}
	}

	move = list->moves[index];
	list->moves[index] = list->moves[bestIndex];
	list->moves[bestIndex] = move;
}

static inline int Quiescence(int alpha, int beta, Position* position, SearchInfo* info) {
	if ((info->nodes & 2047) == 0) CheckTimeUp(info);

	info->nodes++;

	if (IsRepitition(position) || position->fiftyMoveRule >= 100) return 0;

	int score = Evaluate(position);

	if (position->ply >= MaxDepth) return score;

	if (score > alpha) {
		if (score >= beta) return beta;
		
		alpha = score;
	}

	MoveList list[1];
	GenerateCaptures(position, list);

	for (int i = 0; i < list->count; i++) {
		OrderMove(i, list);

		if (!MakeMove(list->moves[i].move, position)) continue;

		int score = -Quiescence(-beta, -alpha, position, info);

		TakeMove(position);

		if (info->stopped) return 0;

		if (score > alpha) {
			if (score >= beta) return beta;

			alpha = score;
		}
	}

	return alpha;
}

static inline int AlphaBeta(int alpha, int beta, int depth, Position* position, SearchInfo* info, int doNull) {
	position->pvLength[position->ply] = position->ply;

	if (depth <= 0) return Quiescence(alpha, beta, position, info);

	if ((info->nodes & 2047) == 0) CheckTimeUp(info);

	info->nodes++;

	if (IsRepitition(position) || position->fiftyMoveRule >= 100) return 0;
	if (position->ply >= MaxDepth) return Evaluate(position);

	int foundPv = False;

	int kingSquare = GLS1BI(position->bitboards[position->side == White ? wK : bK]);
	int inCheck = SquareAttacked(kingSquare, position->side ^ 1, position);

	if (inCheck) depth++;
	else {
		int opponentKingSquare = GLS1BI(position->bitboards[position->side == White ? bK : wK]);
		int opponentInCheck = SquareAttacked(opponentKingSquare, position->side, position);

		if (opponentInCheck) depth++;
	}

	/*if (doNull && !inCheck && position->ply && depth >= 3) {
		MakeNullMove(position);

		int score = -AlphaBeta(-beta, -beta + 1, depth - 1 - NullMoveReduction, position, info, False);

		TakeNullMove(position);

		if (info->stopped) return 0;

		if (score >= beta && !IsMate(score)) return beta;
	}*/

	int legalMoves = 0;
	int movesSearched = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	int pvMove = position->pvTable[0][position->ply];

	if (pvMove) {
		for (int i = 0; i < list->count; i++) {
			if (pvMove == list->moves[i].move) {
				list->moves[i].score = 1000;
				break;
			}
		}
	}

	for (int i = 0; i < list->count; i++) {
		OrderMove(i, list);

		int move = list->moves[i].move;

		if (!MakeMove(move, position)) continue;

		legalMoves++;

		int score;
		//int score = -AlphaBeta(-beta, -alpha, depth - 1, position, info, True);

		if (movesSearched == 0) {
			score = -AlphaBeta(-beta, -alpha, depth - 1, position, info, True);
		}
		else {
			if (!inCheck && movesSearched >= FullDepthMoves && depth >= ReductionLimit && list->moves[i].score == 0) {
				score = -AlphaBeta(-alpha - 1, -alpha, depth - 2, position, info, True);
			}
			else score = alpha + 1;

			if (score > alpha) {
				score = -AlphaBeta(-alpha - 1, -alpha, depth - 1, position, info, True);

				if (score > alpha && score < beta) {
					score = -AlphaBeta(-beta, -alpha, depth - 1, position, info, True);
				}
			}
		}

		TakeMove(position);

		movesSearched++;

		if (info->stopped) return 0;

		if (score > alpha) {
			if (score >= beta) {
				if (!IsCapture(move)) {
					position->killerMoves[1][position->ply] = position->killerMoves[0][position->ply];
					position->killerMoves[0][position->ply] = move;
				}

				return beta;
			}

			alpha = score;

			foundPv = True;
			position->pvTable[position->ply][position->ply] = move;

			for (int nextPly = position->ply + 1; nextPly < position->pvLength[position->ply + 1]; nextPly++) {
				position->pvTable[position->ply][nextPly] = position->pvTable[position->ply + 1][nextPly];
			}

			position->pvLength[position->ply] = position->pvLength[position->ply + 1];
			
			if (!IsCapture(move)) {
				position->historyMoves[MovedPiece(move)][ToSquare(move)] += depth;
			}
		}
	}

	if (legalMoves == 0) {
		if (inCheck) return -Infinity + position->ply;
		else return 0;
	}

	return alpha;
}

void Search(Position* position, SearchInfo* info) {
	ResetSearchInfo(position, info);

	int alpha = -Infinity;
	int beta = Infinity;

	for (int depth = 1; depth <= info->depth; depth++) {
		int score = AlphaBeta(alpha, beta, depth, position, info, True);

		if (info->stopped) break;

		printf("info score cp %d depth %d nodes %d time %d pv",
			score, depth, info->nodes, (GetTimeMS() - info->startTime));

		for (int i = 0; i < position->pvLength[0]; i++) {
			printf(" %s", MoveString(position->pvTable[0][i]));
		}

		printf("\n");

		/*
		if (score <= alpha || score >= beta) {
			alpha = -Infinity;
			beta = Infinity;
			continue;
		}

		alpha = score - Window;
		beta = score + Window;
		*/
	}

	printf("bestmove %s\n", MoveString(position->pvTable[0][0]));
}