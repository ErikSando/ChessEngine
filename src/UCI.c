#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Definitions.h"

#define InputBuffer 2000

void ParsePosition(char* line, Position* position) {
	line += 9;

	char* ptr = line;

	if (!strncmp(line, "startpos", 8)) {
		ParseFEN(StartFEN, position);
	}
	else {
		ptr = strstr(line, "fen");

		if (ptr != NULL) {
			ParseFEN(ptr + 4, position);
		}
		else {
			ParseFEN(StartFEN, position);
		}
	}

	ptr = strstr(line, "moves");

	if (ptr != NULL) {
		ptr += 6;

		while (*ptr) {
			int move = ParseMove(ptr, position);

			if (!move) break;

			if (!MakeMove(move, position)) {
				printf("Illegal move!\n");
				break;
			}

			while (*ptr && *ptr != ' ') ptr++;
			ptr++;
		}
	}
}

void ParseGo(char* line, Position* position) {
	char* ptr = line;

	int depth = MaxDepth;
	int movetime = -1;
	int wtime = -1;
	int btime = -1;
	int winc = 0;
	int binc = 0;
	int movestogo = 30;

	int time = -1;

	SearchInfo info[1];

	info->timeSet = False;

	if ((ptr = strstr(line, "depth"))) depth = atoi(ptr + 6);
	if ((ptr = strstr(line, "movetime"))) movetime = atoi(ptr + 9);
	if ((ptr = strstr(line, "wtime")) && position->side == White) time = atoi(ptr + 6);
	if ((ptr = strstr(line, "btime")) && position->side == Black) time = atoi(ptr + 6);
	if ((ptr = strstr(line, "winc"))) winc = atoi(ptr + 5);
	if ((ptr = strstr(line, "binc"))) binc = atoi(ptr + 5);
	if ((ptr = strstr(line, "movestogo"))) movestogo = atoi(ptr + 10);

	if (movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	if (time != -1) {
		info->timeSet = True;

		int inc = position->side == White ? winc : binc;

		time /= movestogo;
		time += inc;
		time -= 50;
	}

	info->startTime = GetTimeMS();
	info->stopTime = info->startTime + time;
	info->depth = depth;

	Search(position, info);
}

static inline int IsRepetition(Position* position) {
	int repetitions = 0;

	for (int i = position->ply - position->fiftyMoveRule; i < position->ply - 1; i += 2) {
		if (position->positionKey == position->history[i].positionKey) repetitions++;
	}

	// current position is not counted, so 2 repetitions would mean 3 total occurences
	return repetitions >= 2;
}

void UCILoop() {
	Position position[1];
	InitHashTable(position->hashTable, 64);

	ParseFEN(StartFEN, position);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	char input[InputBuffer];

	printf("id name %s\n", NAME);
	printf("id author Erik\n\n");

	while (True) {
		memset(input, 0, sizeof(input));

		fflush(stdout);

		if (!fgets(input, InputBuffer, stdin) || input[0] == '\n') continue;

		if (!strncmp(input, "ucinewgame", 10)) {
			ParseFEN(StartFEN, position);
			ClearHashTable(position->hashTable);
		}
		else if (!strncmp(input, "uci", 3)) {
			printf("uciok\n");
		}
		else if (!strncmp(input, "isready", 7)) {
			printf("readyok\n");
		}
		else if (!strncmp(input, "position", 8)) {
			ParsePosition(input, position);
		}
		else if (!strncmp(input, "go", 2)) {
			ParseGo(input, position);
		}
		else if (!strncmp(input, "quit", 4)) {
			break;
		}
		else if (!strncmp(input, "print", 5)) {
			PrintBoard(position);
		}
		else if (!strncmp(input, "eval", 4)) {
			printf("\nStatic evaluation: %d\n", Evaluate(position));
		}
		else if (!strncmp(input, "perft", 5)) {
			int depth = atoi(input + 5);
			PerftTest(depth, position);
		}
		else if (!strncmp(input, "rep", 3)) {
			printf("%s\n", IsRepetition(position) ? "is repetition" : "is not repetition");
		}
		else if (!strncmp(input, "take", 4)) {
			if (position->ply > 0) {
				TakeMove(position);
				PrintBoard(position);
			}
		}
		else {
			int move = ParseMove(input, position);

			if (move) {
				if (MakeMove(move, position)) PrintBoard(position);
				else printf("Illegal move!\n");
			}
		}
	}

	free(position->hashTable->entries);
}