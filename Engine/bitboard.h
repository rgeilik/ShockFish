#pragma once
#include "utility.h"
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


#define USE_32_BIT_MULTIPLICATIONS

typedef unsigned long long BBOARD; // typedef for bitboard type
typedef unsigned long long U64; // typedef for U64 type for better readability

typedef struct {
	BBOARD  state;
} prng_state;

BBOARD pawn_attacks[2][64]; // Declare pawn attack table
BBOARD knight_attacks[64];   // Declare knight attack tables
BBOARD king_attacks[64];     // Declare king attack tables

BBOARD pawn_attack_mask(int color, int square);
BBOARD knight_attack_mask(int square);
BBOARD king_attack_mask(int square);
void init_leaper_attacks(); // Function to initialize leaper attacks
void printBitboard(BBOARD bitboard); // Declare print function if you need to use it in test.c




enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_sq

}; // Board is reversed because a1 = 1 is top left and h8 = 64

enum { black, white, both }; // Alias for sides, white = 1 and black = 0
enum { rook, bishop }; // Alias for slider pieces, rook = 0 and bishop = 1
enum { P, N, B, R, Q, K, p, n, b, r, q, k }; // Piece encoding  
enum { wk = 1, wq = 2, bk = 4, bq = 8 }; // Castling rights encoding in the form of 4-bit flag 0000



const char* index_to_coordinate[] = {
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

// ASCII pieces
const char ascii_pieces[12] = { "PNBRQKpnbrqk" };

const char piece_promotions[12] = {
	[N] = 'n',
	[B] = 'b',
	[R] = 'r',
	[Q] = 'q',
	[K] = 'k',
	[n] = 'n',
	[b] = 'b',
	[r] = 'r',
	[q] = 'q',
	[k] = 'k'
};

// Encode ASCII chars back to int values
int ascii_to_code[128] = {
	['P'] = P,['N'] = N,['B'] = B,['R'] = R,
	['Q'] = Q,['K'] = K,['p'] = p,['n'] = n,
	['b'] = b,['r'] = r,['q'] = q,['k'] = k
};



// Masks for file exclusions
extern const BBOARD not_A_file = 18374403900871474942ULL;
extern const BBOARD not_H_file = 9187201950435737471ULL;
extern const BBOARD not_AB_file = 18229723555195321596ULL;
extern const BBOARD not_HG_file = 4557430888798830399ULL;
extern const BBOARD not_edges = 35604928818740736ULL;
//extern const BBOARD not_1_rank = 72057594037927935ULL;
//extern const BBOARD not_8_rank = 18446744073709551360ULL;


// Number of relevant occupancy bits for each square for the Rook
const int rook_relevant_bitCount[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

// Number of relevant occupancy bits for each square for the Bishop
const int bishop_relevant_bitCount[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

const int promotion_ability[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0
};


// Precomputed ranks for en passant
const int ep_ranks[65] = {
	8, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	0, 1, 2, 3, 4, 5, 6, 7,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16,
	16,
};

// Precomputed values to help calculate castling rights flag
const int castling_bits[64] = {
	 7, 15, 15, 15,  3, 15, 15, 11,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	13, 15, 15, 15, 12, 15, 15, 14
};




// GET LSB FROM BITBOARD
const int index64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};


int getLSB(BBOARD bb) {
	const BBOARD debruijn64 = 0x03f79d71b4cb0a89;
	assert(bb != 0);
	return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}




/*
|-------------------------------------|
|                                     |
|           MAGIC NUMBERS             |
|                                     |
|-------------------------------------| 
*/

random_state = 1785933498;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number()
{
	// get current state
	unsigned int number = random_state;

	// XOR shift algorithm
	number ^= number << 13;
	number ^= number >> 17;
	number ^= number << 5;

	// update random number state
	random_state = number;

	// return random number
	return number;
}

// generate 64-bit pseudo legal numbers
BBOARD get_random_U64_number()
{
	// define 4 random numbers
	BBOARD n1, n2, n3, n4;

	// init random numbers slicing 16 bits from MS1B side
	n1 = (BBOARD)(get_random_U32_number()) & 0xFFFF;
	n2 = (BBOARD)(get_random_U32_number()) & 0xFFFF;
	n3 = (BBOARD)(get_random_U32_number()) & 0xFFFF;
	n4 = (BBOARD)(get_random_U32_number()) & 0xFFFF;

	// return random number
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
BBOARD generate_magic_candidate()
{
	return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}






























// Transform blockers and magic number into index
int transform(BBOARD blockers, BBOARD magic, int piece) {
	return (int)((blockers * magic) >> (64 - (piece ? 9 : 12)));
}


/*
// MAGIC NUMBER GENERATOR
BBOARD generate_magics(int square, int relevantBits, int piece) {

	BBOARD mask = piece ? bishop_relevant_occupancy(square) : rook_relevant_occupancy(square);
	BBOARD blockers[4096];
	BBOARD attackMask[4096]; // Attack mask after applying blockers
	BBOARD used[4096]; // Array to store used magic index - attackMask pairs
	BBOARD* generatedPatterns = gen_blocker_patterns(mask);

	// Check if the output is not NULL before copying
	if (generatedPatterns != NULL) {
		for (int i = 0; i < (1 << relevantBits); i++) {
			blockers[i] = generatedPatterns[i];
			//printBitboard(blockers[i]); Blockers[] works fine
			attackMask[i] = piece ? bishop_attack_mask(square, blockers[i]) : rook_attack_mask(square, blockers[i]);
			//printBitboard(attackMask[i]); attackMask[] also works fine

		}

		for (int k = 0; k < 100000000; k++) {

			BBOARD magic = generate_magic_candidate();
			if (count_bits((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
			//printf("Magic Number being tested: 0x%llX \n\n", magic);
			//getchar();

			memset(used, 0ULL, sizeof(used));
			//printf("\n\n%d\n\n", used[100]); //used is set to 0 properly

			int fail = 0;
			for (int j = 0; j < (1 << relevantBits); j++) { // i is pattern index, fail is flag for magic number validation

				//printf("Blockers bitboard: \n\n");
				//printBitboard(blockers[j]);
				int index = transform(blockers[j], magic, piece); // Magic Index calculation
				//printf("Magic Index Tested: %d\n\n", index);

				if (used[index] == 0ULL) {

					used[index] = attackMask[j];
					//printf("Index has not been used yet: setting to \n"); 
					//printBitboard(attackMask[j]);

				} // If entry in used[] corresponding to the magic Index is empty, set it to th relevant attackMask

				else if (used[index] != attackMask[j]) { fail = 1; printf("\n\nINVALID COLLISION\n"); break; } // If entry in used[] corresponding to the magic index is not empty and isn't the 

			}
			//printf("\nFail is equal to %d\n", fail);
			//getchar();
			if (!fail) {
				//printf("\n\n\nMAGIC NUMBER FOUND FOR %s: %llu\n\n\n", index_to_coordinate[square], magic); 
				return magic;
			}
		}

		printf("\n\n********FAILED*******\n\n");
		return 0ULL;
	}

	else {
		printf("Error generating blocker patterns!\n");
	}

	return 0ULL;
} */
 


