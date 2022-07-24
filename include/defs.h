/**
 * @file defs.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @brief Some macros and type definitions.
 * @date 02.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// C Includes
#ifdef __cplusplus
extern "C"
{   
    #include <avr/io.h>
}
#endif

#define SET_BIT(reg, pos) reg |= (1<<pos)           ///< Set a single bit at a specific position
#define CLR_BIT(reg, pos) reg &= ~(1<<pos)          ///< Clear a single bit at a specific position
#define GET_BIT(reg, pos) (reg & (1<<pos))          ///< Read a single bit at a specific position

#define WORD_BYTES  (uint8_t)(4)                    ///< Number of bytes in a word (32-bit integer)
#define KEY_BYTES   (uint8_t)(16)                   ///< Number of bytes in an AES key
#define STATE_BYTES (uint8_t)(16)                   ///< Number of bytes in a state (16-byte block)
#define SBOX_BYTES  (uint16_t)(256)                 ///< Number of bytes in the S-Box
#define ROUNDS      (uint8_t)(10)                   ///< Number of rounds in AES-128

typedef bool bit_t;                                 ///< Type definition for a bit
typedef uint8_t byte_t;                             ///< Type definition for a byte
typedef uint8_t state_t[WORD_BYTES][WORD_BYTES];    ///< Variable type for the %AES state matrix
typedef uint8_t aes_key_t[KEY_BYTES];               ///< Variable type for the %AES keys
typedef uint8_t sub_keys_t[ROUNDS+1][KEY_BYTES];    ///< Variable type for the %AES subkeys

#endif // DEFS_H