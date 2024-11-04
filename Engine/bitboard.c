#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"
#include <windows.h>
#include <string.h>

/*
|---------------------------------------------|
|                                             |
|           IMPORTANT DEFINITIONS             |
|                                             |
|---------------------------------------------| 
*/

// CORE BITBOARDS AND FLAGS
BBOARD bitboards[12];
BBOARD occupancy[3];
int sideToMove = -1; // No side to move (illegal)
int ep = no_sq; // En Passant square (illegal)
int castlingRights;


extern BBOARD pawn_attacks[2][BOARD_SIZE]; // Define pawn attack table
extern BBOARD knight_attacks[BOARD_SIZE]; // Define knight attack tables
extern BBOARD king_attacks[BOARD_SIZE]; // Define king attack tables


BBOARD magicNumsRook[64] = {
	0x80002450814000,
	0x1004008900300140,
	0x2404052410200008,
	0xA10007058000280,
	0x40040800120001,
	0x200020000830410,
	0x2B00088000490402,
	0x8100009100002042,
	0x2010400340126008,
	0x400400040096410,
	0x244080068088400,
	0x98008108004020,
	0x4004201003005012,
	0x800400124020002,
	0x8000A48042000408,
	0x128316010050,
	0x8042408001611280,
	0x480102000200800,
	0x304081800248008,
	0x810500800090200,
	0x96100100802440,
	0x4000048002000080,
	0x211006000800100,
	0x2000530880,
	0x2022006200210010,
	0x8048002920080800,
	0x8064014082008800,
	0x1000A28008100008,
	0xC40A1080082400,
	0x8000080100088445,
	0xC48044080040119,
	0x4004002008014114,
	0x180004010100620,
	0xC0602082814C0030,
	0x90042408000A0081,
	0x1108100024501002,
	0x4042011040040080,
	0x9102004010500,
	0x100C481008600080,
	0x4000980108,
	0x40010800A5000,
	0x180220213003,
	0x60800C008403100,
	0x1490000486001801,
	0x80020119220001,
	0xA12088100200210,
	0x8010008028062,
	0x200801140100,
	0x50C004080190028,
	0x30401400820402,
	0x104C02410000810,
	0x8110005900044021,
	0x400010015800490,
	0x5404040100080100,
	0x4003008024420008,
	0x1000401012E4200,
	0x22030038804122,
	0x60D0008203082,
	0xC042000081129,
	0x8001200AC2001002,
	0x1CA00041012,
	0x101000C0008020B,
	0x600108004120363,
	0xC8240041002082

};

BBOARD magicNumsBishop[64] = {
	0xA020008180803880,
	0x80C010046000202,
	0x8005042002100,
	0xA01082010000,
	0x120210041002,
	0x24100084120080,
	0x24C8051124061290,
	0x20041248040208,
	0x180200214004810,
	0xB4200152004421,
	0x5180004350210004,
	0x203040408020,
	0x80404708000440,
	0x48200A110029046,
	0x4001009160280A00,
	0x40009148050209,
	0x400408200420C200,
	0x88083842022220,
	0x200480411140005,
	0x280840606020006,
	0x2400880400200406,
	0x104820890080400,
	0x8002020241008882,
	0x30A8000C000801,
	0x2404188803081280,
	0x1000010954C1,
	0x118080001420004,
	0x2080004004008,
	0x4840010802000,
	0x8000410020500220,
	0x40288158086A0,
	0x406B00302102400,
	0x802911002400024,
	0x4040822008880440,
	0x1000180040080080,
	0x400020080080081,
	0x4040400001100,
	0x400202100009000,
	0x808214400800,
	0x4101580072480,
	0x4101006C800080,
	0x920D040100860400,
	0x1000522802000128,
	0x2800048090400060,
	0x80040404100012,
	0x11402A04403081,
	0x8004080A00200,
	0x1404C82204210008,
	0x50040500200,
	0x400100404228002,
	0x80200204280A8002,
	0x400000212802820,
	0x8000004010212000,
	0x20040420A021680,
	0x2000600808948500,
	0x150208C0200C20,
	0xA8090080102240A,
	0x2002024089D00800,
	0x2200000C88045400,
	0x400020822020203,
	0x412001000C008808,
	0x41100A980100C250,
	0x80880408818,
	0x800422408402018
};

