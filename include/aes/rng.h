/**
 * @file rng.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @brief File that contains the RNG class.
 * @date 23.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef RNG_H
#define RNG_H

#include "defs.h"
#define MAX_RAND 255

/**
 * @brief Class that provides a random number generator. 
 * The RNG can be seeded by reading noise from an unused ADC.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @date 23.06.2022
 * @copyright Philipp Karg 2022
 */
class RNG
{
public:
    /**
     * @brief Construct a new RNG object.
     */
    RNG() = default;

    /**
     * @brief Initialize the RNG & underlying ADC.
     */
    static void init();

    /**
     * @brief Seed the RNG by reading the ADC's LSB 8 times.
     * @pre Requires init() to be called before.
     */
    void seed();
    
    /**
     * @brief Create a pseudo-random number between 0 and 255.
     * 
     * Simple 8-bit adaption of the Xorshift PRNG.
     * @return (uint8_t): The pseudo-random number.
     */
    uint8_t rand();

private:
    uint8_t mRand = 0; ///< The RNG's state

    /**
     * @brief Read the LSB from ADC0.
     * @pre Requires init() to be called before.
     * @return (bit_t): The read LSB.
     */
    static bit_t readADC();
};

#endif // RNG_H