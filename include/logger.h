/**
 * @file logger.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @brief File containing a Logger class using UART.
 * @date 02.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string.h>

#include "defs.h"

/**
 * @brief Logger class that outputs logs over USART.
 *
 * @authors Philipp Karg (philipp.karg@tum.de)
 * 
 * @date 02.06.2022
 * @copyright Philipp Karg 2022
 */
class Logger
{
public:
    /**
     * @brief Construct a new Logger object.
     */
    Logger() = default;

    /**
     * @brief Initialize USART.
     * 
     * -# Enable double speed.
     * -# Set baud-rate in both UBRR registers with #UBRR0_VALUE.
     * -# Enable the transmitter.
     * -# Set the data frame (8 data-bits, 1 stop-bit).
     */
    void init();

    /**
     * @brief Functor to log a string.
     * @param[in] str (const char*): String to log. 
     */
    void operator()(const char* str) const { sendStr(str); }

    /**
     * @brief Functor to log an array of hex-values.
     * @param[in] arr (const uint8_t*): Array of hex values to log. 
     * @param[in] len (const uint8_t): Length of the array.
     */
    void operator()(const uint8_t* arr, const uint8_t len) const;

private:
    /**
     * @brief UBRR0 value for initializing USART.
     * 
     * This value was found through trial and error.
     * According to the data-sheet, this value should be calculated as the following:
     * UBRR0 = f_clk/(16*BAUD) - 1
     * However this did not work with the provided clock-frequency of 3276800 in the manual.
     */
    static constexpr uint16_t UBRR0_VALUE = F_CPU/(8*BAUD)-1;

    /**
     * @brief Transmit a single character over USART.
     * 
     * -# Wait for an empty transmit buffer.
     * -# Write @p c into the buffer.
     * 
     * @param[in] c (const char): Character to send.
     */
    void sendChar(const char c) const;

    /**
     * @brief Transmit a string over USART.
     * 
     * Continuously call sendChar() and send each character of @p str.
     * @param[in] str (const char*): String to send.
     */
    void sendStr(const char* str) const;
};

#endif // LOGGER_H