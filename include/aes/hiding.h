/**
 * @file hiding.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @brief File that contains the Hiding class.
 * @date 04.07.2022
 * @copyright Philipp Karg 2022
 */

#ifndef HIDING_H
#define HIDING_H

#include "defs.h"
#include "rng.h"
#include "aesMath.h"
#include <string.h>

/**
 * @brief Class that implements 2 hiding techniques: dummy-ops & shuffling.
 * 
 * As a counter-measure against DPA attacks, hiding can be used to randomize
 * the timing behaviour of the AES en-/decryption.
 * This can be done by adding random dummy-ops or by randomly shuffling the S-Box access.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @date 04.07.2022
 * @copyright Philipp Karg 2022
 */
class Hiding
{
public:
    /**
     * @brief Construct a new Hiding object.
     */
    Hiding() = default;

    /**
     * @brief Initialize AES hiding operations.
     * 
     * -# Seed the RNG.
     * -# Init the dummy ops by creating an array of random numbers, w
     * which will be the number of dummy ops per round. It is important that the
     * total number of dummy ops stays the same for every AES execution.
     */
    void init();

    /**
     * @brief Shuffle the S-Box access.
     * 
     * Randomize the S-Box access by shuffling the indices of the S-Box.
     * @param[out] indices (uint8_t []): Array of indices of the S-Box. 
     */
    #ifdef SHUFFLING
    void shuffleSBoxAccess(uint8_t indices[]);
    #endif

    /**
     * @brief Perform a dummy NOP operation a random number of times.
     */
    #ifdef DUMMY_OPS
    void dummyOp();
    #endif

private:
    #ifdef DUMMY_OPS
    static constexpr uint8_t MAX_NUMBER_NO_OPS  = 100;      ///< The maximum number of NOPs per AES execution. It is important that this number stays the same for every AES execution.
    static constexpr uint8_t NUMBER_OPS         = 40;       ///< The number of operations before which the dummy ops are executed.
    uint8_t mNumbersDummyOps[NUMBER_OPS]        = {};       ///< Array of random numbers, which specify the number of dummy ops per round.
    uint8_t mNoOpCounter                        = 0;        ///< Counter for the number of dummy ops per round.
    #endif

    #ifdef SHUFFLING
    static uint8_t DEFAULT_INV_SBOX_INDICES[STATE_BYTES];   ///< Array that contains values from 0 to 15.
    #endif

    RNG mRNG;                                               ///< Random number generator.       

    /**
     * @brief Shuffle an array using the Fisher-Yates shuffle.
     * @param[inout] array (uint8_t []): Array to shuffle.
     * @param[in] size (const uint8_t): Size of the array.
     */
    void shuffleArray(uint8_t array[], const uint8_t size);
};

#endif // HIDING_H
