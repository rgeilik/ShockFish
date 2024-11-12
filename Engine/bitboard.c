#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"
#include <windows.h>
#include <string.h>
#include <time.h>

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

// Attack tables for leaper pieces
BBOARD pawn_attacks[2][BOARD_SIZE]; // Define pawn attack table
BBOARD knight_attacks[BOARD_SIZE]; // Define knight attack tables
BBOARD king_attacks[BOARD_SIZE]; // Define king attack tables


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

// Magic structures
SMagic mBishopTb1[64];
SMagic mRookTb1[64];

// Magic attack tables
BBOARD mRookAttacks[64][4096];
BBOARD mBishopAttacks[64][512];

// Zobrist keys
U64 piece_keys[12][64]; //piece_keys[piece][square]
U64 castling_keys[16];
U64 ep_keys[64];
U64 side_key;

// Position Zobrist hash key
U64 hash_key = 0ULL;

int numcaptures = 0;
int numep = 0;
int numcastles = 0;
int numpromotions = 0;





/*
|---------------------------------------------|
|                                             |
|           ZOBRIST HASHING AND KEYS          |
|                                             |
|---------------------------------------------|
*/


void gen_zobrist_keys() {
	random_state = 234893458; // Perhaps make this random later

	// Generate random U64 keys for every piece on every square
	for (int piece = P; piece <= k; piece++) {

		for (int square = 0; square < 64; square++) {
			piece_keys[piece][square] = get_random_U64_number();

		}
	}
	// Generate random U64 key for every possible castling rights combination 
	for (int castling = 0; castling < 16; castling++) {
		castling_keys[castling] = get_random_U64_number();
	}
	// Generate random U64 key for every possible en passant square 
	for (int ep = 0; ep < 16; ep++) {
		ep_keys[ep] = get_random_U64_number();
	}
	// Generate random U64 key for side
	side_key = get_random_U64_number();
}


U64 generate_hash() {
	U64 position_hash = 0ULL;

	// Perform Zobrist hashing on position hash for every piece present
	for (int piece = P; piece <= k; piece++) {
		BBOARD piece_bitboard = bitboards[piece];

		while (piece_bitboard) {
			int LSBPos = getLSB(piece_bitboard);
			position_hash ^= piece_keys[piece][LSBPos];
			pop_bit(piece_bitboard, LSBPos);
		}
	}

	// Perform Zobrist hashing on position for castling rights
	position_hash ^= castling_keys[castlingRights];

	// Perform Zobrist hashing for en passant
	if (ep != no_sq) {
		position_hash ^= ep_keys[ep];
	}

	// Perform Zobrist hashing on position if side is black
	if (sideToMove == black) { position_hash ^= side_key; }

	return position_hash;
}








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
	sideToMove = (fen[i] == 'w') ? white : black;
	

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

	// Generate hash key via Zobrist
	hash_key = generate_hash();

	
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

// GENERATE PAWN QUIET MOVES



// GENERATE KNIGHT ATTACK MASK
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



// Checks if square is attacked by a side
int is_square_attacked(int square, int side) {
	// Check for pawn attacks
	if ( (side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
	if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

	if ( (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) ) return 1;

	if (get_bishop_attacks(square, occupancy[both]) & ((side == white) ? bitboards[B] : bitboards[b]) ) return 1;

	if (get_rook_attacks(square, occupancy[both]) & ((side == white) ? bitboards[R] : bitboards[r]) )  return 1;

	if (get_queen_attacks(square, occupancy[both]) & ((side == white) ? bitboards[Q] : bitboards[q]) ) return 1;

	if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))  return 1;

	return 0;
}

// Get all pieces xraying a square
BBOARD findXrayPieces();

BBOARD xrayRookAttacks(int color, BBOARD occ, int rookSq) {
	
	BBOARD attacks = get_rook_attacks(rookSq, occ);
	
	BBOARD blockers = ((attacks & occ) & (bitboards[(color) ? R : r]) | bitboards[(color) ? Q : q]);
	
	return (attacks ^ get_rook_attacks(rookSq, occ ^ blockers));
}


BBOARD xrayBishopAttacks(int color, BBOARD occ, int bishopSq) {
	BBOARD attacks = get_bishop_attacks(bishopSq, occ);

	// Get piece blockers
	BBOARD pieceBlockers = ((attacks & occ) & (bitboards[(color) ? B : b] | bitboards[(color) ? Q : q]));

	// Get pawn blockers that are in front based on color
	BBOARD pawnBlockers = ((attacks & occ) & bitboards[(color ? P : p)]);
	if (color) { // white bishop
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL << (bishopSq + 1));
	}
	else { // black bishop
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL >> (64 - bishopSq));
	}

	// Get attacks through pieces
	BBOARD throughPieces = (attacks ^ get_bishop_attacks(bishopSq, occ ^ pieceBlockers));

	// Get attacks through pawns
	BBOARD throughPawns = 0ULL;
	while (pawnBlockers) {
		int pawnSq = getLSB(pawnBlockers);
		// Get bishop attacks as if the pawn wasn't there
		BBOARD withoutPawn = get_bishop_attacks(bishopSq, occ ^ (1ULL << pawnSq));
		// Only include the pawn's attack square if it's on the same diagonal
		throughPawns |= (pawn_attacks[color][pawnSq] & withoutPawn);
		pop_bit(pawnBlockers, pawnSq);
	}

	return throughPieces | throughPawns;
}


