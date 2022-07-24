/**
 * @file aes.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @brief File containing the AES class.
 * @date 23.05.2022
 * @copyright Philipp Karg 2022
 */

#ifndef AES_H
#define AES_H

#include <string.h>

#include "defs.h"
#include "lut.h"
#include "aesMath.h"

// Logger
#ifdef DEBUG
#include "logger.h"
#endif

// Masking
#ifdef MASKING
#include "masking.h"
#endif

// Hiding
#if defined(SHUFFLING) || defined(DUMMY_OPS)
#include "hiding.h"
#endif

/**
 * @brief Class providing functionality for 128-bit AES decryption.
 * 
 * If MASKING is defined, the class also provides functionality for masking and unmasking AES-decryption.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @date 23.05.2022
 * @copyright Philipp Karg 2022
 */
class AES
{
public:
    /**
     * @brief Construct a new AES object.
     * @param[in] masterKey (const @ref aes_key_t): The master key.
     */
    AES(const aes_key_t masterKey);

    /**
     * @brief Decrypt a cipher using the 128-bit AES algorithm.
     * @see <a href="https://swarm.cs.pub.ro/~mbarbulescu/cripto/Understanding%20Cryptography%20by%20Christof%20Paar%20.pdf#section.4.5.gb" target="_blank">p. 110-112</a> 
     * @param[inout] cipher (uint8_t *): Cipher to decrypt.
     */
    void decrypt(uint8_t *cipher);
    
    #if defined(SHUFFLING) || defined(DUMMY_OPS)
    void aesWithShuffling();
    #endif
    
private:
    // *******************************************************************************
    // Private Attributes ************************************************************
    // *******************************************************************************
    sub_keys_t mSubkeys = {};       ///< Array that contains all subkeys
    static uint8_t mRCs[ROUNDS];    ///< Array of round coefficients that are used in the key schedule.

    // Logger 
    #ifdef DEBUG
    Logger mLog;                    ///< Logger
    #endif

    // Masking
    #ifdef MASKING
    Masking mMasking;               ///< Masking object
    sub_keys_t mOriginalSubKeys;    ///< Array that contains all original subkeys, if masking is enabled
    #endif

    // Hiding
    #if defined(SHUFFLING) || defined(DUMMY_OPS)
    Hiding mHiding;                 ///< Hiding object
    #endif

    #ifdef SHUFFLING
    uint8_t mShuffledSBoxIndices[STATE_BYTES] = {}; ///< Array of indices of the S-Box, if shuffling is enabled.
    #endif
    
    // ******************************************************************************
    // Private Methods **************************************************************
    // ******************************************************************************
    // Key Schedule *****************************************************************
    /**
     * @brief Create the AES key-schedule & store all subkeys in #mSubkeys.
     * 
     * - The first subkey is @p masterKey.
     * - The remaining subkeys are calculated as defined in the %AES standard.
     *   See <a href="https://swarm.cs.pub.ro/~mbarbulescu/cripto/Understanding%20Cryptography%20by%20Christof%20Paar%20.pdf#subsection.4.4.4.Fa" target="_blank">p. 106-108</a> 
     *   for reference.
     *    
     * @param[in] masterKey (const @ref aes_key_t): The master key, which is used to create the key schedule.
     * @param[out] subKeys ( @ref sub_keys_t): Array that contains all subkeys.
     */
    void createKeySchedule(const aes_key_t masterKey, sub_keys_t subKeys) const;
    
    // Key Addition Layer ***********************************************************
    /**
     * @brief Add the key for the current round to @p state.
     * 
     * X-OR each byte of @p state with the corresponding byte in @p roundKey.
     * @param[in] roundKey (const @ref aes_key_t): Key for the current round. 
     * @param[inout] state ( @ref state_t): Current state matrix. 
     */
    void addRoundKey(const aes_key_t roundKey, state_t state);

    // Diffusion Layer **************************************************************
    /**
     * @brief Inverse MixColumn sublayer.
     * 
     * Do a matrix-matrix multiplication of @p state & #INV_MIX_COL_MATRIX.
     * @param[inout] state ( @ref state_t): Current state matrix. 
     */
    void invMixCols(state_t state);

    /**
     * @brief Inverse ShiftRows sublayer.
     * 
     * Rotate each row of the @p state matrix by the row-number to the right.
     * @param[inout] state ( @ref state_t): Current state matrix. 
     */
    void invShiftRows(state_t state);

    // Byte Substitution layer ******************************************************
    /**
     * @brief Inverse Byte Substituion layer.
     *
     * Substitute each byte in @p state with the corresponding value in #INV_S_BOX.
     * @param[inout] state ( @ref state_t): Current state matrix. 
     */
    void invByteSub(state_t state);
};

#endif // AES_H