typedef struct {
	BBOARD mask;
	BBOARD magic;
} SMagic;


SMagic mBishopTb1[64];
SMagic mRookTb1[64];

BBOARD mRookAttacks[64][4096];
BBOARD mBishopAttacks[64][512];



/*
|----------------------------------------|
|                                        |
|           HELPER FUNCTIONS             |
|                                        |
|----------------------------------------|
*/



// Function to count set bits in bitboard
static inline int count_bits(BBOARD bitboard) {
	int count = 0;

	while (bitboard) {
		count++;
		bitboard &= bitboard - 1;
	}

	return count;
}

/* static int NumberOfSetBits(unsigned long long i)
{
	i = i - ((i >> 1) & 0x5555555555555555UL);
	i = (i & 0x3333333333333333UL) + ((i >> 2) & 0x3333333333333333UL);
	return (int)(((i + (i >> 4)) & 0xF0F0F0F0F0F0F0FUL) * 0x101010101010101UL >> 56);
} */


// FEN PARSER
void parseFen(char* fen) {
	int i = 0;
	int exitLoops = 0;

	// Parse piece positions and update piece bitboards
	for (int rank = 0; rank < 8; rank++) {

		for (int file = 0; file < 8; file++) {

			int square = (rank * 8) + file;

			if (ascii_to_code[fen[i]] != 0 || fen[i] == 'P') {
				int piece = ascii_to_code[fen[i]];
				set_bit(bitboards[piece], square);
				i++;
			}

			else if (isdigit(fen[i])) {
				char* endptr;
				int skip = (int)strtol(fen + i, &endptr, 10);
				file += (skip - 1);
				i++;
			}

			else if (fen[i] == '/') {
				i++;
				file--;
			}

			else { exitLoops = 1; break; }
		}

		if (exitLoops) break;

	}
	//Move to side to move field
	i++;
	// Parse side to move
	int sideToMove = (fen[i] == 'w') ? white : black;

	// Move to castling rights field
	i += 2;

	// Parse Castling Rights field
	while (fen[i] != ' ') {

		switch (fen[i])
		{
		case 'K': castlingRights |= wk; break;
		case 'Q': castlingRights |= wq; break;
		case 'k': castlingRights |= bk; break;
		case 'q': castlingRights |= bq; break;
		}

		i++;
	}

	// Move to en passant field
	i++;

	if (fen[i] != '-') {
		int rank = 8 - (fen[i + 1] - '0');
		int file = fen[i] - 'a';
		ep = (rank * 8) + file;
		printf("\nEn Passant square: %d\n", ep);
		//i += 3;
	}

	else { ep = no_sq; /*i += 2; */ }


	// Populate occupancy bitboards
	for (int piece = P; piece <= K; piece++) {
		occupancy[white] |= bitboards[piece];
	}

	for (int piece = p; piece <= k; piece++) {
		occupancy[black] |= bitboards[piece];
	}

	occupancy[both] |= occupancy[white];
	occupancy[both] |= occupancy[black];

	//TO DO: Generate hash key via Zobrist
}










/*
|---------------------------------------------|
|                                             |
|           LEAPER PIECES ATTACKS             |
|                                             |
|---------------------------------------------| 
*/

//GENERATE PAWN ATTACK MASK
BBOARD pawn_attack_mask(int color, int square) {
	BBOARD attacks = 0ULL; // Pawn attack mask bitboard
	
	BBOARD pawn = 0ULL; // Pawn position bitboard
	set_bit(pawn, square); // Set bit at pawn position

	if (color) {
		if (pawn >> 7 & not_A_file) { attacks |= (pawn >> 7); }
		if (pawn >> 9 & not_H_file) { attacks |= (pawn >> 9); }
	
	}

	else {
		if (pawn << 7 & not_H_file) { attacks |= (pawn << 7); }
		if (pawn << 9 & not_A_file) { attacks |= (pawn << 9); }

	}
	

	return attacks;

}