BBOARD xrayQueenAttacks(int color, BBOARD occ, int queenSq) {
	BBOARD attacks = get_queen_attacks(queenSq, occ);
	BBOARD bishopattacksonfly = bishop_attack_mask(queenSq, 0ULL);

	// Get piece blockers
	BBOARD pieceBlockers = ((attacks & occ) &
		((bitboards[(color) ? Q : q] |
			bitboards[(color) ? R : r] |
			bitboards[(color) ? B : b])));

	printf("Piece Blockers: \n\n");
	printBitboard(pieceBlockers);
	getchar();

	// Get pawn blockers that are in front
	BBOARD pawnBlockers = ((bishopattacksonfly & occ) & bitboards[(color ? P : p)]);
	if (color) {
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL << (queenSq + 1));
	}
	else {
		pawnBlockers &= ~(0xFFFFFFFFFFFFFFFFULL >> (64 - queenSq));
	}

	printf("Pawn Blockers: \n\n");
	printBitboard(pawnBlockers);
	getchar();

	// Get attacks through pieces
	BBOARD throughPieces = (attacks ^ get_queen_attacks(queenSq, occ ^ pieceBlockers));

	// Get attacks through pawns
	BBOARD throughPawns = 0ULL;
	while (pawnBlockers) {
		int pawnSq = getLSB(pawnBlockers); 
		printf("\n\nPawn on square: %s\n\n", index_to_coordinate[pawnSq]);
		getchar();
		// Get queen attacks as if the pawn wasn't there
		BBOARD withoutPawn = bishopattacksonfly;

		// Get the ray direction from queen to pawn
		int pawnFile = pawnSq % 8;
		int pawnRank = pawnSq / 8;
		int queenFile = queenSq % 8;
		int queenRank = queenSq / 8;

		// Calculate direction
		int fileDir = (pawnFile - queenFile) ? (pawnFile - queenFile) / abs(pawnFile - queenFile) : 0;
		int rankDir = (pawnRank - queenRank) ? (pawnRank - queenRank) / abs(pawnRank - queenRank) : 0;

		// Get pawn's attack squares
		BBOARD pawnAttacks = pawn_attacks[color][pawnSq];

		// Filter pawn attacks to only include the square that continues along the ray
		BBOARD validPawnAttacks = 0ULL;
		while (pawnAttacks) {
			int attackSq = getLSB(pawnAttacks);
			int attackFile = attackSq % 8;
			int attackRank = attackSq / 8;

			// Check if attack square continues along the same direction
			if ((attackFile - pawnFile) == fileDir && (attackRank - pawnRank) == rankDir) {
				validPawnAttacks |= (1ULL << attackSq);
			}
			pop_bit(pawnAttacks, attackSq);
		}

		throughPawns |= (validPawnAttacks & withoutPawn);
		pop_bit(pawnBlockers, pawnSq);
	}

	return throughPieces | throughPawns;
}

BBOARD attacksTo(BBOARD occupancy, int sq) {
	BBOARD knights, kings, bishopsQueens, rooksQueens;
	BBOARD attackers = 0ULL;

	knights = bitboards[N] | bitboards[n];
	kings = bitboards[K] | bitboards[k];
	rooksQueens = bitboards[Q] | bitboards[q];
	bishopsQueens = bitboards[Q] | bitboards[q];
	rooksQueens |= bitboards[R] | bitboards[r];
	bishopsQueens |= bitboards[B] | bitboards[b];

	// Get direct attackers
	attackers |= (pawn_attacks[white][sq] & bitboards[p])
		| (pawn_attacks[black][sq] & bitboards[P])
		| (knight_attacks[sq] & knights)
		| (king_attacks[sq] & kings)
		| (get_bishop_attacks(sq, occupancy) & bishopsQueens)
		| (get_rook_attacks(sq, occupancy) & rooksQueens);

	// Add x-raying pieces
	BBOARD whitePieces = bitboards[R] | bitboards[B] | bitboards[Q];
	BBOARD blackPieces = bitboards[r] | bitboards[b] | bitboards[q];

	// For each sliding piece
	while (whitePieces) {
		int piece_sq = getLSB(whitePieces);
		// If this piece x-rays through to our target square
		if ((get_bit(bitboards[R], piece_sq) || get_bit(bitboards[Q], piece_sq)) &&
			(get_bit(xrayRookAttacks(white, occupancy, piece_sq), sq))) {
			set_bit(attackers, piece_sq);
		}
		if ((get_bit(bitboards[B], piece_sq) || get_bit(bitboards[Q], piece_sq)) &&
			(get_bit(xrayBishopAttacks(white, occupancy, piece_sq), sq))) {
			set_bit(attackers, piece_sq);
		}
		pop_bit(whitePieces, piece_sq);
	}

	while (blackPieces) {
		int piece_sq = getLSB(blackPieces);
		if ((get_bit(bitboards[r], piece_sq) || get_bit(bitboards[q], piece_sq)) &&
			(get_bit(xrayRookAttacks(black, occupancy, piece_sq), sq))) {
			set_bit(attackers, piece_sq);
		}
		if ((get_bit(bitboards[b], piece_sq) || get_bit(bitboards[q], piece_sq)) &&
			(get_bit(xrayBishopAttacks(black, occupancy, piece_sq), sq))) {
			set_bit(attackers, piece_sq);
		}
		pop_bit(blackPieces, piece_sq);
	}

	return attackers;
}

/*
|---------------------------------------------|
|                                             |
|           HANDLING MOVES                    |
|                                             |
|---------------------------------------------|
*/


typedef struct {
	int moves[256];
	int moveCount;
} moveList;

// Function to add moves to move list
void add_move(moveList* list, int move) {
	list->moves[list->moveCount] = move;
	list->moveCount++;
}

// Function to print moves in algebraic notation
void print_move(int move) {
	if (!get_move_promotion(move)) {
		printf("%s%s", index_to_coordinate[get_move_source(move)], index_to_coordinate[get_move_target(move)]);
	}

	else {
		printf("%s%s%c", index_to_coordinate[get_move_source(move)], index_to_coordinate[get_move_target(move)], piece_promotions[get_move_promotion(move)]);
	}
}



