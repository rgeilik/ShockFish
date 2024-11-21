#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include "bitboard.h"
#include <windows.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "nnue_eval.h"

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

U64 repetition_table[10000];
int repetition_index = 0;

int numcaptures = 0;
int numep = 0;
int numcastles = 0;
int numpromotions = 0;


// Half move counter for search
int ply = 0;



// TIME MANAGEMENT
// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int _time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

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
		position_hash ^= ep_keys[ep_ranks[ep]];
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

	sideToMove = 0;
	ep = no_sq;
	castlingRights = 0;

	memset(bitboards, 0ULL, sizeof(bitboards));
	memset(occupancy, 0ULL, sizeof(occupancy));

	repetition_index = 0;
	memset(repetition_table, 0ULL, sizeof(repetition_table));

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



int getMajorAndMinors() {
	int numKnights = __popcnt64(bitboards[N] | bitboards[n]);
	int numBishops = __popcnt64(bitboards[B] | bitboards[b]);
	int numRooks = __popcnt64(bitboards[R] | bitboards[r]);
	int numQueens = __popcnt64(bitboards[Q] | bitboards[q]);

	return (numKnights + numBishops + numRooks + numQueens) == 0;
}


int get_captured_piece(int move) {
	int targetSq = get_move_target(move);
	for (int startPiece = (sideToMove) ? p : P; startPiece <= (sideToMove) ? k : K; startPiece++) {
		BBOARD bitboard = bitboards[startPiece];
		if (get_bit(bitboard, targetSq)) {
			return startPiece;
		}
	}
}



static int init = 0, pipe;
static HANDLE inh;
DWORD dw;
int wait_for_input() {
	if (!init)
	{
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);
		if (!pipe)
		{
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}

	if (pipe)
	{
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
		return dw;
	}

	else
	{
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? 0 : dw;
	}
}


// read GUI/user input
void read_input()
{
	// bytes to read holder
	int bytes;

	// GUI/user input
	char input[256] = "", * endc;

	// "listen" to STDIN
	if (wait_for_input())
	{
		// tell engine to stop calculating
		stopped = 1;

		// loop to read bytes from STDIN
		do
		{
			// read bytes from STDIN
			bytes = read(_fileno(stdin), input, 256);
		}

		// until bytes available
		while (bytes < 0);

		// searches for the first occurrence of '\n'
		endc = strchr(input, '\n');

		// if found new line set value at pointer to 0
		if (endc) *endc = 0;

		// if input is available
		if (strlen(input) > 0)
		{
			// match UCI "quit" command
			if (!strncmp(input, "quit", 4))
				// tell engine to terminate exacution    
				quit = 1;

			// // match UCI "stop" command
			else if (!strncmp(input, "stop", 4))
				// tell engine to terminate exacution
				quit = 1;
		}
	}
}