//GENERATE KNIGHT ATTACK MASK
BBOARD knight_attack_mask(int square) {
	BBOARD attacks = 0ULL;

	BBOARD knight = 0ULL;
	set_bit(knight, square);

		if (knight >> 17 & not_H_file) { attacks |= (knight >> 17); }
		if (knight >> 15 & not_A_file) { attacks |= (knight >> 15); }
		if (knight >> 10 & not_HG_file) { attacks |= (knight >> 10); }
		if (knight >> 6 & not_AB_file) { attacks |= (knight >> 6); }

		if (knight << 17 & not_A_file) { attacks |= (knight << 17); }
		if (knight << 15 & not_H_file) { attacks |= (knight << 15); }
		if (knight << 10 & not_AB_file) { attacks |= (knight << 10); }
		if (knight << 6 & not_HG_file) { attacks |= (knight << 6); }

		return attacks;


}

//GENERATE KING ATTACK MASK
BBOARD king_attack_mask(int square) {
	BBOARD attacks = 0ULL;

	BBOARD king = 0ULL;
	set_bit(king, square);

	if (king << 9 & not_A_file) { attacks |= (king << 9); }
	attacks |= (king << 8);
	if (king << 7 & not_H_file) { attacks |= (king << 7); }
	if (king << 1 & not_A_file) { attacks |= (king << 1); }

	if (king >> 9 & not_H_file) { attacks |= (king >> 9); }
	attacks |= (king >> 8);
	if (king >> 7 & not_A_file) { attacks |= (king >> 7); }
	if (king >> 1 & not_H_file) { attacks |= (king >> 1); }
	
	return attacks;
}

// INITIALISE ATTACKS FOR "LEAPER" PIECES (PAWNS, KNIGHTS AND KING)
void init_leaper_attacks() {
	
	for (int square = 0; square < BOARD_SIZE; square++) {
		//Init pawn attacks table
		pawn_attacks[white][square] = pawn_attack_mask(white, square);
		pawn_attacks[black][square] = pawn_attack_mask(black, square);

		//Init knight attacks table
		knight_attacks[square] = knight_attack_mask(square);

		//Init king attacks table
		king_attacks[square] = king_attack_mask(square);
		
	}
	
}




/*
|---------------------------------------------|
|                                             |
|           SLIDER PIECES ATTACKS             |
|                                             |
|---------------------------------------------| 
*/

// GENERATE BISHOP RELEVANT OCCUPANCY MASK
BBOARD bishop_relevant_occupancy(int square) {

	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;
	
	for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++) { attacks |= (1ULL << (rank * 8 + file));	 }
	
	for (rank = targetRank - 1, file = targetFile + 1; rank >= 1 && file <= 6; rank--, file++) { attacks |= (1ULL << (rank * 8 + file)); } 

	for (rank = targetRank - 1, file = targetFile - 1; rank >= 1 && file >= 1; rank--, file--) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file >= 1; rank++, file--) { attacks |= (1ULL << (rank * 8 + file));}

	return attacks; 
}

// GENERATE BISHOP ATTACK MASK BASED ON OCCUPANCY
BBOARD bishop_attack_mask(int square, BBOARD blockers) {
	BBOARD attacks = 0ULL;

	int rank, file;

	int targetRank = square / 8;
	int targetFile = square % 8;


	for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	return attacks;
}

// GENERATE ROOK RELEVANT OCCUPANCY MASK
BBOARD rook_relevant_occupancy(int square) {
	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;

	for (rank = targetRank, file = targetFile + 1; file <= 6; file++) { attacks |= (1ULL << (rank * 8 + file));	}

	for (rank = targetRank, file = targetFile - 1; file >= 1; file--) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank + 1, file = targetFile; rank <= 6; rank++) { attacks |= (1ULL << (rank * 8 + file));}

	for (rank = targetRank - 1, file = targetFile; rank >= 1; rank--) { attacks |= (1ULL << (rank * 8 + file));}

	return attacks;
}

