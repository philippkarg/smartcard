#include "aes.h"

uint8_t AES::mRCs[ROUNDS] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

// **********************************************************************************
// Public Methods *******************************************************************
// **********************************************************************************
AES::AES(const aes_key_t masterKey)
{
    // If MASKING is defined, create key schedule for mOriginalSubKeys
    #ifdef MASKING
    createKeySchedule(masterKey, mOriginalSubKeys);
    #else
    createKeySchedule(masterKey, mSubkeys);
    #endif
}

void AES::decrypt(uint8_t *cipher)
{
    state_t state = {};
    uint8_t inByte = 0;
    // Read cipher column by column
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            state[row][col] = cipher[inByte++];

    // Init Masking *****************************************************************
    #ifdef MASKING
    // Init the masks
    mMasking.init();
    // Mask the keys
    mMasking.maskSubKeys(mOriginalSubKeys, mSubkeys);
    // Mask the State
    mMasking.invMaskState(state);
    #endif

    // Init Hiding *******************************************************************
    #if defined(SHUFFLING) || defined(DUMMY_OPS)
    mHiding.init();
    #endif

    // Shuffle S-Box indices *********************************************************
    #ifdef SHUFFLING
    mHiding.shuffleSBoxAccess(mShuffledSBoxIndices);
    #endif

    // Start Decryption **************************************************************
    // Round 10
    addRoundKey(mSubkeys[ROUNDS], state);
    invShiftRows(state);
    invByteSub(state);

    // Rounds 9-1
    for(uint8_t round=ROUNDS-1; round>0; round--)
    {
        addRoundKey(mSubkeys[round], state);
        invMixCols(state);
        // Re-Mask state after inverse MixCol
        #ifdef MASKING
        mMasking.invReMaskState(state);
        #endif
        invShiftRows(state);
        invByteSub(state);
    }

    // Last round
    addRoundKey(mSubkeys[0], state);

    // Unmask the state
    #ifdef MASKING
    mMasking.invUnMaskState(state);
    #endif

    // Read decrypted data back into cipher array ***********************************
    inByte = 0;
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            cipher[inByte++] = state[row][col];
}

// **********************************************************************************
// Private Methods ******************************************************************
// **********************************************************************************
// Key Schedule *********************************************************************
void AES::createKeySchedule(const aes_key_t masterKey, sub_keys_t subKeys) const
{
    // The first sub-key is the master-key itself
    memcpy(subKeys[0], masterKey, KEY_BYTES*sizeof(uint8_t));

    for(uint8_t keyIndex=1; keyIndex <= ROUNDS; keyIndex++)
    {
        // g-function
        uint8_t g[WORD_BYTES] =
        {
            static_cast<uint8_t>(pgm_read_byte(&LUT::S_BOX[subKeys[keyIndex-1][13]]) ^ mRCs[keyIndex-1]),
            static_cast<uint8_t>(pgm_read_byte(&LUT::S_BOX[subKeys[keyIndex-1][14]])),
            static_cast<uint8_t>(pgm_read_byte(&LUT::S_BOX[subKeys[keyIndex-1][15]])),
            static_cast<uint8_t>(pgm_read_byte(&LUT::S_BOX[subKeys[keyIndex-1][12]]))
        };
        // Add g-function to first 4 bytes
        for(uint8_t i=0; i<WORD_BYTES; i++)
            subKeys[keyIndex][i] = subKeys[keyIndex-1][i] ^ g[i];

        // For the other bytes, x-or the value from the last subkey
        // with the byte from the previous word from the current key
        // E.g: subKeys[1][4] = subKeys[0][4] ^ subKeys[1][0]
        for(uint8_t i=WORD_BYTES; i<KEY_BYTES; i++)
            subKeys[keyIndex][i] = subKeys[keyIndex-1][i] ^ subKeys[keyIndex][i-WORD_BYTES];
    }
}

// Key Addition Layer ***************************************************************
void AES::addRoundKey(const aes_key_t roundKey, state_t state)
{
    // Perform some NOPs before the actual operation
    // In case of DPA dummy ops are only performed before the S-Box accesses, to make DPA easier.
    #if defined(DUMMY_OPS) && !defined(DPA)
    mHiding.dummyOp();
    #endif

    uint8_t keyByte = 0;
    // Add key column by column
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            // Add each byte of the round key to the state in GF(2^8)
            state[row][col] ^= roundKey[keyByte++];
}

// Diffusion Layer ******************************************************************
void AES::invMixCols(state_t state)
{
    // Perform some NOPs before the actual operation
    // In case of DPA dummy ops are only performed before the S-Box accesses, to make DPA easier.
    #if defined(DUMMY_OPS) && !defined(DPA)
    mHiding.dummyOp();
    #endif

    // Do a matrix matrix multiplication of the inverse Mix-Column matrix & state matrix
    state_t tempState = {};
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            for(uint8_t element=0; element<WORD_BYTES; element++)
                tempState[row][col] ^= AESMath::ffMul(LUT::INV_MIX_COL_MATRIX[row][element], state[element][col]);

    memcpy(state, tempState, sizeof(state_t));
}

void AES::invShiftRows(state_t state)
{
    // Perform some NOPs before the actual operation
    // In case of DPA dummy ops are only performed before the S-Box accesses, to make DPA easier.
    #if defined(DUMMY_OPS) && !defined(DPA)
    mHiding.dummyOp();
    #endif

    // Shift each row right by the row number, 0 for the first row, 1 for the second row etc.
    for(uint8_t row=0; row<WORD_BYTES; row++)
        AESMath::rightRotateArray(state[row], WORD_BYTES, row);
}

// Byte Substitution layer **********************************************************
void AES::invByteSub(state_t state)
{
    #ifdef DUMMY_OPS
    // Perform some NOPs before the actual operation
    mHiding.dummyOp();
    #endif
    
    #ifdef SHUFFLING
    uint8_t index = 0;
    for(uint8_t i=0; i<STATE_BYTES; i++)
    {
        index = mShuffledSBoxIndices[i];
        // The row number is index mod 4, e.g. for 4 the row number is 0.
        // The column number is index / 4, e.g. for 4 the column number is 1.
        #ifdef MASKING
        state[index%4][index/4] = mMasking.getInvMaskedSBoxValue(state[index%4][index/4]);
        #else
        state[index%4][index/4] = pgm_read_byte(&LUT::INV_S_BOX[state[index%4][index/4]]);
        #endif
    }
    #else
    // Access the S-Box column by column
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            #ifdef MASKING
            state[row][col] = mMasking.getInvMaskedSBoxValue(state[row][col]);
            #else
            state[row][col] = pgm_read_byte(&LUT::INV_S_BOX[state[row][col]]);
            #endif
    #endif

}

