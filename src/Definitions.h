#ifndef Definitions_h
#define Definitions_h

#define NAME "Erik Engine"
#define VERSION "1.0"

#define MaxGameMoves 2048
#define MaxPositionMoves 256
#define MaxDepth 64
#define Infinity 30000
#define MateScore Infinity - MaxDepth

#define StartFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

enum { wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8 };
enum { FileA, FileB, FileC, FileD, FileE, FileF, FileG, FileH };
enum { White, Black, Both };
enum { False, True };
enum { WKC = 1, WQC = 2, BKC = 4, BQC = 8 }; // castling

enum {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

#define NoSquare -1

typedef unsigned long long U64;

#define GetSquare(file, rank) ((file) + (rank) * 8)
#define GetFile(square) (SquareFiles[(square)])
#define GetRank(square) (SquareRanks[(square)])

#define SetBit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define GetBit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define ClearBit(bitboard, square) (GetBit((bitboard), (square)) ? (bitboard) ^= (1ULL << (square)) : 0)
#define CountBits(bitboard) __builtin_popcountll((bitboard))
//#define GLS1BI(bitboard) ((bitboard) ? CountBits(((bitboard) & -(bitboard)) - 1) : -1)
#define GLS1BI(bitboard) (CountBits(((bitboard) & -(bitboard)) - 1)) // get least significant 1st bit index

#define FromSquare(move) ((move) & 0x3F)
#define ToSquare(move) (((move) & 0xFC0) >> 6)
#define MovedPiece(move) (((move) & 0xF000) >> 12)
#define CapturedPiece(move) (((move) & 0xF0000) >> 16)
#define PromotedPiece(move) (((move) & 0xF00000) >> 20)

#define CaptureFlag 0x1000000
#define PawnStartFlag 0x2000000
#define EnPassantFlag 0x4000000
#define CastlingFlag 0x8000000
#define NoFlag 0

#define IsCapture(move) ((move) & CaptureFlag)
#define IsPawnStart(move) ((move) & PawnStartFlag)
#define IsEnPassant(move) ((move) & EnPassantFlag)
#define IsCastling(move) ((move) & CastlingFlag)

#define IsPawn(piece) (PiecePawn[(piece)])
#define IsKnight(piece) (PieceKnight[(piece)])
#define IsBishop(piece) (PieceBishop[(piece)])
#define IsRook(piece) (PieceRook[(piece)])
#define IsQueen(piece) (PieceQueen[(piece)])
#define IsKing(piece) (PieceKing[(piece)])

typedef struct {
	int move;
	int score;
} Move;

typedef struct {
	Move moves[MaxPositionMoves];
	int count;
} MoveList;

#define ExactFlag 0
#define AlphaFlag 1
#define BetaFlag 2

#define NoScore 40000

typedef struct {
	U64 positionKey;
	int move;
	int score;
	int depth;
	int flag;
} HashEntry;

typedef struct {
	HashEntry* entries;
	int count;
} HashTable;

typedef struct {
	int time;
	int startTime;
	int stopTime;
	int timeSet;
	int depth;
	int stopped;
	int bestMove;
	long nodes;
	int quit;
} SearchInfo;

typedef struct {
	U64 bitboards[12];
	U64 occupancy[3];
	U64 positionKey;
	int side;
	int enPassant;
	int castling;
	int fiftyMoveRule;
} PositionInfo;

typedef struct {
	U64 bitboards[12];
	U64 occupancy[3];
	U64 positionKey;
	int side;
	int enPassant;
	int castling;
	int fiftyMoveRule;
	int ply;
	int historyPly;
	PositionInfo history[MaxGameMoves];
	int killerMoves[2][MaxDepth];
	int historyMoves[12][64];
	HashTable hashTable[1];
	int pvArray[MaxDepth];
} Position;

extern int SquareFiles[64];
extern int SquareRanks[64];

extern char PieceChar[12];
extern char PromotedChar[];
extern int CharPiece[];
extern int PiecePawn[12];
extern int PieceKnight[12];
extern int PieceBishop[12];
extern int PieceRook[12];
extern int PieceQueen[12];
extern int PieceKing[12];
extern int PieceColour[12];

extern U64 PieceKeys[12][64];
extern U64 EnPassantKeys[64];
extern U64 CastlingKeys[16];
extern U64 SideKey;

extern U64 PawnMoves[2][64];
extern U64 PawnCaptures[2][64];
extern U64 KnightAttacks[64];
extern U64 KingAttacks[64];

extern void InitAttackTables();
extern void InitBitMasks();
extern void InitPieceSquareTables();

extern U64 GetBishopAttacks(int square, U64 occupancy);
extern U64 GetRookAttacks(int square, U64 occupancy);
extern U64 GetQueenAttacks(int square, U64 occupancy);

extern char* SquareString(int square);
extern char* MoveString(int move);
extern int ParseMove(char* move, const Position* position);
extern void PrintBoard(const Position* position);
extern void PrintBitboard(U64 bitboard);

extern void ParseFEN(char* fen, Position* position);
extern int SquareAttacked(int square, int side, const Position* position);

extern int MoveExists(int move, Position* position);
extern void GenerateMoves(const Position* position, MoveList* list);
extern void GenerateCaptures(const Position* position, MoveList* list);

extern int GetTimeMS();
extern void ReadInput(SearchInfo* info);

extern int MakeMove(int move, Position* position);
extern void TakeMove(Position* position);
extern void MakeNullMove(Position* position);
extern void TakeNullMove(Position* position);

extern void PerftTest(int depth, Position* position);

extern int Evaluate(const Position* position);

extern void ClearHashTable(HashTable* table);
extern void InitHashTable(HashTable* table, const int MB);
extern int GetHashEntry(int* pvMove, int alpha, int beta, int depth, Position* position);
extern void StoreHashEntry(int move, int score, int depth, int flag, Position* position);
extern int GetPvLength(const int depth, Position* position);

extern void Search(Position* position, SearchInfo* info);

extern void Init();

extern void UCILoop();

#endif