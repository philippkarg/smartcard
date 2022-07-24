#include "masking.h"
// **********************************************************************************
// Public Methods *******************************************************************
// **********************************************************************************
void Masking::init()
{
    // Seed RNG
    mRNG.seed();
    // Compute SubBytes mask
    mSubByteMask.input = mRNG.rand();

    // When recording DPA traces, reduce the number of random masks to make DPA easier.
    #ifdef DPA
    mSubByteMask.output = mSubByteMask.input;
    // Compute MixCols masks
    for(uint8_t i = 0; i < 4; i++)
    {
        mMixColMasks[i].output = mSubByteMask.input;
        mMixColMasks[i].input = 0;
    }
    #else
    mSubByteMask.output = mRNG.rand();
    // Compute MixCols masks
    for(uint8_t i = 0; i < 4; i++)
    {
        mMixColMasks[i].output = mRNG.rand();
        mMixColMasks[i].input = 0;
    }
    #endif

    // Init S-Box
    initInvMaskedSBox(mInvMaskedSBox, mSubByteMask);

    initMixColInputMask(mMixColMasks);
}

void Masking::maskSubKeys(const sub_keys_t subKeys, sub_keys_t maskedSubKeys) const
{
    // The round keys are masked with (m_i' ^ m), i=1..4.
    // m_i' are the MixCol output masks & m is the SubBytes input mask.
    for(uint8_t i=0; i<ROUNDS+1; i++)
        for(uint8_t j=0; j<STATE_BYTES; j++)
            maskedSubKeys[i][j] = subKeys[i][j] ^ mMixColMasks[j%4].output ^ mSubByteMask.input;
}

void Masking::invMaskState(state_t state) const
{
    // The first decryption round starts with AddRoundKey & then InvShiftRows.
    // Before InvShiftRows, the state needs to be masked with m'.
    // Since the key is masked with (m_i' ^ m), i=1..4,
    // the state needs to be masked with (m_i' ^ m ^ m') before the first round.
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            state[row][col] ^= mMixColMasks[row].output ^ mSubByteMask.input ^ mSubByteMask.output;
}

void Masking::invReMaskState(state_t state) const
{
    // After inverse  MixCol, the first state row is masked with m_1, the second one with m_2, etc.
    // We want to change this mask to be m' for all state bytes, by first XORing with m_i
    // to remove the m_i masks and then XORing with m'.
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            state[row][col] ^= mMixColMasks[row].input ^ mSubByteMask.output;
}

void Masking::invUnMaskState(state_t state) const
{
    // After the last AddKey round, the first state row is masked with m_1', the second one with m_2', etc.
    // We want to remove this mask, by XORing with m_i' for the respective rows.
    for(uint8_t col=0; col<WORD_BYTES; col++)
        for(uint8_t row=0; row<WORD_BYTES; row++)
            state[row][col] ^= mMixColMasks[row].output;
}

// **********************************************************************************
// Private Methods ******************************************************************
// **********************************************************************************
void Masking::initInvMaskedSBox(uint8_t maskedSBox[], const mask_t &subByteMask) const
{
    // See Power Analysis Attacks p. 239: S_m(x ^ m') = S(x) ^ m (inverted since we are doing decryption).
    for(uint16_t i=0; i<SBOX_BYTES; i++)
        maskedSBox[i ^ subByteMask.output] = pgm_read_byte(&LUT::INV_S_BOX[i]) ^ subByteMask.input;
}

void Masking::initMixColInputMask(mask_t mixColMasks[]) const
{
    // Do a matrix vector multiplication of the inverse Mix-Column matrix & output mask vector.
    // We are computing the input masks instead of the output masks since we are doing decryption.
    for(uint8_t row=0; row<WORD_BYTES; row++)
        for(uint8_t element=0; element<WORD_BYTES; element++)
            mixColMasks[row].input ^= AESMath::ffMul(LUT::INV_MIX_COL_MATRIX[row][element], mixColMasks[element].output);
}