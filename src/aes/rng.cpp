#include "rng.h"

void RNG::init()
{
    ADCSRA = (1 << ADPS2) | (1 << ADPS0);   // ADC clock prescaler divide by 32
    SET_BIT(ADCSRA, ADEN);                  // Enable ADC0

    // Dummy read
    SET_BIT(ADCSRA, ADSC);                  // Start ADC conversion
    while(GET_BIT(ADCSRA, ADSC));           // Wait for conversion to finish
    (void) ADCL;                            // Throw away the result
}

void RNG::seed()
{
    mRand = 0;
    for(uint8_t i=0; i<8; i++)      // Seed the RNG
        mRand |= (readADC() << i);
}

uint8_t RNG::rand()
{
    mRand ^= (mRand << 7);
    mRand ^= (mRand >> 5);
    return mRand ^= (mRand << 3);
}

bit_t RNG::readADC()
{
    SET_BIT(ADCSRA, ADSC);          // Start ADC conversion
    while (GET_BIT(ADCSRA, ADSC));  // Wait until conversion is finished
    uint16_t adcValue = ADCL;
    adcValue |= (ADCH << 8);
    return (adcValue & 0x01);       // Return the LSB of the ADC value
}