// print move list
void printMoveList(moveList* move_list)
{
	// do nothing on empty move list
	if (!move_list->moveCount)
	{
		printf("\n     No move in the move list!\n");
		return;
	}

	printf("\n     move    piece     capture   double    enpassant    castling\n\n");

	// loop over moves within a move list
	for (int move_count = 0; move_count < move_list->moveCount; move_count++)
	{
		// init move
		int move = move_list->moves[move_count];
		
		// print move
		printf("      %s%s%c   %c         %d         %d         %d         %d\n", index_to_coordinate[get_move_source(move)],
			index_to_coordinate[get_move_target(move)],
			get_move_promotion(move) ? piece_promotions[get_move_promotion(move)] : ' ',
			ascii_pieces[get_move_piece(move)],
			get_move_capture(move) ? 1 : 0,
			get_move_double(move) ? 1 : 0,
			get_move_enpassant(move) ? 1 : 0,
			get_move_castling(move) ? 1 : 0); 

	}

	// print total number of moves
	printf("\n\n     Total number of moves: %d\n\n", move_list->moveCount);

}


// print board
void print_board()
{
	// print offset
	printf("\n");

	// loop over board ranks
	for (int rank = 0; rank < 8; rank++)
	{
		// loop ober board files
		for (int file = 0; file < 8; file++)
		{
			// init square
			int square = rank * 8 + file;

			// print ranks
			if (!file)
				printf("  %d ", 8 - rank);

			// define piece variable
			int piece = -1;

			// loop over all piece bitboards
			for (int bb_piece = P; bb_piece <= k; bb_piece++)
			{
				// if there is a piece on current square
				if (get_bit(bitboards[bb_piece], square))
					// get piece code
					piece = bb_piece;
			}

			// print different piece set depending on OS
			printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);

		}

		// print new line every rank
		printf("\n");
	}

	// print board files
	printf("\n     a b c d e f g h\n\n");

	// print side to move
	printf("     Side:     %s\n", !sideToMove ? "white" : "black");

	// print enpassant square
	printf("     Enpassant:   %s\n", (ep != no_sq) ? index_to_coordinate[ep] : "no");

	// print castling rights
	printf("     Castling:  %c%c%c%c\n\n", (castlingRights & wk) ? 'K' : '-',
		(castlingRights & wq) ? 'Q' : '-',
		(castlingRights & bk) ? 'k' : '-',
		(castlingRights & bq) ? 'q' : '-');

	
	// print hash key
	printf("     Hash key:  %llx\n\n", hash_key);
}





#define copy_position() \
BBOARD bitboards_copy[12], occupancy_copy[3]; \
int sideToMove_copy, ep_copy, castlingRights_copy; \
memcpy(bitboards_copy, bitboards, 96); \
memcpy(occupancy_copy, occupancy, 24); \
sideToMove_copy = sideToMove; ep_copy = ep; castlingRights_copy = castlingRights; \
U64 hash_key_copy = hash_key;

#define take_back() \
memcpy(bitboards, bitboards_copy, 96); \
memcpy(occupancy, occupancy_copy, 24); \
sideToMove = sideToMove_copy; ep = ep_copy; castlingRights = castlingRights_copy; \
hash_key = hash_key_copy;








int make_move(int move, int moveFlag) {

	if (moveFlag == all_moves) {
		copy_position();
		int source = get_move_source(move);
		int target = get_move_target(move);
		int piece = get_move_piece(move);
		int capture = get_move_capture(move);
		int promotion = get_move_promotion(move);
		int pawn_double = get_move_double(move);
		int enpassant = get_move_enpassant(move);
		int castling = get_move_castling(move);

		if (source == target) return 0;

		pop_bit(bitboards[piece], source);
		set_bit(bitboards[piece], target);

		hash_key ^= piece_keys[piece][source];
		hash_key ^= piece_keys[piece][target];

		// Handle castling (source and target will be king moving 2 squares)
		if (castling) {
			switch (target) {
			case(g1): pop_bit(bitboards[R], h1); set_bit(bitboards[R], f1); hash_key ^= piece_keys[R][h1]; hash_key ^= piece_keys[R][f1]; break;
			case(c1): pop_bit(bitboards[R], a1); set_bit(bitboards[R], d1); hash_key ^= piece_keys[R][a1]; hash_key ^= piece_keys[R][d1]; break;
			case(g8): pop_bit(bitboards[r], h8); set_bit(bitboards[r], f8); hash_key ^= piece_keys[r][h8]; hash_key ^= piece_keys[r][f8]; break;
			case(c8): pop_bit(bitboards[r], a8); set_bit(bitboards[r], d8); hash_key ^= piece_keys[r][a8]; hash_key ^= piece_keys[r][d8]; break;
			}
		}

		// Handle promotions
		if (promotion) {
			pop_bit(bitboards[piece], target); // Pop pawn that has moved to 1st or 8th rank to promote
			set_bit(bitboards[promotion], target); // Set piece that pawn is being promoted to

			// Remove pawn on target square from hash key and add promoted piece on targe square to hash key
			hash_key ^= piece_keys[piece][target];
			hash_key ^= piece_keys[promotion][target];
		}

		// Handle captures
		if (capture) {

			int startpiece = (sideToMove == white) ? p : P;
			int endpiece = (sideToMove == white) ? k : K;

			for (; startpiece <= endpiece; startpiece++) {
				// If piece found on target square, pop and remove from hash key
				if (get_bit(bitboards[startpiece], target)) {

					pop_bit(bitboards[startpiece], target);
					hash_key ^= piece_keys[startpiece][target];
					break;
				}
			}

			// Handle en passant
			if (enpassant) {

				if (sideToMove == white) {
					pop_bit(bitboards[p], target + 8);
					hash_key ^= piece_keys[p][target + 8];
					hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
				}

				else {
					pop_bit(bitboards[P], target - 8);
					hash_key ^= piece_keys[P][target - 8];
					hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
				}

			}
		}



		// TO DO: Possibly look into adding an additional ep_key for no_sq, which is equal to 0 (and add to ep_ranks)
		if (ep != no_sq) hash_key ^= ep_keys[ep_ranks[ep]];
		ep = no_sq;

		// Handle double pawn moves (last because of en passant removal problems)
		if (pawn_double) {
			switch (sideToMove) {

			case(white): ep = source - 8; hash_key ^= ep_keys[ep_ranks[source - 8]]; break;
			case(black): ep = source + 8; hash_key ^= ep_keys[ep_ranks[source + 8]]; break;

			}
		}

		hash_key ^= castling_keys[castlingRights]; // Undo previous move castling rights from hash key

		castlingRights &= castling_bits[source]; // Use castling bits array to create 4-bit flag for castling on current move
		castlingRights &= castling_bits[target]; // In case rook moved

		hash_key ^= castling_keys[castlingRights]; // Update hash key with current move castling rights

		memset(occupancy, 0ULL, 24);

		for (int whitePiece = P; whitePiece <= K; whitePiece++) {
			occupancy[white] |= bitboards[whitePiece];
			occupancy[both] |= bitboards[whitePiece];

		}

		for (int blackPiece = p; blackPiece <= k; blackPiece++) {
			occupancy[black] |= bitboards[blackPiece];
			occupancy[both] |= bitboards[blackPiece];
		}

		sideToMove ^= 1;
		hash_key ^= side_key; // Don't need to differ between white and black because XOR undoes itself

		if (is_square_attacked((sideToMove == white) ? getLSB(bitboards[k]) : getLSB(bitboards[K]), sideToMove)) {
			take_back();

			return 0;
		}

		// Increment counters for perft 
		if (capture) numcaptures++;
		if (promotion) numpromotions++;
		if (enpassant) numep++;
		if (castling) numcastles++;


		return 1;

	}

	// If only_captures is used
	else {
		// Check if move is indeed a capture
		if (get_move_capture(move)) {
			make_move(move, all_moves);
		}

		return 0; // Move is not a capture, return 0
	}
}


