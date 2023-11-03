#include <stdio.h>
#include "Definitions.h"

void PrintAttackedSquares(int side, Position* position) {
    printf("\n");

    for (int rank = Rank8; rank >= Rank1; rank--) {
        for (int file = FileA; file <= FileH; file++) {
            int square = GetSquare(file, rank);

            if (SquareAttacked(square, side, position)) printf(" X");
            else printf(" .");
        }

        printf("\n");
    }

    printf("\n");
}

int main() {
    Init();

    printf("%s v%s\n\n", NAME, VERSION);

    UCILoop();

    return 0;
}