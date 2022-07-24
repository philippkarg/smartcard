/**
 * @file communication.h
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @brief File containing the Communication class.
 * @date 05.06.2022
 * @copyright Philipp Karg 2022
 */

#ifndef COMM_INTERFACE_H
#define COMM_INTERFACE_H

#ifdef __cplusplus
extern "C" 
{
    #include <avr/interrupt.h>
}
#endif

#include "defs.h"
#include "protocol.h"

#ifdef DEBUG
#include "logger.h"
#endif

/**
 * @brief Enum class to select the direction of the IOPin.
 */
enum class PinDir
{
    OUTPUT  = 0,    ///< Set the IOPin to output
    INPUT   = 1     ///< Set the IOPin to input
};

/**
 * @brief Class that implements the communication protocol between the SmartCard & the Terminal.
 * 
 * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
 * 
 * @date 05.06.2022
 * @copyright Philipp Karg 2022
 */
class Communication
{
public:
    // ******************************************************************************
    // Public Methods ***************************************************************
    // ******************************************************************************
    /**
     * @brief Construct a new Communication object.
     * 
     * Initialize the Timer & IOPin class.
     */
    Communication();
    
    /**
     * @brief Send the Answer-To-Reset sequence to the Terminal.
     * 
     * After the Reset-Pin is set to 1, send this sequence to initiate the transfer.
     */
    void sendATR() { sendBytes(Protocol::ATR_SEQ, Protocol::ATR_LENGTH); }

    /**
     * @brief Receive data to decrypt from the Terminal.
     * 
     * -# Receive the protocol header, #Protocol::DATA_IN_HEADER, by calling receiveProtocolHeader().
     * -# Receive 16 bytes of data byte-by-byte & send #Protocol::ACK_DATA_IN after each byte.
     * 
     * @param[out] data ( @ref byte_t*): Byte array to store the received data in. 
     */
    void receiveDataToDecrypt(byte_t *data);
    
    /**
     * @brief Send the decrypted data to the Terminal.
     * 
     * -# Indicate that the decryption is done by sending #Protocol::RESPONSE_DECRYPTED.
     * -# Receive the protocol header, #Protocol::DATA_OUT_HEADER, by calling receiveProtocolHeader().
     * -# Send #Protocol::ACK_DATA_OUT.
     * -# Send each decrypted byte consequentially.
     * -# Indicate that the transfer of decrypted data is done, by sending #Protocol::RESPONSE_DATA_OUT.
     * 
     * @param[in] data (const @ref byte_t*): Decrypted byte array to send to the Terminal. 
     */
    void sendDecryptedData(const byte_t *data);

private:
    // ******************************************************************************
    // Private Subclasses ***********************************************************
    // ******************************************************************************
    /**
     * @brief Class that provides functionality for the ATmega644's on-board 16-bit timer, such as an ISR.
     * 
     * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
     * 
     * @date 05.06.2022
     * @copyright Philipp Karg 2022
     */
    class Timer
    {
    public:
        // Public Methods ***********************************************************
        /**
         * @brief Initialize the Timer class.
         * 
         * - Copy the address of the provided Communication pointer, @p comm to #mComm.
         * - Set-up the timer: Enable CTC mode, enable Output Compare Match A Interrupts
         *   & set the match value to #ETU.
         * 
         * @param[in] comm (Communication *): Communication pointer. 
         */
        static void init(Communication *comm);
        
        /**
         * @brief Stop the 16-bit timer by setting no clock source.
         */
        static void stop() { CLR_BIT(TCCR1B, CS10); }

        /**
         * @brief Start the timer, by setting the counter value to #TIMER_BOTTOM & setting the clock source to CS10.
         */
        static void start() { TCNT1 = TIMER_BOTTOM; SET_BIT(TCCR1B, CS10); }

        /**
         * @brief Change the value of register OCR1A.
         * @param[in] matchValue (const uint16_t): Value to set for OCR1A. 
         */
        static void setMatchValue(const uint16_t matchValue) { OCR1A = matchValue; }
        
        /**
         * @brief The counter value which the timer should match.
         * 
         * The default value for a single elementary-time unit (ETU) is: 1ETU = F/D * 1/f_clk,
         * where F is the clock rate conversion integer with a default value of 372 &
         * D is the baud rate adjustment integer with a default value of 1.
         * 
         * If the Timer is run at the same clock speed as the CPU, it needs to count F/D = 372/1 times,
         * to match at every ETU.
         */
        static constexpr uint16_t ETU = 372/1;

    private:
        // Private Attributes *******************************************************
        static Communication *mComm;                            ///< Communication object to access the class methods & attributes
        static constexpr uint16_t TIMER_BOTTOM = 0x0000;        ///< Timer bottom value

        // Private Methods **********************************************************
        /**
         * @brief Interrupt Service Routine for the 16-bit timer. 
         *        An interrupt is triggered if the timer hits the value stored in OCR1A.
         */
        static void serviceRoutine() __asm__("__vector_13") __attribute__((__signal__, __used__, __externally_visible__));
    };

    /**
     * @brief Class that provides functionality for the ATmega644's PinB6, such as an ISR.
     * 
     * @authors Philipp Karg (philipp.karg@tum.de/ge49bov)
     * 
     * @date 05.06.2022
     * @copyright Philipp Karg 2022
     */
    class IOPin
    {
    public:
        // Public Methods ***********************************************************
        /**
         * @brief Initialize the IOPin class.
         * 
         * - Copy the address of the provided Communication pointer, @p comm to #mComm.
         * - Enable interrupts for Pins 8-15.
         * 
         * @param[in] comm (Communication *): Communication pointer. 
         */
        static void init(Communication *comm);
        
