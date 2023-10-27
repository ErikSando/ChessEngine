#include <stdio.h>
#include "Definitions.h"

long Perft(int depth, Position* position) {
	if (depth <= 0) return 1L;

	long leafNodes = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	for (int i = 0; i < list->count; i++) {
		if (!MakeMove(list->moves[i].move, position)) continue;

		leafNodes += Perft(depth - 1, position);

		TakeMove(position);
	}

	return leafNodes;
}

void PerftTest(int depth, Position* position) {
	long leafNodes = 0;

	MoveList list[1];
	GenerateMoves(position, list);

	printf("\nBeginning test to depth %d\n\n", depth);

	int start = GetTimeMS();

	for (int i = 0; i < list->count; i++) {
		if (!MakeMove(list->moves[i].move, position)) continue;

		long oldNodes = leafNodes;
		leafNodes += Perft(depth - 1, position);
		long newNodes = leafNodes - oldNodes;

		printf("%s: %ld\n", MoveString(list->moves[i].move), newNodes);

		TakeMove(position);
	}

	int time = GetTimeMS() - start;
	int nps = time != 0 ? leafNodes / time * 1000 : 0;

	printf("\n");
	printf("Nodes visited:      %ld\n", leafNodes);
	printf("Time elapsed:       %d ms\n", time);
	printf("Nodes per second:   ");
	nps != 0 ? printf("%d", nps) : printf("nil");
	printf("\n\n");
}