// a bridge function to interact between search and GUI input
static void communicate() {
	// if time is up break here
	if (timeset == 1 && get_elapsed_time_ms() > stoptime) {
		// tell engine to stop calculating
		stopped = 1;
	}

	// read GUI input
	read_input();
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
	
	int bitCount = __popcnt64(attackMask);  // Determine the number of set bits
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



BBOARD getFurthestBishopBlocker(int bishopSq, BBOARD occupancy) {
	BBOARD furthestBlockers = 0ULL;
	int bishopRank = bishopSq / 8;
	int bishopFile = bishopSq % 8;

	// Check all 4 diagonal directions
	const int directions[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} }; // {rank_dir, file_dir}

	for (int dir = 0; dir < 4; dir++) {
		int rank = bishopRank + directions[dir][0];
		int file = bishopFile + directions[dir][1];
		int furthestSq = -1;

		// Follow the diagonal until edge or blocker
		while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
			int sq = rank * 8 + file;
			if (occupancy & (1ULL << sq)) {
				furthestSq = sq; // Keep updating - last one will be furthest
			}
			rank += directions[dir][0];
			file += directions[dir][1];
		}

		// If we found a blocker in this direction, add it
		if (furthestSq != -1) {
			furthestBlockers |= (1ULL << furthestSq);
		}
	}

	return furthestBlockers;
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

	/*
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
	} */

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
					//hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
				}

				else {
					pop_bit(bitboards[P], target - 8);
					hash_key ^= piece_keys[P][target - 8];
					//hash_key ^= ep_keys[ep_ranks[target]]; // Remove en passant from hash
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
		
		else
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
int initialDepth = 0;

int move_history[100];
int history_count = 0;

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

		// Add move to history
		move_history[history_count++] = move_list.moves[move_count];
		
		
		printf("---------POSITION AFTER MOVE----------\n\n");
		printf("Move: ");
		printf("%d. ", (initialDepth - depth + 1));
		print_move(move_list.moves[move_count]);
		printf("\n\n");
		// Print move sequence at current node (before recursion)
		printf("Sequence at depth %d: ", depth);
		for (int i = 0; i < history_count; i++) {
			if (i > 0) printf(" ");
			if (i % 2 == 0) printf("%d.", (i / 2) + 1);
			print_move(move_history[i]);
		}
		printf("\n");
		print_board();
		printf("Updated hash key: %llx\n", hash_key);
		printf("Hash key from scratch: %llx\n\n", generate_hash());
		if (hash_key != generate_hash())
			getchar(); 

		//assert(hash_key == generate_hash());

		perft_driver(depth - 1);

		history_count--;
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
	printMoveList(&move_list);
	// Loop over generated moves
	for (int move_count = 0; move_count < move_list.moveCount; move_count++)
	{
		
		copy_position();

		
		if (!make_move(move_list.moves[move_count], all_moves))
		{
			continue;
		}
		
		// Add move to history
		move_history[history_count++] = move_list.moves[move_count];
		
		printf("---------POSITION AFTER MOVE----------\n\n");
		printf("Move: ");
		printf("%d. ", (initialDepth - depth + 1));
		print_move(move_list.moves[move_count]);
		printf("\n\n");
		// Print move sequence at current node (before recursion)
		printf("Sequence at depth %d: ", depth);
		for (int i = 0; i < history_count; i++) {
			if (i > 0) printf(" ");
			if (i % 2 == 0) printf("%d.", (i / 2) + 1);
			print_move(move_history[i]);
		}
		printf("\n");
		print_board();
		printf("Updated hash key: %llx\n", hash_key);
		printf("Hash key from scratch: %llx\n\n", generate_hash());
		if (hash_key != generate_hash())
			getchar();

			

		//assert(hash_key == generate_hash());
		
		long cummulative_nodes = nodes;

		
		perft_driver(depth - 1);

		// Old nodes
		long old_nodes = nodes - cummulative_nodes;

		history_count--;
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
int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

int nnue_squares[64] = {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

int nnue_sides[2] = { white, black };

void create_nnue_input(int* pieces, int* squares) {
	BBOARD bitboard;
	int piece, square;
	int index = 2;

	for (int startPiece = P; startPiece <= k; startPiece++) {
		bitboard = bitboards[startPiece];

		while (bitboard) {

			piece = startPiece;
			square = getLSB(bitboard);

			if (piece == K) {
				pieces[0] = nnue_pieces[K];
				squares[0] = nnue_squares[square];
			}

			else if (piece == k) {
				pieces[1] = nnue_pieces[k];
				squares[1] = nnue_squares[square];
			}

			else {
				pieces[index] = nnue_pieces[piece];
				squares[index] = nnue_squares[square];
				index++;
			}

			pop_bit(bitboard, square);
		}
	}
	pieces[index] = 0;
	squares[index] = 0;
}

int nnue_eval() {
	int pieces[33];
	int squares[33];

	create_nnue_input(pieces, squares);
	return (5/4 * evaluate_nnue(nnue_sides[sideToMove], pieces, squares));
}



// All values taken from Ronald Friedrich's PeSTO eval function
const int mg_value[12] = { 82, 337, 365, 477, 1025, 0, 82, 337, 365, 477, 1025, 0 };
const int eg_value[12] = { 94, 281, 297, 512, 936, 0, 94, 281, 297, 512, 936, 0 };

const int PASSED_PAWN_BONUS[8] = { 0, 10, 20, 30, 50, 70, 90, 200 }; // By rank
const int ISOLATED_PAWN_PENALTY = 15;
const int DOUBLED_PAWN_PENALTY = 10;
const int MOBILITY_BONUS[6] = { 0, 4, 4, 2, 1, 0 }; // N, B, R, Q
const int KING_SAFETY_PENALTY = 10; // Per missing pawn in shelter

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








/*
|---------------------------------------------|
|                                             |
|               SEARCH FUNCTION               |
|                                             |
|---------------------------------------------|
*/

#define hash_table_size 32000000
#define max_ply 64

#define ALLNODE 0
#define FAILHIGH 1
#define FAILOW 2
#define NO_HASH_ENTRY 100000

#define mate_score 49000
#define mate_detected 48000

int hash_entries = 0;

typedef struct {
	U64 zobrist;
	int depth;
	int eval;
	int type;
	int hash_move;

} tt;

tt hash_table[hash_table_size];

int pv_length[max_ply];
int pv_table[max_ply][max_ply];

void clear_hashtable() {

		// loop over TT elements
		for (int index = 0; index < hash_table_size; index++)
		{
			// reset TT inner fields
			hash_table[index].zobrist = 0;
			hash_table[index].depth = 0;
			hash_table[index].type = 0;
			hash_table[index].eval = 0;
			hash_table[index].hash_move = 0;
		}
}

/*
void init_hash_table(int sizemb) {
	
	int bytes = sizemb * 1024 * 1024;

	if (hash_table != NULL) {
		printf("Freeing cached table...");
		free(hash_table);
	}

	hash_entries = bytes / sizeof(tt);

	hash_table = (tt*)malloc(sizeof(tt) * hash_entries);

	clear_hashtable();
} */

int read_hash_entry(int alpha, int beta, int depth) {
	int index = hash_key % hash_table_size;

	tt* entry = &hash_table[index];

	if (entry->zobrist == hash_key) {

		if (entry->depth >= depth) {

			int score = entry->eval;

			if (score < -mate_score) score += ply;
			if (score > mate_score) score -= ply; // IMPORTANT FOR CHECKMATES

			/*
			printf("READING HASH ENTRY...\n");
			printf("Score: %d\n", score);
			printf("Depth: %d\n", entry->depth);
			printf("Hash Key: %llx\n", entry->zobrist);
			printf("Node type: %d\n\n", entry->type);*/


			if (entry->type == ALLNODE)
				return score;

			if (entry->type == FAILHIGH && score >= beta)
				return beta;

			if (entry->type == FAILOW && score <= alpha)
				return alpha;
		}
	}

		return NO_HASH_ENTRY;
}

void write_hash_entry(int score, int depth, int hash_flag, int hashmove) {

	int index = hash_key % hash_table_size;

	tt* entry = &hash_table[index];

	if (score < -mate_score) score -= ply;
	if (score > mate_score) score += ply;

	entry->zobrist = hash_key;
	entry->eval = score;
	entry->type = hash_flag;
	entry->depth = depth;
	entry->hash_move = hashmove;
	/*
	printf("WRITING HASH ENTRY...\n");
	printf("Score: %d\n", score);
	printf("Depth: %d\n", entry->depth);
	printf("Hash Key: %llx\n", entry->zobrist);
	printf("Node type: %d\n\n", entry->type); */
	

//#define ALLNODE 0
//#define FAILHIGH 1
//#define FAILOW 2
}

/*------------------------MOVE ORDERING--------------------------*/


// MVV_LVA[attacker][target]
/*
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
}; */

#define MVV_LVA_OFFSET (INT_MAX - 256) // Constants for move scoring
#define PV_OFFSET 65

int MvvLva[7][6] = {
	{15, 14, 13, 12, 11, 10}, // victim Pawn
	{25, 24, 23, 22, 21, 20}, // victim Knight
	{35, 34, 33, 32, 31, 30}, // victim Bishop
	{45, 44, 43, 42, 41, 40}, // victim Rook
	{55, 54, 53, 52, 51, 50}, // victim Queen

	{0, 0, 0, 0, 0, 0}, // victim King
	{0, 0, 0, 0, 0, 0}, // No piece
};

int killer_moves[2][max_ply]; // Define killer moves array
int history_moves[12][64]; // Define history moves array

int score_pv, follow_pv; // Define PV move scoring flags

// Futility margins (taken from Blunder)
int FutilityMargins[9] = {
	0,
	100, // depth 1
	160, // depth 2
	220, // depth 3
	280, // depth 4
	340, // depth 5
	400, // depth 6
	460, // depth 7
	520, // depth 8
};

// Late Move Pruning Margins (taken from Blunder)
int LateMovePruningMargins[6] = { 0, 8, 12, 16, 20, 24 };


BBOARD getLeastValuablePiece(BBOARD attadef, int side, int* piece) {
	for (int startPiece = p; startPiece <= k; startPiece++) {
		int offset = side * 6; // Black piece - offset = white piece
		BBOARD bitboard = (bitboards[startPiece - offset] & attadef);
		if (bitboard) {
			*piece = startPiece - offset;
			return bitboard & (~bitboard + 1);
		}
	
	}
	return 0ULL;
}


int see(int move) {
	int gain[32], d = 0;
	BBOARD pawns = (bitboards[P] | bitboards[p]);
	BBOARD verticals = bitboards[R] | bitboards[r] | bitboards[Q] | bitboards[q];
	BBOARD diagonals = bitboards[B] | bitboards[b] | bitboards[Q] | bitboards[q];
	BBOARD allsliders = verticals | diagonals | pawns;
	BBOARD fromSet = (1ULL << get_move_source(move));
	BBOARD occ = occupancy[both];
	BBOARD attadef = attacksTo(occ, get_move_target(move));
	int capturingPiece = get_move_piece(move);
	int side = sideToMove;

	if (capturingPiece == P && get_rank[get_move_target(move)] == 8) {
		int promotedPiece = get_move_promotion(move);
		gain[d] = abs(mg_value[promotedPiece]) + abs(mg_value[get_captured_piece(move)]) - abs(mg_value[P]);
	}

	else if (capturingPiece == p && get_rank[get_move_target(move)] == 1) {
		int promotedPiece = get_move_promotion(move);
		gain[d] = abs(mg_value[promotedPiece]) + abs(mg_value[get_captured_piece(move)]) - abs(mg_value[p]);
	}

	else {
		gain[d] = get_captured_piece(move) != -1 ? abs(mg_value[get_captured_piece(move)]) : 0;
	}

	if (get_move_enpassant(move)) {
		gain[d] = mg_value[P];
	}

	do {
		d++;
		gain[d] = abs(mg_value[capturingPiece]) - gain[d - 1];
		occ ^= fromSet;
		attadef ^= fromSet;

		if (fromSet & allsliders) {

			if (capturingPiece == R || capturingPiece == r || capturingPiece == Q || capturingPiece == q) {
				attadef |= (get_rook_attacks(get_move_target(move), occ) & verticals);
			}

			if (capturingPiece == P || capturingPiece == p || capturingPiece == B || capturingPiece == b || capturingPiece == Q || capturingPiece == q) {
				attadef |= (get_bishop_attacks(get_move_target(move), occ) & diagonals);
			}
			attadef ^= fromSet;
		}

		attadef &= occ;
		side ^= 1;
		fromSet = getLeastValuablePiece(attadef, side, &capturingPiece);

		if (capturingPiece == P && get_rank[get_move_target(move)] == 8) {
			capturingPiece = Q;
		}

		else if (capturingPiece == p && get_rank[get_move_target(move)] == 1) {
			capturingPiece = q;
		}

	} while (fromSet);

	while (--d) {
		gain[d - 1] = -max(-gain[d - 1], gain[d]);
	}

	return gain[0];
}


void enable_pv_scoring(moveList* move_list) {
	follow_pv = 0;

	for (int move = 0; move < move_list->moveCount; move++) {

		if (move_list->moves[move] == pv_table[0][ply]) {

			follow_pv = 1;
			score_pv = 1;
		}
	}
}

int compare_moves(const void* a, const void* b) {
	// Extract the move indices
	int moveA = *(int*)a;
	int moveB = *(int*)b;

	// Compute move scores for both moves
	int scoreA = score_move(moveA);  // Get the score for moveA
	int scoreB = score_move(moveB);  // Get the score for moveB

	// Return the comparison result (in descending order)
	return scoreB - scoreA;  // If scoreB > scoreA, return positive, so moveB comes first
}

int score_move(int move) {

	int index = hash_key % hash_table_size;
	tt* entry = &hash_table[index];

	// PV move scoring
	if (score_pv) {

		if (move == pv_table[0][ply]) {
			score_pv = 0;
			return 20000;
			//return (MVV_LVA_OFFSET + PV_OFFSET);
		}
	}

	

	else if (move == entry->hash_move) {
		return 19000;
	}

	// Order captures
	else if (get_move_capture(move)) {
		int piece = get_move_piece(move);
		int capturedPiece = get_captured_piece(move);
		
		//return (MVV_LVA_OFFSET + MvvLva[capturedPiece][piece]);


		
		if (mg_value[piece] >= mg_value[capturedPiece]) {
			
			int move_score = see(move);
			if (move_score > 0) { move_score += 10000; }
			return move_score;
		}

		// If piece captured has same or more value than capturing piece
		else {
			return (mg_value[capturedPiece] - mg_value[piece] + 10000);
			//return (MvvLva[capturedPiece][piece] + 10000);
		} 
	}

	// Order quiet moves
	else {
		
		if (killer_moves[0][ply] == move) {
			return 9000;
			//return (MVV_LVA_OFFSET - 10);
		}

		else if (killer_moves[1][ply] == move) {
			return 8000;
			//return (MVV_LVA_OFFSET - 20);
		}

		else {
			return history_moves[get_move_piece(move)][get_move_target(move)];
		} 
	}
	return 0;
}

void order_moves(moveList* move_list) {
	// Sort moves[] array by their move_score using qsort
	qsort(move_list->moves, move_list->moveCount, sizeof(int), compare_moves);

}

// print move scores
void print_move_scores(moveList* move_list)
{
	printf("     Move scores:\n\n");

	// loop over moves within a move list
	for (int count = 0; count < move_list->moveCount; count++)
	{
		printf("     move: ");
		print_move(move_list->moves[count]);
		printf(" score: %d\n", score_move(move_list->moves[count]));
	}
}



int is_repetition()
{

	for (int index = 0; index < repetition_index; index++) {

		if (repetition_table[index] == hash_key)
			return 1;
	}


	return 0;
}


int quiescence(int alpha, int beta) {
	
	if ((nodes & 2047) == 0) {
		communicate();
	}

	if (ply > max_ply - 1) { return evaluate(); }
	
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
	
	//printf("----------BOARD AT QUIESCENCE SEARCH---------\n");
	//print_board();
	//printf("\n\n\n");
	generate_legal_moves(&every_move);
	order_moves(&every_move);
	//print_move_scores(&every_move);
	//getchar();
	for (int move = 0; move < every_move.moveCount; move++) {
		
	
		copy_position();
		int see_score = 0;
		ply++;

		repetition_index++;
		repetition_table[repetition_index] = hash_key;
		
		if (get_move_capture(every_move.moves[move])) {
			see_score = see(every_move.moves[move]);
			//printf("SEE score: %d\n", see_score);
			//printf("CAPTURE FLAG AT SEE SCORE: %d\n", (get_move_capture(every_move.moves[move])) ? 1 : 0);
			if (see_score < 0) {
				//printf("BAD CAPTURE, SKIPPING..\n\n");
				ply--;
				repetition_index--;
				continue;
			}
		} 

		if (!make_move(every_move.moves[move], only_captures)) {
			//printf("CAPTURE FLAG: %d\n", (get_move_capture(every_move.moves[move])) ? 1 : 0);
			//printf("NOT A CAPTURE, SKIPPING..\n\n");
			ply--;
			repetition_index--;
			continue;
			
		}

		nodes++;

		//printf("Good capture found...\n");

		int score = -quiescence(-beta, -alpha);

		ply--;
		repetition_index--;
		take_back();

		if (stopped) return 0;

		if (score >= beta) {
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	return alpha;
}


const int reduction_limit = 3;
const int full_depth_moves = 4;


int alphaBeta(int alpha, int beta, int depthleft) {

	if (ply > max_ply - 1) { return evaluate(); }

	int foundPV = 0;
	int score = 0;
	
	int node_type = FAILOW; // Define node type for storing entries in transposition tables
	int canFutilityPrune = 0;




	if (ply && is_repetition())
		return 0;

	int is_pv_node = beta - alpha > 1;

	// Mate Distance Pruning
	if (ply != 0) {

		alpha = max(alpha, -mate_score + ply);
		beta = min(beta, mate_score - ply - 1);
		if (alpha >= beta) {
			return alpha;
		}
	}

	
	if (ply && (score = read_hash_entry(alpha, beta, depthleft)) != NO_HASH_ENTRY && !is_pv_node) {
		
		return score;
	}  


	if ((nodes & 2047) == 0) {
		communicate();
	}

	// PV length initialisation
	pv_length[ply] = ply;

	int is_king_in_check = is_square_attacked(
		(sideToMove == white) ? getLSB(bitboards[K]) : getLSB(bitboards[k]),
		sideToMove ^ 1);

	if (is_king_in_check) { depthleft++; }

	if (depthleft == 0) {
		nodes++;
		return quiescence(alpha, beta);
	}

	

	moveList move_list = {
		.moves = {0},
		.moveCount = 0
	};

	nodes++;

	

	int legal_moves = 0;

	
	int evaluation = evaluate();


	/*--------------STATIC NULL MOVE PRUNING--------------*/
	if (!is_king_in_check && !is_pv_node && abs(beta) < mate_detected) {
		int scoreMargin = 85 * depthleft;
		if ((evaluation - scoreMargin) >= beta) {
			return (evaluation - scoreMargin);
		}
	} 


	

	/*-----------NULL MOVE PRUNING-------------*/
	if (depthleft >= 3 && !is_king_in_check && evaluation >= beta && !getMajorAndMinors()) {
		copy_position();
		ply++;

		repetition_index++;
		repetition_table[repetition_index] = hash_key;

		if (ep != no_sq) hash_key ^= ep_keys[ep];
		ep = no_sq;

		sideToMove ^= 1;
		hash_key ^= side_key;

		score = -alphaBeta(-beta, -beta + 1, depthleft - 1 - 2);

		ply--;
		repetition_index--;
		take_back();

		if (stopped) return 0;

		if (score >= beta && abs(score) < mate_detected) {
			return beta;
		}
	} 
	
	
	/*-------------RAZORING---------------- 
	if (depthleft <= 2 && !is_pv_node && !is_king_in_check) {
		int staticEval = evaluate();
		//printf("static eval + fut margins = %d\n", staticEval + (FutilityMargins[depthleft] * 3));
		//printf("Alpha: %d\n", alpha);
		
		if (staticEval + (FutilityMargins[depthleft] * 3) < alpha) {
			score = quiescence(alpha, beta);
			if (score < alpha) {
				//printf("Returning alpha!!!!\n\n");
				//getchar();
				return alpha;
			}
		}
	} */

	
	// Razoring v2
	if (!is_pv_node && !is_king_in_check && depthleft <= 2) {
		int static_eval = evaluate();
		int razor_margin = FutilityMargins[depthleft];

		// Only razor if evaluation isn't too far from alpha
		if (static_eval < alpha - razor_margin) {
			int q_score = quiescence(alpha - razor_margin, alpha);
			if (q_score < alpha) {
				return q_score;
			}
		}
	} 


	/*--------FUTILITY PRUNING CONDITIONS------------*/
	if (depthleft <= 8 && !is_pv_node && !is_king_in_check && alpha < mate_detected && beta < mate_detected) {
		int staticScore = evaluate();
		int margin = FutilityMargins[depthleft];
		canFutilityPrune = (staticScore + margin) <= alpha;
	}


	generate_legal_moves(&move_list);

	
	if (follow_pv) {
		enable_pv_scoring(&move_list);
	} 

	order_moves(&move_list);

	int searched_moves = 0; // For late move reductions

	// Search moves
	for (int i = 0; i < move_list.moveCount; i++) {
		
		copy_position();
		ply++;
		
		repetition_index++;
		repetition_table[repetition_index] = hash_key;

		if (!make_move(move_list.moves[i], all_moves)) {
			ply--;
			repetition_index--;
			continue;
		}

		int in_check = is_square_attacked(
			(sideToMove == white) ? getLSB(bitboards[K]) : getLSB(bitboards[k]),
			sideToMove ^ 1);

		legal_moves++;
		
		
		// LATE MOVE PRUNING
		if (depthleft <= 5 && !is_pv_node && !is_king_in_check && legal_moves > LateMovePruningMargins[depthleft])
		{
			int is_tactical = in_check || get_move_promotion(move_list.moves[i]);
			if (!is_tactical) {
				ply--;
				repetition_index--;
				take_back();
				continue;
			}
		} 


		// Futility prune
		if (canFutilityPrune && legal_moves > 1 && !in_check
			&& !get_move_promotion(move_list.moves[i])
			&& !get_move_capture(move_list.moves[i])) 
		{
			ply--;
			repetition_index--;
			take_back();
			continue;
		} 




		// Do full search if no moves searched
		if (searched_moves == 0) {
			score = -alphaBeta(-beta, -alpha, depthleft - 1);
		}


		/*---------LATE MOVE REDUCTIONS------------*/
		else {

			if (searched_moves >= full_depth_moves && depthleft >= reduction_limit &&
				!is_king_in_check && !get_move_capture(move_list.moves[i]) && !get_move_promotion(move_list.moves[i])) {
				score = -alphaBeta(-alpha - 1, -alpha, depthleft - 3);
			}

			else score = alpha + 1; // Full depth search


			if (score > alpha) {

				score = -alphaBeta(-alpha - 1, -alpha, depthleft - 1); // Null window search

				if ((score > alpha) && (score < beta)) {
					score = -alphaBeta(-beta, -alpha, depthleft - 1); // Re-search
				}
			}
		} 


		/*
		if (foundPV) {
			score = -alphaBeta(-alpha - 1, -alpha, depthleft - 1);
			if ((score > alpha) && (score < beta)) {
				score = -alphaBeta(-beta, -alpha, depthleft - 1);
			}
		}

		else {
			score = -alphaBeta(-beta, -alpha, depthleft - 1);
		}   */

		ply--;
		repetition_index--;
		take_back();
		
		if (stopped) return 0;

		searched_moves++;

		// Beta cutoff
		if (score >= beta) {

			// TT entry write
			write_hash_entry(beta, depthleft, FAILHIGH, move_list.moves[i]);

			// Make sure to only add on quiet moves
			if (!get_move_capture(move_list.moves[i])) {
				// Add killer moves
				killer_moves[1][ply] = killer_moves[0][ply];
				killer_moves[0][ply] = move_list.moves[i];
			}
			return beta;
			
		}

		// Found better move
		if (score > alpha) {

			node_type = ALLNODE;

			if (!get_move_capture(move_list.moves[i])) {
				// Store history moves
				history_moves[get_move_piece(move_list.moves[i])][get_move_target(move_list.moves[i])] += (depthleft * depthleft + depthleft - 1);
				
			}

			alpha = score;
			//foundPV = 1;

			// Write move to TT as hash move
			write_hash_entry(alpha, depthleft, node_type, move_list.moves[i]);


			// Write move to PV
			pv_table[ply][ply] = move_list.moves[i];

			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			pv_length[ply] = pv_length[ply + 1];
			
			
		}
	}

	// Handle checkmate/stalemate
	if (legal_moves == 0) {
		if (is_king_in_check) {
			return -mate_score + ply; // Checkmate
		}
		else {
			return 0; // Stalemate
		}
	}

	

	if (node_type == FAILOW) {
		write_hash_entry(alpha, depthleft, node_type, 0);
	}
	

	return alpha;
}






void search(int depth) {
	int score = 0;
	int bestScore = -inf;
	int bestMoveFound = 0;  // Track if we have a valid best move

	follow_pv = 0;
	score_pv = 0;
	stopped = 0;
	

	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_length, 0, sizeof(pv_length));
	memset(pv_table, 0, sizeof(pv_table));

	int savedBestMove = 0;  // Store the best move from completed iterations
	int time_extension = 0;

	int alpha = -inf;
	int beta = inf;

	for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
		if (stopped) break;

		// Only update best move if search completed
		if (!stopped) {
			savedBestMove = pv_table[0][0];
			bestMoveFound = 1;
		}

		follow_pv = 1;

		score = alphaBeta(alpha, beta, currentDepth);

		
		if ((score <= alpha) || (score >= beta)) {
			alpha = -inf;
			beta = inf;
			currentDepth--;

			if (currentDepth >= 6 && !time_extension) {
				_time *= (13 / 10);
				time_extension = 1;
			}

			continue;
		}

		alpha = score - 30;
		beta = score + 30; 
		

		if (score > mate_detected) {
			int mateInN = (mate_score - abs(score)) / 2 + 1;
			printf("MATE IN %d\n", mateInN);
			printf("info score mate %d depth %d nodes %ld pv ", mateInN, currentDepth, nodes);
		}

		else if (score < -mate_detected) {
			int mateInN = -(mate_score - abs(score)) / 2;
			printf("info score mate %d depth %d nodes %ld pv ", mateInN, currentDepth, nodes);
		}

		else {
			printf("info score cp %d depth %d nodes %ld pv ", score, currentDepth, nodes);
		}

		for (int move = 0; move < pv_length[0]; move++) {
			print_move(pv_table[0][move]);
			printf(" ");
		}
		printf("\n");
	}

	printf("bestmove ");
	// Use the last completely searched move
	print_move(bestMoveFound ? savedBestMove : pv_table[0][0]);
	printf("\n");
}



/*
|---------------------------------------------|
|                                             |
|           UCI AND GUI HANDNLING             |
|                                             |
|---------------------------------------------|
*/

void reset_time_control()
{
	
	quit = 0;
	movestogo = 30;
	movetime = -1;
	_time = -1;
	inc = 0;
	starttime = 0;
	stoptime = 0;
	timeset = 0;
	stopped = 0;
}


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

	memset(repetition_table, 0ULL, sizeof(repetition_table));

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

			repetition_index++;
			repetition_table[repetition_index] = hash_key;

			if (!parse_uci_move(move)) {
				printf("info string Invalid move: %s\n", move);
				return 0;
			}
			move = strtok_s(NULL, " \n", &context);
		}
	}

	return 1;
}