// GENERATE ROOK ATTACK MASK BASE ON OCCUPANCY
BBOARD rook_attack_mask(int square, BBOARD blockers) {
	BBOARD attacks = 0ULL;

	int rank, file;
	int targetRank = square / 8;
	int targetFile = square % 8;

	for (rank = targetRank, file = targetFile + 1; file <= 7; file++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank, file = targetFile - 1; file >= 0; file--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank + 1, file = targetFile; rank <= 7; rank++) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	for (rank = targetRank - 1, file = targetFile; rank >= 0; rank--) {
		attacks |= (1ULL << (rank * 8 + file));
		if ((1ULL << (rank * 8 + file)) & blockers) break;
	}

	return attacks;
}

//Relevant occupancy bit precalculated generator
void print_occupancy_bits(int piece) {
	if (!piece) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				int square = (rank * 8) + file;
				printf("%d, ", count_bits(rook_relevant_occupancy(square)));
			}
			printf("\n");
		}
	}

	if (piece) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				int square = (rank * 8) + file;
				printf("%d, ", count_bits(bishop_relevant_occupancy(square)));
			}
			printf("\n");
		}
	}
}





//GENERATE ALL POSSIBLE BLOCKER ARRANGEMENTS 
BBOARD* gen_blocker_patterns(BBOARD attackMask) {
	
	int bitCount = count_bits(attackMask);  // Determine the number of set bits
	//printf("Number of non-zero bits: %d\n", bitCount);

	int bitIndexes[16];
	// Initialise bitIndexes dynamic array
	/* int* bitIndexes = (int*)malloc(bitCount * sizeof(int));
	if (bitIndexes == NULL) {
		printf("Memory allocation failed for bitIndexes!\n");
		return NULL;
	} */
	
	int counter = 0;

	// Store indices of set bits in bitIndexes array
	for (int i = 0; i < BOARD_SIZE; i++) {
		if (attackMask & (1ULL << i)) { bitIndexes[counter++] = i; }
	}

	// Calculate number of possible blocker patterns for piece and square
	//printf("Number of non-zero elements in bitIndexes: %d\n", bitCount);
	int numPatterns = 1ULL << bitCount;
	//printf("Expected blocker patterns: %d\n", numPatterns);

	
	 BBOARD* blockerPatterns = (BBOARD*)malloc(numPatterns * sizeof(BBOARD));
	if (blockerPatterns == NULL) {
		printf("Memory allocation failed!\n");
		return 0ULL; // Handle memory allocation failure
	} 
	
	for (int pattern = 0; pattern < numPatterns; pattern++) {
		blockerPatterns[pattern] = 0ULL;
		for (int bitIndex = 0; bitIndex < bitCount; bitIndex++) {
			int bit = (pattern >> bitIndex) & 1;
			blockerPatterns[pattern] |= ((BBOARD)bit << bitIndexes[bitIndex]);
		}
	}

	/*
	// Loop through blockerPatterns and print entries
	 for (int i = 0; i < numPatterns; i++) {
		printBitboard(blockerPatterns[i]); 
		printf("Blocker arrangement ID: %d \n\n\n", i);
		
	} */
	
	return blockerPatterns; 
}







