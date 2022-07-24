/**
 * @file aesMath.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @brief File containing the AESMath class.
 * @date 02.07.2022
 * @copyright Philipp Karg 2022
 */

#ifndef AES_MATH_H
#define AES_MATH_H

#include "defs.h"

/**
 * @brief Class providing some mathematical functions needed for AES.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @date 02.07.2022
 * @copyright Philipp Karg 2022
 */
class AESMath
{
public:
    /**
     * @brief Reverse an array by swapping the lowest and highest element.
     * @param[inout] arr (uint8_t[]): Array to reverse. 
     * @param[in] low (uint8_t): Index of the lowest element.
     * @param[in] high (uint8_t): Index of the highest element.
     */
    static void reverseArray(uint8_t arr[], uint8_t low, uint8_t high);

    /**
     * @brief Swap the values of 2 integers.
     * @param[inout] a (uint8_t): First element to swap. 
     * @param[inout] b (uint8_t): Second element to swap.
     */
    static void swap(uint8_t &a, uint8_t &b);
    
    /**
     * @brief Rotate an array @p arr of length @p n by @p k to the right.
     * @param[in] arr (uint8_t*): Array to rotate.
     * @param[in] n (uint8_t): Length of the array.
     * @param[in] k (uint8_t): Amount to rotate.
     */
    static void rightRotateArray(uint8_t arr[], uint8_t n, uint8_t k);

    /**
     * @brief Multiply @p x and @p y in GF(2^8)
     * Implemented after https://en.wikipedia.org/wiki/Finite_field_arithmetic
     * @param[in] x (uint8_t): Left parameter to multiply. 
     * @param[in] y (uint8_t): Right parameter to multiply. 
     * @return (uint8_t): The result of the Finite-Field multiplication.
     */
    static uint8_t ffMul(uint8_t x, uint8_t y);
private:
    static constexpr uint8_t IRREDUCIBLE_POLYNOMIAL = 0x1B; ///< Irreducible polynomial: x^8 + x^4 + x^3 + x + 1
    
    /**
     * @brief Construct a new AESMath object.
     */
    AESMath() = default;
};

#endif // AES_MATH_H