#define MOVE_OVERHEAD 200

void parse_uci_go(char* cmd) {
	reset_time_control();

	int depth = -1;

	char* current_cmd = cmd + 3; // Move from 'go' to next part

	// infinite search
	if ((current_cmd = strstr(cmd, "infinite"))) {}

	// match UCI "binc" cmd
	if ((current_cmd = strstr(cmd, "binc")) && sideToMove == black)
		// parse black time increment
		inc = atoi(current_cmd + 5);

	// match UCI "winc" cmd
	if ((current_cmd = strstr(cmd, "winc")) && sideToMove == white)
		// parse white time increment
		inc = atoi(current_cmd + 5);

	// match UCI "wtime" cmd
	if ((current_cmd = strstr(cmd, "wtime")) && sideToMove == white)
		// parse white time limit
		_time = atoi(current_cmd + 6);

	// match UCI "btime" cmd
	if ((current_cmd = strstr(cmd, "btime")) && sideToMove == black)
		// parse black time limit
		_time = atoi(current_cmd + 6);

	// match UCI "movestogo" cmd
	if ((current_cmd = strstr(cmd, "movestogo")))
		// parse number of moves to go
		movestogo = atoi(current_cmd + 10);

	// match UCI "movetime" cmd
	if ((current_cmd = strstr(cmd, "movetime")))
		// parse amount of time allowed to spend to make a move
		movetime = atoi(current_cmd + 9);

	// match UCI "depth" cmd
	if ((current_cmd = strstr(cmd, "depth")))
		// parse search depth
		depth = atoi(current_cmd + 6);

	

	// if move time is available
	if (movetime != -1)
	{
		// set time equal to move time
		_time = movetime;

		// set moves to go to 1
		movestogo = 1;
	}

	// init start time
	starttime = get_elapsed_time_ms();

	// init search depth
	depth = depth;

	// if time control is available
	if (_time != -1)
	{
		// flag we're playing with time control
		timeset = 1;

		// set up timing
		_time = (_time / movestogo);

		// disable time buffer when time is almost up
		if (_time > 1500) _time -= MOVE_OVERHEAD;

		// init stoptime
		stoptime = (starttime + _time + (inc - MOVE_OVERHEAD));

		// treat increment as seconds per move when time is almost up
		if (_time < 1500 && inc && depth == 64) stoptime = starttime + (inc - MOVE_OVERHEAD) + 200;
	}

	// if depth is not available
	if (depth == -1)
		// set depth to 64 plies (takes ages to complete...)
		depth = 64;

	// print debug info
	printf("time: %d  start: %u  stop: %u  depth: %d  timeset:%d\n",
		_time, starttime, stoptime, depth, timeset);

	// search position
	search(depth);
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
			clear_hashtable();
		}

		else if (!strncmp(input, "ucinewgame\n", 11)) {
			parseFen(starting_pos);

			clear_hashtable();
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
	
	init_leaper_attacks();
	init_slider_attackTables(rook);
	init_slider_attackTables(bishop);
	gen_zobrist_keys();
	//init_hash_table(20);
	clear_hashtable();
	init_piece_values();
	init_nnue("nn-6b4236f2ec01.nnue");
	
}

int main() {
	// DO NOT COMMENT OUT INIT ALL
	init_all();

	int debug = 1;

	if (debug) {
		
		parseFen(kiwipete);
		printf("-------INITIAL POSITION-----------\n");
		print_board();
		printf("\n\n\n");
		search(12);
	}



	else {
		uci_loop();
	}

	return 0;
}