void generate_legal_moves(moveList* movelist) {

	// Generate pawn and castling moves for WHITE
	if (sideToMove == white) {
		BBOARD whitepawn_bitboard = bitboards[P];
		
		while (whitepawn_bitboard) {
			
			int source = getLSB(whitepawn_bitboard);
			int push = source - 8;
			int doublepush = source - 16;
			BBOARD captures = (pawn_attacks[white][source] & occupancy[black]);
			
			BBOARD en_passant = 0ULL;
			if (ep != no_sq) {
				set_bit(en_passant, ep);
			}
			
			// Check if square directly in front of pawn is available
			if (!get_bit(occupancy[both], push)) {
	
				// Check if pawn is on 7th rank to give promotion option
				if (promotion_ability[source] == 1) {

					for (int promote = N; promote < K; promote++) {
						add_move(movelist, encode_move(source, push, P, promote, 0, 0, 0, 0));
					}
				}

				// Otherwise do normal push
				else {

					add_move(movelist, encode_move(source, push, P, 0, 0, 0, 0, 0));

					// Check if pawn is on 2nd rank to give double push option
					if (!get_bit(occupancy[both], doublepush) && promotion_ability[source] == 2) { add_move(movelist, encode_move(source, doublepush, P, 0, 0, 1, 0, 0)); }
				}
			}


			// Check if pawn attacks and en passant square intersect
			if (pawn_attacks[white][source] & en_passant) {
				add_move(movelist, encode_move(source, ep, P, 0, 1, 0, 1, 0));
				
				pop_bit(en_passant, ep);
			}

			// Generate pawn captures
			while (captures) {
				
				int target = getLSB(captures);

				// Check if pawn is on 7th rank to give promotion option
				if (promotion_ability[source] == 1) {

					for (int promote = N; promote < K; promote++) {
						add_move(movelist, encode_move(source, target, P, promote, 1, 0, 0, 0));
						
					}
					
					pop_bit(captures, target);
				}

				// Gen normal captures
				else {
					add_move(movelist, encode_move(source, target, P, 0, 1, 0, 0, 0)); 
					
					pop_bit(captures, target);
				}
			}


			pop_bit(whitepawn_bitboard, source);
		}

		
		// Generate castling moves for kingside castling for white
		if (castlingRights & wk) {
			
			if (!get_bit(occupancy[both], f1) && !get_bit(occupancy[both], g1)) { 

				if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black)) {

					add_move(movelist, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
					
				}
			}
		}

		// Generate castling moves for queenside castling for white
		if (castlingRights & wq) {
			
			if (!get_bit(occupancy[both], d1) && !get_bit(occupancy[both], c1) && !get_bit(occupancy[both], b1)) {

				if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black)) {

					add_move(movelist, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
					
				}
			}
		}


	}




	// Generate pawn moves and castling for BLACK
	else {
		BBOARD blackpawn_bitboard = bitboards[p];
		
		while (blackpawn_bitboard) {

			int source = getLSB(blackpawn_bitboard);
			int push = source + 8;
			int doublepush = source + 16;
			BBOARD attacks = (pawn_attacks[black][source] & occupancy[white]);
			BBOARD en_passant = 0ULL;
			
			if (ep != no_sq) {
				set_bit(en_passant, ep);
			}

			// Check if square directly in front of pawn is available
			if (!get_bit(occupancy[both], push)) {
				// Check if pawn is on 2nd rank
				if (promotion_ability[source] == 2) {

					for (int promote = n; promote < k; promote++) {
						add_move(movelist, encode_move(source, push, p, promote, 0, 0, 0, 0));
						
					}
				}


				else {

					add_move(movelist, encode_move(source, push, p, 0, 0, 0, 0, 0));
					// Check if pawn is on 7th rank to give double push option
					if (!get_bit(occupancy[both], doublepush) && promotion_ability[source] == 1) { add_move(movelist, encode_move(source, doublepush, p, 0, 0, 1, 0, 0)); }
				}
			}



			// Check if pawn attacks and en passant square intersect
			if (pawn_attacks[black][source] & en_passant) {
				add_move(movelist, encode_move(source, ep, p, 0, 1, 0, 1, 0));
				
				pop_bit(en_passant, ep);
			}

			// Generate pawn captures
			while (attacks) {
				
				int target = getLSB(attacks);

				// Check if pawn is on 2nd rank and able to capture and promote
				if (promotion_ability[source] == 2) {

					for (int promote = n; promote < k; promote++) {
						add_move(movelist, encode_move(source, target, p, promote, 1, 0, 0, 0));
						
					}
					
					pop_bit(attacks, target);
				}
				
				else {
					add_move(movelist, encode_move(source, target, p, 0, 1, 0, 0, 0));
					
					pop_bit(attacks, target);
				}
			}


			pop_bit(blackpawn_bitboard, source);
		}



		
		

		// Generate castling moves for queenside and kingside castling for white
		if (castlingRights & bk) {

			if (!get_bit(occupancy[both], f8) && !get_bit(occupancy[both], g8)) {

				if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white) && !is_square_attacked(g8, white)) {

					add_move(movelist, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
					
				}
			}
		}

		if (castlingRights & bq) {

			if (!get_bit(occupancy[both], d8) && !get_bit(occupancy[both], c8) && !get_bit(occupancy[both], b8)) {

				if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white) && !is_square_attacked(c8, white)) {

					add_move(movelist, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
					
				}
			}
		}
	
	}


	/*-------GEN MOVES FOR EVERY OTHER PIECE----------*/

	// Generate moves for knights
	BBOARD knight_bitboard = (sideToMove == white) ? bitboards[N] : bitboards[n];

	while (knight_bitboard) {
		int source = getLSB(knight_bitboard);
		BBOARD captures = (knight_attacks[source] & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (knight_attacks[source] & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		
		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? N : n), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? N : n), 0, 1, 0, 0, 0));
			
			pop_bit(captures, target);
		}

		pop_bit(knight_bitboard, source);
	}


	// Generate bishop moves
	BBOARD bishop_bitboard = (sideToMove == white) ? bitboards[B] : bitboards[b];

	while (bishop_bitboard) {
		int source = getLSB(bishop_bitboard);
		BBOARD captures = (get_bishop_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_bishop_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? B : b), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? B : b), 0, 1, 0, 0, 0));
			
			pop_bit(captures, target);
		}

		pop_bit(bishop_bitboard, source);
	}

	// Generate rook moves
	BBOARD rook_bitboard = (sideToMove == white) ? bitboards[R] : bitboards[r];

	while (rook_bitboard) {
		int source = getLSB(rook_bitboard);
		BBOARD captures = (get_rook_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_rook_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? R : r), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? R : r), 0, 1, 0, 0, 0));
	
			pop_bit(captures, target);
		}

		pop_bit(rook_bitboard, source);
	}

	// Generate moves for queens
	BBOARD queen_bitboard = (sideToMove == white) ? bitboards[Q] : bitboards[q];

	while (queen_bitboard) {
		int source = getLSB(queen_bitboard);
		BBOARD captures = (get_queen_attacks(source, occupancy[both]) & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (get_queen_attacks(source, occupancy[both]) & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? Q : q), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? Q : q), 0, 1, 0, 0, 0));
			
			pop_bit(captures, target);
		}

		pop_bit(queen_bitboard, source);
	}

	// Generate moves for kings
	BBOARD king_bitboard = (sideToMove == white) ? bitboards[K] : bitboards[k];

	while (king_bitboard) {
		int source = getLSB(king_bitboard);
		BBOARD captures = (king_attacks[source] & occupancy[(sideToMove == white) ? black : white]);
		BBOARD quietMoves = (king_attacks[source] & ~occupancy[(sideToMove == white) ? white : black] & ~occupancy[(sideToMove == white) ? black : white]);

		while (quietMoves) {
			int target = getLSB(quietMoves);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? K : k), 0, 0, 0, 0, 0));
			pop_bit(quietMoves, target);
		}

		while (captures) {
			int target = getLSB(captures);
			add_move(movelist, encode_move(source, target, ((sideToMove == white) ? K : k), 0, 1, 0, 0, 0));
			
			pop_bit(captures, target);
		}

		pop_bit(king_bitboard, source);
	}

}


