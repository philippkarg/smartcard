#include "hiding.h"

#ifdef SHUFFLING
uint8_t Hiding::DEFAULT_INV_SBOX_INDICES[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
#endif

void Hiding::init()
{
    // Seed RNG *********************************************************************
    mRNG.seed();

    // Init dummy ops ***************************************************************
    #ifdef DUMMY_OPS
    uint8_t remainingDummyOps = MAX_NUMBER_NO_OPS;
    // Create an array of random numbers
    for(uint8_t i=0; i<NUMBER_OPS-1; i++)
    {
        mNumbersDummyOps[i] = mRNG.rand() % (remainingDummyOps/6);
        remainingDummyOps -= mNumbersDummyOps[i];
    }
    mNumbersDummyOps[NUMBER_OPS-1] = remainingDummyOps;

    // With this approach, the first few array entries are more likely to be bigger.
    // Therefore we also shuffle the array, to eliminate this bias.
    shuffleArray(mNumbersDummyOps, NUMBER_OPS);
    #endif
}

#ifdef SHUFFLING
void Hiding::shuffleSBoxAccess(uint8_t indices[])
{
    // Init indices
    memcpy(indices, DEFAULT_INV_SBOX_INDICES, STATE_BYTES);
    // Shuffle the array
    shuffleArray(indices, STATE_BYTES);
}
#endif

#ifdef DUMMY_OPS
void Hiding::dummyOp()
{
    // Execute as many dummy ops as specified in the array
    for(uint8_t i=0; i<mNumbersDummyOps[mNoOpCounter]; i++)
        __asm__("nop");
    mNoOpCounter++;
}
#endif

void Hiding::shuffleArray(uint8_t array[], const uint8_t size)
{
    // Shuffle the array with Fisher-Yates shuffle
    uint8_t j=0;
    for(uint8_t i=0; i<size-1; i++)
    {
        j = i + mRNG.rand() / (MAX_RAND / (size - i) + 1);;
        AESMath::swap(array[i], array[j]);
    }
}