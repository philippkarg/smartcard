/**
 * @file masking.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @brief File containing the Masking class.
 * @date 28.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef MASK_H
#define MASK_H

#include "defs.h"
#include "lut.h"
#include "aesMath.h"
#include "rng.h"

// Logger
#ifdef DEBUG
#include "logger.h"
#endif

/**
 * @brief Masking class that provides functionality for masking and unmasking AES-decryption.
 * 
 * The masking techniques used here are strongly based on the work presented by Mangard, Oswald & Popp
 * in their book "Power Analysis Attacks: Revealing the Secrets of Smart Cards".
 * The implementation presented on pp. 228 ff. for encryption was adjusted to fit the decryption that was implemented here.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @date 28.06.2022
 * @copyright Philipp Karg 2022
 */
class Masking
{
public:
    /**
     * @brief Construct a new Masking object.
     */
    Masking() = default;

    /**
     * @brief Initialize the masks & the masked inverse S-Box.
     * 
     * -# Seed the Random-Number-Generator.
     * -# Create random m & m' masks.
     * -# Create the masked inverse S-Box, by calling initInvMaskedSBox().
     * -# Create random masks m_i', i=1..4.
     * -# Calculate the corresponding masks m_i, i=1..4 by calling initMixColInputMask().
     */
    void init();

    /**
     * @brief Mask the @p subKeys & store the masked keys in @p maskedSubKeys.
     * 
     * XOR the original keys with masks (m_i' ^ m), i=1..4.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[in] subKeys (const @ref sub_keys_t): Original sub-keys to be masked. 
     * @param[out] maskedSubKeys ( @ref sub_keys_t): Masked sub-keys. 
     */
    void maskSubKeys(const sub_keys_t subKeys, sub_keys_t maskedSubKeys) const;
    
    /**
     * @brief (Inverse) mask the state before the first AddRoundKey step.
     * 
     * XOR the state with (m_i' ^ m ^ m'), i=1..4.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[inout] state ( @ref state_t): State to be masked. 
     */
    void invMaskState(state_t state) const;

    /**
     * @brief (Inverse) re-mask the state after every MixCol step.
     * 
     * XOR the state with (m_i ^ m'), i=1..4.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[inout] state ( @ref state_t): State to be re-masked. 
     */
    void invReMaskState(state_t state) const;

    /**
     * @brief (Inverse) un-mask the state after the last AddRoundKey step.
     * 
     * XOR the state mit m_i', i=1..4.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[inout] state ( @ref state_t): State to be un-masked. 
     */
    void invUnMaskState(state_t state) const;

    /**
     * @brief Get a value of the (inverse) masked S-Box at a specific index.
     * @param[in] index (const uint8_t): Index to get value for. 
     * @return (uint8_t): The value at @p index.
     */
    uint8_t getInvMaskedSBoxValue(const uint8_t index) const { return mInvMaskedSBox[index]; }

private:
    // ******************************************************************************
    // Private Structures ***********************************************************
    // ******************************************************************************
    /**
     * @brief Structure for masks. 
     * Masks always come in pairs, with an input & output mask.
     */
    struct mask_t
    {
        uint8_t input;  ///< Input mask
        uint8_t output; ///< Output mask
    };

    // ******************************************************************************
    // Private Attributes ***********************************************************
    // ******************************************************************************
    uint8_t mInvMaskedSBox[SBOX_BYTES]; ///< Inverse S-Box with masked values
    
    /**
     * @brief SubByte input & output mask. 
     * 
     * In "Power Analysis Attacks" by Mangard et. al. p. 228 ff.,
     * the SubByte mask input mask is noted as m, while the output mask is noted as m'.
     */
    mask_t mSubByteMask     = {};

    /**
     * @brief 4 MixCol input & output masks.
     * 
     * In "Power Analysis Attacks" by Mangard et. al. p. 228 ff.,
     * the MixCol input masks are noted as m_i, while the output masks are noted as m_i', where i=1..4.
     */
    mask_t mMixColMasks[4]  = {};
    
    RNG mRNG;       ///< Random-Number-Generator
    #ifdef DEBUG
    Logger mLog;    ///< Logger
    #endif
    // ******************************************************************************
    // Private Methods **************************************************************
    // ******************************************************************************
    /**
     * @brief Compute the (inverse) masked S-Box.
     * 
     * Masking is done as follows: S_masked(x + m') = S(x) + m, where x is any index of the S-Box.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[out] maskedSBox (uint8_t*): The masked S-Box.
     * @param[in] subByteMask (const @ref mask_t): Masks m & m'. 
     */
    void initInvMaskedSBox(uint8_t maskedSBox[], const mask_t &subByteMask) const;

    /**
     * @brief Compute masks m_i, i=1..4, by performing a MixCol operation on masks m_i'.
     * @see "Power Analysis Attacks" by Mangard et. al. p. 228 ff.
     * @param[inout] mixColMasks ( @ref mask_t): Masks m_i & m_i'. 
     */
    void initMixColInputMask(mask_t mixColMasks[]) const;
};

#endif // MASK_H