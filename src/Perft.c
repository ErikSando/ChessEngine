#include <stdio.h>
#include "Definitions.h"

U64 Perft(int depth, Position* position) {
	if (depth <= 0) return 1L;

	U64 nodes = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	// use this once the legal move generator is finished
	//if (depth == 1) return (U64) list->count;

	for (int i = 0; i < list->count; i++) {
		if (!MakeMove(list->moves[i].move, position)) continue;

		nodes += Perft(depth - 1, position);

		TakeMove(position);
	}

	return nodes;
}

void PerftTest(int depth, Position* position) {
	U64 nodes = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	printf("\nBeginning test to depth %d\n\n", depth);

	int start = GetTimeMS();

	for (int i = 0; i < list->count; i++) {
		if (!MakeMove(list->moves[i].move, position)) continue;

		U64 oldNodes = nodes;
		nodes += Perft(depth - 1, position);
		U64 newNodes = nodes - oldNodes;

		printf("%s: %llu\n", MoveString(list->moves[i].move), newNodes);

		TakeMove(position);
	}

	int time = GetTimeMS() - start;
	int nps = time != 0 ? nodes / time * 1000 : 0;

	printf("\n");
	printf("Nodes visited:      %llu\n", nodes);
	printf("Time elapsed:       %d ms\n", time);
	printf("Nodes per second:   ");
	nps != 0 ? printf("%d", nps) : printf("nil");
	printf("\n\n");
}