        /**
         * @brief Set logical level of the Pin.
         * @param[in] bit (const @ref bit_t): Level to set (0 or 1). 
         */
        static void setLevel(const bit_t bit);

        /**
         * @brief Set the direction of the Pin.
         * @param[in] direction (const ::PinDir): Direction to set (PinDir::INPUT or PinDir::OUTPUT). 
         */
        static void setDirection(const PinDir direction);

        /**
         * @brief Enable/disable interrupts for the Pin.
         * @param[in] enabled (const bool): Whether interrupts should be enabled or not. 
         */
        static void setInterrupt(const bool enabled);
    
    private:
        // Private Attributes *******************************************************
        static Communication *mComm; ///< Communication object to access the class methods & attributes

        // Private Methods **********************************************************
        /**
         * @brief Interrupt Service Routine for the IOPin. 
         *        An interrupt is triggered if the logic-level of the Pin changes.
         */
        static void serviceRoutine() __asm__("__vector_5") __attribute__((__signal__, __used__, __externally_visible__));
    };

    // ******************************************************************************
    // Private Attributes ***********************************************************
    // ******************************************************************************
    // Static Constexpr Attributes **************************************************
    static constexpr bit_t START_BIT        = 0;                ///< The start bit of a transfer
    static constexpr bit_t STOP_BIT         = 1;                ///< The stop bit of a transfer

    // Class Objects ****************************************************************
    friend Timer;   ///< 16-bit Timer/Counter
    friend IOPin;   ///< IOPin (PinB6)
    #ifdef DEBUG
    Logger mLog;    ///< Logger
    #endif

    // Volatile Attributes **********************************************************
    volatile PinDir     mDirection          = PinDir::OUTPUT;   ///< Current direction of the IOPin
    // Output flags/data
    volatile bool       mBitSent            = true;             ///< Whether the last bit of a transmission was sent
    volatile bit_t      mOutputBit          = 0;                ///< The next bit to output
    // Input flags/data
    volatile bool       mByteReceived       = 0;                ///< Whether a byte was received successfully
    volatile uint8_t    mInputBitCounter    = 0;                ///< Number of input bits in the current transfer
    volatile byte_t     mInputByte          = 0x00;             ///< The currently received input byte
    // Error flags/data
    volatile bool       mCheckErrors        = false;            ///< Whether to check for errors after sending a byte
    volatile bit_t      mErrorBit           = 0;                ///< Error bit that is set to 0/1 during the stop bit indicating failure/success.
    volatile bool       mParityError        = false;            ///< Whether a parity error occurred while receiving a byte

    // ******************************************************************************
    // Private Methods **************************************************************
    // ******************************************************************************
    // Send/Receive *****************************************************************
    /**
     * @brief Send a single @p bit to the Terminal.
     * 
     * -# Wait for the last bit to be sent, (#mBitSent to be true).
     * -# Set #mOutputBit to @p bit.
     * -# Set #mBitSent to false.
     * 
     * @param[in] bit (const @ref bit_t): The bit to send. 
     */
    void sendBit(const bit_t bit);

    /**
     * @brief Send a single @p byte to the Terminal.
     * 
     * -# Set the direction to output (IOPin::setDirection()) 
     *    & disable interrupts for the IOPin (IOPin::setInterrupt()).
     * -# Start the timer & set the match value to 1 Timer::ETU.
     * -# Send the #START_BIT.
     * -# Send each bit of @p byte separately.
     * -# Calculate the parity (getParity()) & send the parity bit.
     * -# Send the #STOP_BIT.
     * -# After half an Timer::ETU, check if the IOPin is low, which indicates a parity error.
     *    If so, restart at 2.
     * 
     * @param[in] byte (const @ref byte_t): Byte to send. 
     */
    void sendByte(const byte_t byte);

    /**
     * @brief Send an array of @p bytes byte by byte.
     * @param[in] bytes (const @ref byte_t*): Array of bytes. 
     * @param[in] len (const uint8_t): Length of the array.
     */
    void sendBytes(const byte_t *bytes, const uint8_t len) { for(uint8_t i=0; i<len; i++) sendByte(bytes[i]); }

    /**
     * @brief Sample the IOPin 3-times for a more reliable result.
     * @return (bit_t): The sample value of the bit.
     */
    static bit_t sampleBit();

    /**
     * @brief Receive a single byte from the Terminal.
     * 
     * -# Set the direction to input (IOPin::setDirection()) 
     *    & enable interrupts for the IOPin (IOPin::setInterrupt()).
     * -# Set #mByteReceived to false & wait until it's true again.
     * 
     * @return ( @ref byte_t): The received byte
     */
    byte_t receiveByte();

    /**
     * @brief Receive a protocol header, which contains 5 bytes.
     * @param[in] header (const @ref byte_t*): Header to expect.
     */
    void receiveProtocolHeader(const byte_t *header);

    // Helper functions *************************************************************
    /**
     * @brief Calculate the parity of a @p byte.
     * 
     * Copied from https://stackoverflow.com/questions/21617970/how-to-check-if-value-has-even-parity-of-bits-or-odd.
     * @param[in] byte ( @ref byte_t): Byte to calculate parity for.
     * @return ( @ref bit_t): The parity bit.
     */
    static bit_t getParity(byte_t byte);
};

#endif // COMM_INTERFACE_H