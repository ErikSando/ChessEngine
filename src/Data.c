#include "Definitions.h"

char PieceChar[12] = "PNBRQKpnbrqk";

char PromotedChar[] = {
	[wQ] = 'q', [wR] = 'r', [wB] = 'b', [wN] = 'n',
	[bQ] = 'q', [bR] = 'r', [bB] = 'b', [bN] = 'n'
};

int CharPiece[] = {
	['P'] = wP, ['N'] = wN, ['B'] = wB, ['R'] = wR, ['Q'] = wQ, ['K'] = wK,
	['p'] = bP, ['n'] = bN, ['b'] = bB, ['r'] = bR, ['q'] = bQ, ['k'] = bK
};

int PiecePawn[12] = { True, False, False, False, False, False, True, False, False, False, False, False };
int PieceKnight[12] = { False, True, False, False, False, False, False, True, False, False, False, False };
int PieceBishop[12] = { False, False, True, False, False, False, False, False, True, False, False, False };
int PieceRook[12] = { False, False, False, True, False, False, False, False, False, True, False, False };
int PieceQueen[12] = { False, False, False, False, True, False, False, False, False, False, True, False };
int PieceKing[12] = { False, False, False, False, False, True, False, False, False, False, False, True };

int PieceBig[12] = { False, True, True, True, True, True, False, True, True, True, True, True };

int PieceColour[12] = { White, White, White, White, White, White, Black, Black, Black, Black, Black, Black };