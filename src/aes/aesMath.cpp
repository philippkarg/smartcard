#include "aesMath.h"

void AESMath::swap(uint8_t &a, uint8_t &b)
{
    uint8_t temp = a;
    a = b;
    b = temp;
}

void AESMath::reverseArray(uint8_t arr[], uint8_t low, uint8_t high)
{
    while(low < high) swap(arr[low++], arr[high--]);
}

void AESMath::rightRotateArray(uint8_t arr[], uint8_t n, uint8_t k)
{
    // Reverse the last k elements
    reverseArray(arr, n-k, n-1);
    // Reverse the first n-k elements
    reverseArray(arr, 0, n-k-1);
    // Reverse the whole array
    reverseArray(arr, 0, n-1);
}

uint8_t AESMath::ffMul(uint8_t x, uint8_t y)
{
    uint8_t product = 0;
    // Divide by 2 in GF(2^8) until y is 0
    for(; y; y >>= 1)
    {
        // LSB set in y
        if(y & 0x01) product ^= x;
        // Check if MSB set in x
        if(x & 0x80)
            // Left-shift x (multiply by 2 in GF(2^8))
            // & add the irreducible polynomial
            x = (x << 0x01) ^ IRREDUCIBLE_POLYNOMIAL;
        else
            // Otherwise just left shift x (multiply by 2 in GF(2^8))
            x <<= 0x01;
    }

    return product;
}