/*----------PERFT DEBUG FUNCTIONS-------------*/

U64 nodes;

void perft_driver(int depth) {
	if (depth == 0) {
		nodes++;
		return;
	}

	moveList move_list = { 0 };
	move_list.moveCount = 0;

	
	generate_legal_moves(&move_list);
	

	for (int move_count = 0; move_count < move_list.moveCount; move_count++) {
		copy_position();
		
		if (!make_move(move_list.moves[move_count], all_moves))
		{
			continue;
		}
		

		perft_driver(depth - 1);

		take_back();
	}
}


void perft_test(int depth)
{
	printf("\n     PERFT\n\n");
	
	
	moveList move_list = { 0 };
	move_list.moveCount = 0;
	


	// Call move generator
	generate_legal_moves(&move_list);
	
	
	 


	
	long start = get_elapsed_time_ms();

	// Loop over generated moves
	for (int move_count = 0; move_count < move_list.moveCount; move_count++)
	{
		
		copy_position();

		
		if (!make_move(move_list.moves[move_count], all_moves))
			
		{
			continue;
		}
	
		
		
		long cummulative_nodes = nodes;

		
		perft_driver(depth - 1);

		// Old nodes
		long old_nodes = nodes - cummulative_nodes;

		
		take_back();

		// Print Move
		printf("%s%s%c: %ld\n", index_to_coordinate[get_move_source(move_list.moves[move_count])],
			index_to_coordinate[get_move_target(move_list.moves[move_count])],
			get_move_promotion(move_list.moves[move_count]) ? piece_promotions[get_move_promotion(move_list.moves[move_count])] : ' ',
			old_nodes);
	}

	// Results
	printf("\n    Depth: %d\n", depth);
	printf("    Nodes: %lld\n", nodes);
	printf("       Number of Captures: %d\n", numcaptures);
	printf("       Number of Castles: %d\n", numcastles);
	printf("       Number of En Passant: %d\n", numep);
	printf("       Number of Promotions: %d\n", numpromotions);
	printf("     Time: %lu\n\n", get_elapsed_time_ms() - start);
	
}











