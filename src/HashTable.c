#include <stdio.h>
#include <stdlib.h>
#include "Definitions.h"

void ClearHashTable(HashTable* table) {
	HashEntry* entry;
	
	for (entry = table->entries; entry < table->entries + table->count; entry++) {
		entry->positionKey = 0ULL;
		entry->score = 0;
		entry->depth = 0;
		entry->flag = 0;
	}
}

void InitHashTable(HashTable* table, const int MB) {
	int HashSize = 0x100000 * MB;

	table->count = HashSize / sizeof(HashEntry) - 2;
	table->entries = NULL;
	free(table->entries);
	table->entries = (HashEntry*) malloc(table->count * sizeof(HashEntry));

	ClearHashTable(table);

	printf("Hash table initialised with %d entries\n", table->count);
}

int GetHashEntry(int* pvMove, int alpha, int beta, int depth, Position* position) {
	int index = position->positionKey % position->hashTable->count;
	HashEntry* entry = &position->hashTable->entries[index];

	if (entry->positionKey == position->positionKey) {
		*pvMove = entry->move;

		if (entry->depth >= depth) {
			int score = entry->score;

			switch (entry->flag) {
				case ExactFlag:
					// if (score >= MateScore) ??? if (score <= -MateScore) ???
					if (score > MateScore) score -= position->ply;
					else if (score < -MateScore) score += position->ply;

					return score;

				case AlphaFlag:
					if (score <= alpha) return alpha;
					break;

				case BetaFlag:
					if (score >= beta) return beta;
					break;
			}
		}
	}

	return NoScore;
}

void StoreHashEntry(int move, int score, int depth, int flag, Position* position) {
	int index = position->positionKey % position->hashTable->count;
	HashEntry* entry = &position->hashTable->entries[index];

	if (score > MateScore) score += position->ply;
	else if (score < -MateScore) score -= position->ply;

	entry->positionKey = position->positionKey;
	entry->move = move;
	entry->score = score;
	entry->depth = depth;
	entry->flag = flag;
}

int GetPvMove(const Position* position) {
	int index = position->positionKey % position->hashTable->count;
	HashEntry* entry = &position->hashTable->entries[index];

	if (position->positionKey == entry->positionKey) return entry->move;

	return False;
}

int GetPvLength(const int depth, Position* position) {
	int move = GetPvMove(position);
	int count = 0;

	while (move && count < depth) {
		if (MoveExists(move, position)) {
			MakeMove(move, position);
			position->pvArray[count] = move;
			count++;
		}
		else break;

		move = GetPvMove(position);
	}

	while (position->ply > 0) TakeMove(position);

	return count;
}