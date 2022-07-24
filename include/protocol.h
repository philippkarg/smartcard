/**
 * @file protocol.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @brief File containing a structure with protocol definitions.
 * @date 09.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "defs.h"

/**
 * @brief Some definitions of the protocol used for communication between the SmartCard & Terminal.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @date 09.06.2022
 * @copyright Philipp Karg 2022
 */
struct Protocol
{
private:
    static constexpr byte_t TS                  = 0x3b;
    static constexpr byte_t T0                  = 0x90;
    static constexpr byte_t TA1                 = 0x11;
    static constexpr byte_t TD1                 = 0x00;
    static constexpr byte_t CLA                 = 0x88;
    static constexpr byte_t INS_DATA_IN         = 0x10;
    static constexpr byte_t INS_DATA_OUT        = 0xc0;
    static constexpr byte_t P1                  = 0x00;
    static constexpr byte_t P2                  = 0x00;
    static constexpr byte_t P3                  = 0x10;
public:
    // Protocol definitions *********************************************************
    // ATR
    static constexpr byte_t ATR_SEQ[]           = {TS, T0, TA1, TD1};               ///< Answer-to-reset sequence, send at the start
    static constexpr uint8_t ATR_LENGTH         = 4;                                ///< Length of the Answer-to-reset sequence
    // Data in/out
    static constexpr byte_t DATA_IN_HEADER[]    = {CLA, INS_DATA_IN, P1, P2, P3};   ///< T=0 protocol header for incoming data to be decrypted
    static constexpr byte_t DATA_OUT_HEADER[]   = {CLA, INS_DATA_OUT, P1, P2, P3};  ///< T=0 protocol header for decrypted outgoing data
    static constexpr uint8_t HEADER_LENGTH      = 5;                                ///< Length of the T=0 protocol headers
    static constexpr byte_t ACK_DATA_IN         = INS_DATA_IN ^ 0xff;               ///< Acknowledge byte for instruction 0x10
    static constexpr byte_t ACK_DATA_OUT        = INS_DATA_OUT;                     ///< Acknowledge byte for instruction 0xc0
    static constexpr byte_t RESPONSE_DECRYPTED[]= {0x61, 0x10};                     ///< Response that is sent after the data to decrypt has been received
    static constexpr byte_t RESPONSE_DATA_OUT[] = {0x9d, 0x00};                     ///< Response after sending the decrypted data
    static constexpr uint8_t RESPONSE_LENGTH    = 2;                                ///< Response length
};

#endif // PROTOCOL_H