/*
|---------------------------------------------|
|                                             |
|               EVALUATION                    |
|                                             |
|---------------------------------------------|
*/

// All values taken from Ronald Friedrich's PeSTO eval function
int mg_value[12] = { 82, 337, 365, 477, 1025, 0, 82, 337, 365, 477, 1025, 0 };
int eg_value[12] = { 94, 281, 297, 512, 936, 0, 94, 281, 297, 512, 936, 0 };

// PSQT
int mg_pawn_table[64] = {
	  0,   0,   0,   0,   0,   0,  0,   0,
	 98, 134,  61,  95,  68, 126, 34, -11,
	 -6,   7,  26,  31,  65,  56, 25, -20,
	-14,  13,   6,  21,  23,  12, 17, -23,
	-27,  -2,  -5,  12,  17,   6, 10, -25,
	-26,  -4,  -4, -10,   3,   3, 33, -12,
	-35,  -1, -20, -23, -15,  24, 38, -22,
	  0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	 94, 100,  85,  67,  56,  53,  82,  84,
	 32,  24,  13,   5,  -2,   4,  17,  17,
	 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	  4,   7,  -6,   1,   0,  -5,  -1,  -8,
	 13,   8,   8,  10,  13,   0,   2,  -7,
	  0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
	-167, -89, -34, -49,  61, -97, -15, -107,
	 -73, -41,  72,  36,  23,  62,   7,  -17,
	 -47,  60,  37,  65,  84, 129,  73,   44,
	  -9,  17,  19,  53,  37,  69,  18,   22,
	 -13,   4,  16,  13,  28,  19,  21,   -8,
	 -23,  -9,  12,  10,  19,  17,  25,  -16,
	 -29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	 -4,   5,  19,  50,  37,  37,   7,  -2,
	 -6,  13,  13,  26,  34,  12,  10,   4,
	  0,  15,  15,  15,  14,  27,  18,  10,
	  4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
	-14, -21, -11,  -8, -7,  -9, -17, -24,
	 -8,  -4,   7, -12, -3, -13,  -4, -14,
	  2,  -8,   0,  -1, -2,   6,   0,   4,
	 -3,   9,  12,   9, 14,  10,   3,   2,
	 -6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
	 32,  42,  32,  51, 63,  9,  31,  43,
	 27,  32,  58,  62, 80, 67,  26,  44,
	 -5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	 7,  7,  7,  5,  4,  -3,  -5,  -3,
	 4,  3, 13,  1,  2,   1,  -1,   2,
	 3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	 -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
	 -9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	  3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
	-65,  23,  16, -15, -56, -34,   2,  13,
	 29,  -1, -20,  -7,  -8,  -4, -38, -29,
	 -9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	  1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	 10,  17,  23,  15,  20,  45,  44,  13,
	 -8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};

int* mg_pesto_table[12] =
{    // White pieces
	mg_pawn_table,
	mg_knight_table,
	mg_bishop_table,
	mg_rook_table,
	mg_queen_table,
	mg_king_table,

	// Black pieces
	mg_pawn_table,
	mg_knight_table,
	mg_bishop_table,
	mg_rook_table,
	mg_queen_table,
	mg_king_table
};

int* eg_pesto_table[12] =
{   // White pieces
	eg_pawn_table,
	eg_knight_table,
	eg_bishop_table,
	eg_rook_table,
	eg_queen_table,
	eg_king_table,

	// Black pieces
	eg_pawn_table,
	eg_knight_table,
	eg_bishop_table,
	eg_rook_table,
	eg_queen_table,
	eg_king_table
};

int gamePhaseInc[12] = { 0,0,1,1,1,1,2,2,4,4,0,0 };
int mg_table[12][64]; // Piece value table for middlegame
int eg_table[12][64]; // Piece value table for endgame

void init_piece_values() {
	for (int startPiece = P; startPiece <= K; startPiece++) {

		for (int square = 0; square < 64; square++) {
			mg_table[startPiece][square] = mg_value[startPiece] + mg_pesto_table[startPiece][square];
			eg_table[startPiece][square] = eg_value[startPiece] + eg_pesto_table[startPiece][square];
		}
	}

	for (int startPiece = p; startPiece <= k; startPiece++) {

		for (int square = 0; square < 64; square++) {
			mg_table[startPiece][square] = mg_value[startPiece] + mg_pesto_table[startPiece][mirror(square)];
			eg_table[startPiece][square] = mg_value[startPiece] + mg_pesto_table[startPiece][mirror(square)];
		}
	}
}


