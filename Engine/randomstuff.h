/* // Declare dynamic arrays
	BBOARD** mRookAttacks;
	BBOARD** mBishopAttacks; 

	// Allocate memory
	const int NUM_SQUARES = 64;
	const int NUM_PATTERNS_ROOK = 4096;
	const int NUM_PATTERNS_BISHOP = 512;

	// Allocate memory for rook attacks
	mRookAttacks = malloc(NUM_SQUARES * sizeof(BBOARD*));
	for (int i = 0; i < NUM_SQUARES; i++) {
		mRookAttacks[i] = malloc(NUM_PATTERNS_ROOK * sizeof(BBOARD));
	}

	// Allocate memory for bishop attacks
	mBishopAttacks = malloc(NUM_SQUARES * sizeof(BBOARD*));
	for (int i = 0; i < NUM_SQUARES; i++) {
		mBishopAttacks[i] = malloc(NUM_PATTERNS_BISHOP * sizeof(BBOARD));
	}
	

	int seenMagicIndexes[4096] = { 0 };


	BBOARD* rookPatterns;
	BBOARD* bishopPatterns;
	for (int square = 0; square < 64; square++) {
		rookPatterns = gen_blocker_patterns(rook_relevant_occupancy(square));
		bishopPatterns = gen_blocker_patterns(bishop_relevant_occupancy(square));
		
		for (int pattern = 0; pattern < (1 << rook_relevant_bitCount[square]); pattern++) {
			int magicIndex = (int)((rookPatterns[pattern] * magicNumsRook[square]) >> (64 - 12));
			printf("\n\n\nMagic Index (Bishop): %d            ", magicIndex);
			printf("Square: %d          ", square);
			//printf("Pattern: %d\n\n", pattern);
			mRookAttacks[square][magicIndex] = rook_attack_mask(square, rookPatterns[pattern]);
			printf("Iteration %d\n", pattern);
			printBitboard(mRookAttacks[square][magicIndex]);
			
			if (seenMagicIndexes[magicIndex]) { printf("WARNING: MAGIC INDEX %d HAS ALREADY BEEN SEEN", magicIndex); getchar(); }
			else { seenMagicIndexes[magicIndex] = 1; }
		} 
		for (int k = 0; k < 4096; k++) { seenMagicIndexes[k] = 0; }
		printf("seenMagicIndexes has been reset!");
		getchar();
		

	} */

