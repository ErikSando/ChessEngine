#include <stdio.h>
#include <string.h>
#include "Definitions.h"

#define Window 50
#define NullMoveReduction 2
#define FullDepthMoves 4
#define ReductionLimit 3

void CheckTimeUp(SearchInfo* info) {
	if (info->timeSet && (GetTimeMS() > info->stopTime)) info->stopped = True;

	ReadInput(info);
}

void ResetSearchInfo(Position* position, SearchInfo* info) {
	memset(position->killerMoves, 0, sizeof(position->killerMoves));
	memset(position->historyMoves, 0, sizeof(position->historyMoves));

	position->ply = 0;

	info->stopped = False;
	info->nodes = 0;
}

static inline int IsRepetition(Position* position) {
	int repetitions = 0;

	for (int i = position->historyPly - position->fiftyMoveRule; i < position->historyPly - 1; i += 2) {
		if (position->positionKey == position->history[i].positionKey) repetitions++;
	}

	// current position is not counted, so 2 repetitions would mean 3 total occurences
	return repetitions >= 2;
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

	if (IsRepetition(position) || position->fiftyMoveRule >= 100) return 0;

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
	if (depth <= 0) return Quiescence(alpha, beta, position, info);

	if ((info->nodes & 2047) == 0) CheckTimeUp(info);

	info->nodes++;

	if (IsRepetition(position) || position->fiftyMoveRule >= 100) return 0;
	if (position->ply >= MaxDepth) return Evaluate(position);

	int kingSquare = GLS1BI(position->bitboards[position->side == White ? wK : bK]);
	int inCheck = SquareAttacked(kingSquare, position->side ^ 1, position);

	if (inCheck) depth++;
	else {
		int opponentKingSquare = GLS1BI(position->bitboards[position->side == White ? bK : wK]);
		int opponentInCheck = SquareAttacked(opponentKingSquare, position->side, position);

		if (opponentInCheck) depth++;
	}

	int pvMove = 0;
	//int pvNode = beta - alpha > 1;
	int score = GetHashEntry(&pvMove, alpha, beta, depth, position);

	if (position->ply && score != NoScore) return score;
	//if (position->ply && !pvNode && score != NoScore) return score;

	if (doNull && !inCheck && position->ply && depth >= 3) {
		MakeNullMove(position);

		score = -AlphaBeta(-beta, -beta + 1, depth - 1 - NullMoveReduction, position, info, False);

		TakeNullMove(position);

		if (info->stopped) return 0;

		if (score >= beta && !IsMate(score)) return beta;
	}

	int legalMoves = 0;
	int movesSearched = 0;
	int oldAlpha = alpha;
	int bestMove = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	if (pvMove) {
		for (int i = 0; i < list->count; i++) {
			if (pvMove == list->moves[i].move) {
				list->moves[i].score = 1000;
				break;
			}
		}
	}

	int hashFlag = AlphaFlag;

	for (int i = 0; i < list->count; i++) {
		OrderMove(i, list);

		int move = list->moves[i].move;

		if (!MakeMove(move, position)) continue;

		legalMoves++;

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
				StoreHashEntry(move, beta, depth, BetaFlag, position);

				if (!IsCapture(move)) {
					position->killerMoves[1][position->ply] = position->killerMoves[0][position->ply];
					position->killerMoves[0][position->ply] = move;
				}

				return beta;
			}

			bestMove = move;

			hashFlag = ExactFlag;
			alpha = score;

			if (!IsCapture(move)) {
				position->historyMoves[MovedPiece(move)][ToSquare(move)] += depth;
			}
		}
	}

	if (legalMoves == 0) {
		if (inCheck) return -Infinity + position->ply;
		else return 0;
	}

	StoreHashEntry(bestMove, alpha, depth, hashFlag, position);
	
	return alpha;
}

void Search(Position* position, SearchInfo* info) {
	ResetSearchInfo(position, info);

	int alpha = -Infinity;
	int beta = Infinity;

	for (int depth = 1; depth <= info->depth; depth++) {
		int score = AlphaBeta(alpha, beta, depth, position, info, True);

		if (info->stopped) break;

		int pvLength = GetPvLength(depth, position);

		printf("info score cp %d depth %d nodes %d time %d pv",
			score, depth, info->nodes, (GetTimeMS() - info->startTime));

		for (int i = 0; i < pvLength; i++) {
			printf(" %s", MoveString(position->pvArray[i]));
		}

		printf("\n");

		if (score <= alpha || score >= beta) {
			alpha = -Infinity;
			beta = Infinity;
			continue;
		}

		alpha = score - Window;
		beta = score + Window;
	}

	printf("bestmove %s\n", MoveString(position->pvArray[0]));
}