int evaluate() {
	int mg_eval_side[2];
	int eg_eval_side[2];
	int gamePhase = 0;

	mg_eval_side[black] = 0;
	mg_eval_side[white] = 0;
	eg_eval_side[black] = 0;
	eg_eval_side[white] = 0;

	for (int startPiece = P; startPiece <= K; startPiece++) {
		BBOARD bitboard = bitboards[startPiece];

		while (bitboard) {
			int square = getLSB(bitboard);
			mg_eval_side[white] += mg_table[startPiece][square];
			eg_eval_side[white] += eg_table[startPiece][square];
			gamePhase += gamePhaseInc[startPiece];

			pop_bit(bitboard, square);
		}
	}

	for (int startPiece = p; startPiece <= k; startPiece++) {
		BBOARD bitboard = bitboards[startPiece];

		while (bitboard) {
			int square = getLSB(bitboard);
			mg_eval_side[black] += mg_table[startPiece][square];
			eg_eval_side[black] += eg_table[startPiece][square];
			gamePhase += gamePhaseInc[startPiece];

			pop_bit(bitboard, square);
		}
	}

	int mgScore = mg_eval_side[sideToMove] - mg_eval_side[((sideToMove) ^ 1)];
	int egScore = eg_eval_side[sideToMove] - mg_eval_side[((sideToMove) ^ 1)];
	int mgPhase = gamePhase;
	if (mgPhase > 24) mgPhase = 24; // Handle early promotions
	int egPhase = 24 - mgPhase;
	int score = (mgScore * mgPhase + egScore * egPhase) / 24;

	return score; // For NegaMax
}




int getLeastValuablePiece;



/*
|---------------------------------------------|
|                                             |
|               SEARCH FUNCTION               |
|                                             |
|---------------------------------------------|
*/

#define hash_table_size 800000

typedef struct {
	U64 zobrist;
	int depth;
	int eval;
	int bestmove;

} tt;

tt hash_table[hash_table_size];

