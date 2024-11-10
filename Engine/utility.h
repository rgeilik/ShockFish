#pragma once
#include <windows.h>
/*TO DO:
 - Implement pawn move gen
 - Implement kngiht move gen
 - Implement Rook and Bishop move gen (Magic bitboards)


*/

/*CODE FOR GENERATING NOT FILE/RANK MASKS:
	BBOARD mask = 0ULL;
	for (int rank = 0; rank < 8; rank++) {

		for (int file = 0; file < 8; file++) {
			int square = (rank * 8) + file;
			if (file < 6) {
				set_bit(mask, square);
			}
		}
	}

*/

typedef unsigned long long BBOARD; // typedef for bitboard type

#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square))) // Define get bit macro - checks if bit at square position is set
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square))) // Define set bit macro
#define pop_bit(bitboard, square) get_bit(bitboard, square) ? ((bitboard) ^= (1ULL << (square))) : 0 // Define pop bit macro
#define BOARD_SIZE 64
#define starting_pos "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"



/* Bitboard Print Function */
void printBitboard(BBOARD bitboard) {

	for (int rank = 0; rank < 8; rank++) {

		printf("  %d   ", 8 - rank); // Print ranks

		for (int file = 0; file < 8; file++) {

			int square = (rank * 8) + file;

			printf(" %d ", get_bit(bitboard, square) ? 1 : 0);
		}

		printf("\n");
	}
	printf("\n       a  b  c  d  e  f  g  h  "); // Print files
	printf("\n\n Bitboard: %llud\n", bitboard);
}

// get time in milliseconds
int get_elapsed_time_ms()
{
	return GetTickCount();
}