// INITIALISE ROOK AND BISHOP ATTACK TABLES AND MAGICS
void init_slider_attackTables(int piece) {

	BBOARD* rookPatterns;
	BBOARD* bishopPatterns;

	
	/*
	// Allocate memory for rook attacks
	mRookAttacks = malloc(BOARD_SIZE * sizeof(BBOARD*));
	for (int i = 0; i < BOARD_SIZE; i++) {
		mRookAttacks[i] = malloc(4096 * sizeof(BBOARD));
	}

	// Allocate memory for bishop attacks
	mBishopAttacks = malloc(BOARD_SIZE * sizeof(BBOARD*));
	for (int i = 0; i < BOARD_SIZE; i++) {
		mBishopAttacks[i] = malloc(512 * sizeof(BBOARD));
	} */
	
	//Initialise bishop and rook magic attack tables
	for (int square = 0; square < 64; square++) {
		rookPatterns = gen_blocker_patterns(rook_relevant_occupancy(square));
		bishopPatterns = gen_blocker_patterns(bishop_relevant_occupancy(square));
		
		

		

		// Initialise rook attack tables
		if (!piece) {
			for (int pattern = 0; pattern < (1 << rook_relevant_bitCount[square]); pattern++) {
				int magicIndex = (int)((rookPatterns[pattern] * magicNumsRook[square]) >> (64 - 12));
				mRookAttacks[square][magicIndex] = rook_attack_mask(square, rookPatterns[pattern]);
				//printBitboard(mRookAttacks[square][magicIndex]);
				//getchar();
				// Initialise Rook Magic Struct
				mRookTb1[square] = (SMagic){
					.mask = rook_relevant_occupancy(square),
					.magic = magicNumsRook[square]
				};
				
			}
		}

		// Initialise bishop attack tables
		if (piece) {
			for (int pattern = 0; pattern < (1 << bishop_relevant_bitCount[square]); pattern++) {
				int magicIndex = (int)((bishopPatterns[pattern] * magicNumsBishop[square]) >> (64 - 9));
				//printf("Magic Index: %d\n\n", magicIndex);
				//printf("Blocker pattern: \n");
				//printBitboard(bishopPatterns[pattern]);
				//getchar();
				mBishopAttacks[square][magicIndex] = bishop_attack_mask(square, bishopPatterns[pattern]);
				//printf("Attack Pattern: \n");
				//printBitboard(mBishopAttacks[square][magicIndex]);
				//getchar();
;				// Initialise Bishop Magic Struct
				mBishopTb1[square] = (SMagic){
					.mask = bishop_relevant_occupancy(square),
					.magic = magicNumsBishop[square]
				};
				
			}
		}

	}


}



// GET BISHOP, ROOK AND QUEEN ATTACKS USING MAGIC NUMBERS
BBOARD get_bishop_attacks(int square, BBOARD occupancy) {
	occupancy &= mBishopTb1[square].mask;
	occupancy *= mBishopTb1[square].magic;
	occupancy >>= (64 - 9);
	return mBishopAttacks[square][occupancy];
}

BBOARD get_rook_attacks(int square, BBOARD occupancy) {
	occupancy &= mRookTb1[square].mask;
	occupancy *= mRookTb1[square].magic;
	occupancy >>= (64 - 12);
	return mRookAttacks[square][occupancy];
}

BBOARD get_queen_attacks(int square, BBOARD occupancy) {
	BBOARD rookAttacks = get_rook_attacks(square, occupancy);
	BBOARD bishopAttacks = get_bishop_attacks(square, occupancy);

	return (rookAttacks |= bishopAttacks);
}




int is_square_attacked(int square, int side) {
	// Check for pawn attacks
	if ( (side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

	if ( (knight_attacks[square] & (bitboards[n] | bitboards[N])) ) return 1;

	if (get_bishop_attacks(square, occupancy[both]) & (bitboards[b] | bitboards[B]) ) return 1;

	if (get_rook_attacks(square, occupancy[both]) & (bitboards[r] | bitboards[R]) ) return 1;

	if (get_queen_attacks(square, occupancy[both]) & (bitboards[q] | bitboards[Q]) ) return 1;

	if (king_attacks[square] & (bitboards[k] | bitboards[K]) ) return 1;

	return 0;
}







int main() {
	
	init_leaper_attacks(); 

	
	init_slider_attackTables(rook);
	init_slider_attackTables(bishop);
	BBOARD test = 0ULL;
	printBitboard(test);
	
	
	
	
	return 0;
}