// MVV_LVA[attacker][target]
int mvv_lva[12][12] = {
	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

int score_moves(int move) {
	if (get_move_capture(move)) {
		int targetpiece;

		int startpiece = (sideToMove == white) ? p : P;
		int endpiece = (sideToMove == white) ? k : K;

		for (; startpiece <= endpiece; startpiece++) {
			// If piece found on target square, pop and remove from hash key
			if (get_bit(bitboards[startpiece], get_move_target(move))) {

				targetpiece = startpiece;
				break;
			}
		}

		printf("Source piece: %c\n", ascii_pieces[get_move_piece(move)]);
		printf("Target piece: %c\n", ascii_pieces[targetpiece]);

		return mvv_lva[get_move_piece(move)][targetpiece];
	}
}


int quiescence(int alpha, int beta) {
	int stand_pat = evaluate();
	if (stand_pat >= beta) {
		return beta;
	}

	if (alpha < stand_pat) {
		alpha = stand_pat;
	}

	moveList every_move = (moveList){
		.moves = {0},
		.moveCount = 0
	};
	generate_legal_moves(&every_move);

	for (int move = 0; move < every_move.moveCount; move++) {
		nodes++;
		copy_position();

		if (!make_move(every_move.moves[move], only_captures)) {
			continue;
		}
		nodes--;
		int score = -quiescence(-beta, -alpha);

		take_back();

		if (score >= beta) {
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	return alpha;
}


int ply;
int bestMove;

int alphaBeta(int alpha, int beta, int depthleft) {
	int foundPV = 0;

	if (depthleft == 0) {
		nodes++;
		return quiescence(alpha, beta);
	}

	moveList move_list = {
		.moves = {0},
		.moveCount = 0
	};

	int is_king_in_check = is_square_attacked(
		(sideToMove == white) ? getLSB(bitboards[K]) : getLSB(bitboards[k]),
		sideToMove ^ 1
	);

	int legal_moves = 0;
	generate_legal_moves(&move_list);

	// Search moves
	for (int i = 0; i < move_list.moveCount; i++) {
		int score;
		copy_position();

		if (!make_move(move_list.moves[i], all_moves)) {
			continue;
		}

		ply++;
		legal_moves++;
		nodes++;

		if (foundPV) {
			score = -alphaBeta(-alpha - 1, -alpha, depthleft - 1);
			if ((score > alpha) && (score < beta)) {
				score = -alphaBeta(-beta, -alpha, depthleft - 1);
			}
		}

		else {
			score = -alphaBeta(-beta, -alpha, depthleft - 1);
		}

		take_back();
		ply--;

		// Beta cutoff
		if (score >= beta) {
			return beta;
		}

		// Found better move
		if (score > alpha) {
			alpha = score;
			//foundPV = 1;
			
			if (ply == 0) {
				bestMove = move_list.moves[i];
			}
		}
	}

	// Handle checkmate/stalemate
	if (legal_moves == 0) {
		if (is_king_in_check) {
			return -49000 + ply; // Checkmate
		}
		else {
			return 0; // Stalemate
		}
	}

	return alpha;
}


int search() {

}



/*
|---------------------------------------------|
|                                             |
|           UCI AND GUI HANDNLING             |
|                                             |
|---------------------------------------------|
*/




// Function to convert algebraic square notation (e.g. "a2") to board index
int square_to_index(char* square) {
	int file = square[0] - 'a';  // Convert a-h to 0-7
	int rank = 8 - (square[1] - '0');  // Convert 1-8 to 7-0
	return rank * 8 + file;
}

// Function to parse and play UCI move if it's legal
int parse_uci_move(const char* uci_move) {
	char source_sq[3] = { uci_move[0], uci_move[1], '\0' };
	char target_sq[3] = { uci_move[2], uci_move[3], '\0' };
	int promotion = 0;
	if (strlen(uci_move) == 5) {
		char promotion_piece[2] = { uci_move[4], '\0' };
		promotion = (sideToMove == white) ? (piece_promotions_mapback_white[promotion_piece[0]]) : (piece_promotions_mapback_black[promotion_piece[0]]);
	}

	int source = square_to_index(source_sq);
	
	int target = square_to_index(target_sq);
	

	moveList legal_moves = (moveList){
		.moves = { 0 },
		.moveCount = 0
	};

	generate_legal_moves(&legal_moves);

	for (int move = 0; move < legal_moves.moveCount; move++) {
		//printf("Source: %s\n", index_to_coordinate[get_move_source(legal_moves.moves[move])]);

		if (source == get_move_source(legal_moves.moves[move]) && target == get_move_target(legal_moves.moves[move]) && promotion == get_move_promotion(legal_moves.moves[move])) {

			if (make_move(legal_moves.moves[move], all_moves)) {
				//printf("Made move..\n");
				return 1;
			}

			// Handle illegal move
			else {
				//printf("Illegal move, puts king in check");
				return 0;
			}
		}
	}
	// Move not found in legal moves, return 0
	//printf("Move not found in move list");
	return 0;
}


int parse_uci_position(char* cmd) {
	// Reset the board state before processing new position
	memset(bitboards, 0, sizeof(bitboards));
	memset(occupancy, 0, sizeof(occupancy));
	ep = no_sq;
	castlingRights = 0;
	sideToMove = -1;
	hash_key = 0ULL;

	// Create a modifiable copy of the command string
	char cmd_copy[4096];
	strcpy_s(cmd_copy, sizeof(cmd_copy), cmd);
	char* current_cmd = cmd_copy;

	current_cmd += 9; // Shift from 'position' to 'fen' or 'startpos'

	// Handle startpos
	if (!strncmp(current_cmd, "startpos", 8)) {
		parseFen(starting_pos);
		current_cmd += 8; // Move past "startpos"
	}
	// Handle FEN
	else if (!strncmp(current_cmd, "fen", 3)) {
		current_cmd += 4; // Move past "fen "
		parseFen(current_cmd);
		// Move current_cmd to end of FEN string
		while (*current_cmd && *current_cmd != 'm') current_cmd++;
	}
	else {
		return 0; // Invalid command
	}

	// Handle moves if present
	char* moves = strstr(current_cmd, "moves");
	if (moves) {
		char* context = NULL;
		moves += 6; // Move past "moves "

		char* move = strtok_s(moves, " \n", &context);
		while (move) {
			if (!parse_uci_move(move)) {
				printf("info string Invalid move: %s\n", move);
				return 0;
			}
			move = strtok_s(NULL, " \n", &context);
		}
	}

	return 1;
}

void parse_uci_go(char* cmd) {
	char* current_cmd = cmd + 3; // Move from 'go' to next part

	if (!strncmp(current_cmd, "depth", 5)) {
		current_cmd += 6; // Shift from 'depth' to depth number
		int depth = atoi(current_cmd);
		alphaBeta(-inf, inf, depth);
		printf("bestmove ");
		print_move(bestMove);
		printf("\n");
	}

	else if (!strncmp(current_cmd, "perft", 5)) {
		current_cmd += 6; // Shift from 'perft' to depth number
		int depth = atoi(current_cmd);
		perft_test(depth);
	}

	else {
		int depth = 6;
		alphaBeta(-inf, inf, depth);
		printf("bestmove ");
		print_move(bestMove);
		printf("\n");
	}


}

void uci_loop() {
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	char input[2000];

	

	printf("id name ShockFish\n");
	printf("id author AtomicChessSensei\n");
	printf("uciok\n");

	while (1) {
		memset(input, 0, sizeof(input));

		fflush(stdout);

		if (!fgets(input, 2000, stdin)) {
			continue;
		}

		if (input[0] == '\n') {
			continue;
		}

		else if (!strncmp(input, "isready\n", 8)) {
			printf("readyok\n");
			continue;
		}

		else if (!strncmp(input, "position", 8)) {
			parse_uci_position(input);
			print_board();
		}

		else if (!strncmp(input, "ucinewgame\n", 11)) {
			parseFen(starting_pos);

		}

		else if (!strncmp(input, "go", 2)) {
			parse_uci_go(input);
			
		}

		else if (!strncmp(input, "board", 5)) {
			print_board();
		}

		else if (!strncmp(input, "uci\n", 4)) {
			printf("id name ShockFish\n");
			printf("id author AtomicChessSensei\n");
			printf("uciok\n");
			
		}

		else if (!strncmp(input, "quit\n", 5)) {
			break;
		}

		else {
			printf("Unknown command: %s", input);
		}
	}


}



void init_all() {
	gen_zobrist_keys();
	init_leaper_attacks();
	init_slider_attackTables(rook);
	init_slider_attackTables(bishop);
	init_piece_values();
}

int main() {
	// DO NOT COMMENT OUT INIT ALL
	init_all();

	int debug = 1;

	if (debug) {


		parseFen("1k6/8/8/5P2/4B3/3Q4/8/1K6 w - - 0 1");
		printBitboard(xrayQueenAttacks(white, occupancy[both], d3));
	}






	else {
		uci_loop();
	}
	
	

	/*
	parseFen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
	
	moveList test = (moveList){
		.moves = {0},
		.moveCount = 0
	};
	generate_legal_moves(&test);
	
	
	for (int move = 0; move < test.moveCount; move++) {
		
		
		print_move(test.moves[move]);
		
		printf("\n");
		printf("MVV_LV VALUE: %d\n", score_moves(test.moves[move]));
		getchar();
	} 
	*/
	

	return 0;
}






