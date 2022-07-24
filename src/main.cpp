#ifdef __cplusplus
extern "C" {
#include <avr/io.h>
#include <util/delay.h>
}
#endif

#ifdef DEBUG
#include "logger.h"
#endif

#include "defs.h"
#include "aes.h"
#include "communication.h"

int main()
{
    // Initialization ***************************************************************
    // Communication Protocol
    Communication comm;

    // RNG
    #if defined(MASKING) || defined(SHUFFLING) || defined(DUMMY_OPS)
    RNG::init();
    #endif

    // Setting direction trigger (JP5) pin
    SET_BIT(DDRB, DDB4);
    
    // AES
    uint8_t key[] = { 0xff, 0xcd, 0x13, 0xbd, 0xd3, 0xc8, 0x7f, 0xb4, 0x41, 0x25, 0xe8, 0x46, 0x18, 0xfa, 0xb7, 0xd4 };
    AES aes(key);
    uint8_t cipher[STATE_BYTES] = {};

    // Logger
    #ifdef DEBUG
    Logger log;
    log.init();
    #endif

    // Global interrupts
    sei();

    // Send ATR
    comm.sendATR();
    
    // Infinite loop ****************************************************************
    while(1)
    {
        // Receive data to decrypt
        comm.receiveDataToDecrypt(cipher);

        // Received data
        #ifdef DEBUG
        log("Received data to decrypt: ");
        log(cipher, KEY_BYTES);
        #endif

        // Setting value of trigger (JP5) pin
        SET_BIT(PORTB, PB4);
    
        // Decrypt data
        aes.decrypt(cipher);

        // Clearing value of trigger (JP5) pin
        CLR_BIT(PORTB, PB4);
        
        // Decrypted data
        #ifdef DEBUG
        log("Decrypted data: ");
        log(cipher, KEY_BYTES);
        #endif
        // Send the decrypted data back
        comm.sendDecryptedData(cipher);
    }
}