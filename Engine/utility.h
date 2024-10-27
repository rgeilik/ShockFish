#pragma once
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

#define get_bit(bitboard, square) bitboard & (1ULL << square) // Define get bit macro
#define set_bit(bitboard, square) bitboard |= (1ULL << square) // Define set bit macro
#define pop_bit(bitboard, square) get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0 // Define pop bit macro

enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1

}; // Board is reversed because a1 = 1 is top left and h8 = 64

enum { black, white }; // Alias for sides, white = 1 and black = 0
enum { rook, bishop };

// Masks for file exclusions
extern const BBOARD not_A_file = 18374403900871474942ULL;
extern const BBOARD not_H_file = 9187201950435737471ULL;
extern const BBOARD not_AB_file = 18229723555195321596ULL;
extern const BBOARD not_HG_file = 4557430888798830399ULL;
extern const BBOARD not_edges = 35604928818740736ULL;
//extern const BBOARD not_1_rank = 72057594037927935ULL;
//extern const BBOARD not_8_rank = 18446744073709551360ULL;

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

