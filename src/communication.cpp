#include "communication.h"

// **********************************************************************************
// Timer Methods ********************************************************************
// **********************************************************************************
Communication *Communication::Timer::mComm = 0;

void Communication::Timer::init(Communication *comm)
{
    // Init Communication object ****************************************************
    mComm = comm;
    // Timer settings ***************************************************************
    // Timer/Counter control register description (ATmega644 data sheet p.125-132):
    // ------------------------------------------------------------------------------
    // Timer/Counter Control Register 1 A (TCCR1A):
    // Bits 4:7 set to 0 (default): Normal port operation, OC1A/OC1B are disconnected
    // Bits 2:3 are reserved
    // Bits 0:1 set to 0 (default): We want to use the timer in Clear Timer on Compare Match (CTC) mode
    // ------------------------------------------------------------------------------
    // Timer/Counter Control Register 1 B (TCCR1B):
    // Bits 6:7 set to 0 (default): We are not using input capture events
    // Bit 5 is reserved
    // Bit 4 is a don't care, so we can leave it the default value 0
    // Bit 3 must be set to 1 to enable CTC mode 
    // Bits 0:2 set to 0 (default): These pins are used to select a clock. Since we don't want to start the timer right now, we leave these pins
    // ------------------------------------------------------------------------------
    SET_BIT(TCCR1B, WGM12);     // Enable CTC mode
    SET_BIT(TIMSK1, OCIE1A);    // Enable Output Compare Match A Interrupts
    setMatchValue(ETU);         // Set value for the Output Compare Register
}

void Communication::Timer::serviceRoutine()
{
    if(!mComm) return;
    switch(mComm->mDirection)
    {
        case PinDir::OUTPUT:
        {
            if(mComm->mParityError)
            {
                IOPin::setLevel(0); // Indicate an error in the stop bit
                stop();             // Stop the timer
                // Wait for the next interrupt on the I/O-Pin
                IOPin::setDirection(PinDir::INPUT);
                IOPin::setInterrupt(true);
            }
            else
            {
                // Send a 0 or 1 depending on the next bit to send
                IOPin::setLevel(mComm->mOutputBit);
                mComm->mBitSent = true;
            }
        }
        break;
        
        case PinDir::INPUT:
        {
            // When checking for tranmission errors, stop timer & receive a single bit
            if(mComm->mCheckErrors)
            {
                stop();
                mComm->mErrorBit = sampleBit();
                mComm->mCheckErrors = false;
            }
            else
            {
                // After receiving the first data bit, set the match value to 1 ETU
                if(mComm->mInputBitCounter == 0)
                    setMatchValue(ETU);
                // Read the current bit
                bit_t currBit = sampleBit();
                // Bits 0:7 are the data bits. Add them to the current byte
                if(mComm->mInputBitCounter < 8)
                    mComm->mInputByte |= (currBit << mComm->mInputBitCounter);
                // Bit 8 is the parity bit
                else
                {
                    // If the received parity bit is wrong, set the I/O-Pin
                    // to 0 during the stop bit.
                    if(currBit != getParity(mComm->mInputByte))
                    {
                        mComm->mParityError = true;
                        IOPin::setDirection(PinDir::OUTPUT);
                    }
                    // Otherwise, the byte transfer is done & the timer can be stopped
                    else
                    {
                        stop();
                        mComm->mByteReceived = true;
                    }
                }
                mComm->mInputBitCounter++;
            }
        }
        break;

        default: break;
    }
}

// **********************************************************************************
// IOPin Methods ********************************************************************
// **********************************************************************************
Communication *Communication::IOPin::mComm = 0;

void Communication::IOPin::init(Communication *comm)
{
    // Init Communication object ****************************************************
    mComm = comm;
    // Pin settings *****************************************************************
    setDirection(PinDir::INPUT);    // Set direction to input
    setLevel(true);                 // Activate pull-up resistor
}

void Communication::IOPin::serviceRoutine()
{
    if(!mComm) return;
    if(GET_BIT(PINB, PINB6) == 0 && mComm->mDirection == PinDir::INPUT)
    {
        // Immediately start timer 
        Timer::start();
        // Set the match value to 1.5*ETU - 50 to sample each bit in the middle
        Timer::setMatchValue(Timer::ETU*1.5-50);
        // Reset input bit counter & input byte
        mComm->mInputBitCounter = 0;
        mComm->mInputByte = 0x00;
        // Disable the interrupt for the I/O-Pin until the next start bit
        setInterrupt(false);
    }
    #ifdef DEBUG
    else
    {
        if(mComm->mDirection != PinDir::INPUT)
            mComm->mLog("The I/O-Pin should be set to input right now!\r\n");
        else
            mComm->mLog("We either missed the start-bit, or just indicated an error-bit.\r\n");
    }
    #endif
}

void Communication::IOPin::setLevel(const bit_t bit)
{
    if(bit) SET_BIT(PORTB, PB6);    // Set Pin to high
    else    CLR_BIT(PORTB, PB6);    // Set Pin to low
}

void Communication::IOPin::setDirection(const PinDir direction)
{
    switch(direction)
    {
        case PinDir::INPUT:
            CLR_BIT(DDRB, DDB6);    // Set direction for I/O-Pin to input
            break;
        case PinDir::OUTPUT:
            SET_BIT(DDRB, DDB6);    // Set direction for I/O-Pin to output
            break;
        default:
            break;
    }
    mComm->mDirection = direction;  // Set mDirection
}

void Communication::IOPin::setInterrupt(const bool enabled)
{
    if(enabled)
    {
        SET_BIT(PCICR, PCIE1);      // Enable interrupts for pins PCINT15:8
        SET_BIT(PCMSK1, PCINT14);   // Enable interrupts for PinB6
    }
    else
    {
        CLR_BIT(PCICR, PCIE1);      // Disable interrupts for Pins PCINT15:8    
        CLR_BIT(PCMSK1, PCINT14);   // Disable interrupts for PinB6
    }
}

// **********************************************************************************
// Communication Methods ************************************************************
// **********************************************************************************
constexpr byte_t Protocol::ATR_SEQ[];
constexpr byte_t Protocol::DATA_IN_HEADER[];
constexpr byte_t Protocol::DATA_OUT_HEADER[];
constexpr byte_t Protocol::RESPONSE_DECRYPTED[];
constexpr byte_t Protocol::RESPONSE_DATA_OUT[];

// **********************************************************************************
// Public Methods *******************************************************************
// **********************************************************************************

Communication::Communication()
{
    // Init Timer class *************************************************************
    Timer::init(this);
    // Init IOPin class *************************************************************
    IOPin::init(this);
}

void Communication::receiveDataToDecrypt(byte_t *data)
{
    // Receive header
    receiveProtocolHeader(Protocol::DATA_IN_HEADER);
    // Receive key byte by byte & send ACK
    for(uint8_t i=0; i<KEY_BYTES; i++)
    {
        sendByte(Protocol::ACK_DATA_IN);    // Send OxEF (the last byte will not have an ACK)
        data[i] = receiveByte();            // Receive byte
    }
}

void Communication::sendDecryptedData(const byte_t *data)
{
    // Send indication that the decryption is done
    sendBytes(Protocol::RESPONSE_DECRYPTED, Protocol::RESPONSE_LENGTH);
    // Receive header
    receiveProtocolHeader(Protocol::DATA_OUT_HEADER);
    // Send Acknowledge
    sendByte(Protocol::ACK_DATA_OUT);
    // Send decrypted data
    sendBytes(data, KEY_BYTES);
    // Send response after sending data
    sendBytes(Protocol::RESPONSE_DATA_OUT, Protocol::RESPONSE_LENGTH);
}

// **********************************************************************************
// Private Methods ******************************************************************
// **********************************************************************************

void Communication::sendBit(const bit_t bit)
{
    while(!mBitSent);   // Wait for the last bit to be sent
    mBitSent = false;   // Set #mBitSent to false
    mOutputBit = bit;   // Send next bit
}

void Communication::sendByte(const byte_t byte)
{   
    IOPin::setInterrupt(false);             // Disable interrupt for I/O-Pin    
    
    // Send byte at least once & re-send it in case of failure
    do
    {
        // Set I/O-Pin to output ****************************************************
        IOPin::setDirection(PinDir::OUTPUT);
        // Start timer **************************************************************
        Timer::start();                     // start the 16-bit timer
        Timer::setMatchValue(Timer::ETU);   // Set match value to 372
        // Send bits ****************************************************************
        sendBit(START_BIT);                 // Send start bit (0)
        for(byte_t mask=0x01; mask!=0x00; mask<<=1)
            sendBit(byte & mask);           // Send each data bit individually
        sendBit(getParity(byte));           // Send parity (1 if number of set bits is odd, 0 otherwise)
        sendBit(STOP_BIT);                  // Send stop bit (1)
        while(!mBitSent);                   // Assure that stop bit was sent correctly
        
        // Check for errors *********************************************************
        mCheckErrors = true;
        mErrorBit = 1;                      // Set error bit to true, to ensure it is read correctly
        Timer::setMatchValue(Timer::ETU-50);// Set match value to 372-50 to make sure we don't miss the error indication
        IOPin::setDirection(PinDir::INPUT); // Now receiving data
        Timer::start();                     // Start timer to check for errors
        while(mCheckErrors);                // Wait until the error-bit is read
    } while(!mErrorBit);

    // End of transfer **************************************************************
    Timer::stop();                          // Stop the timer
}

bit_t Communication::sampleBit()
{
    int8_t majority = 0;
    // Sample 3x & make majority decision (see Wolfgang Rank, Smart Card Handbook p. 247)
    majority += GET_BIT(PINB, PINB6) ? 1 : -1;
    majority += GET_BIT(PINB, PINB6) ? 1 : -1;
    majority += GET_BIT(PINB, PINB6) ? 1 : -1;
    return (majority > 0);
}

byte_t Communication::receiveByte()
{
    // Set I/O-Pin to input & enable interrupt
    IOPin::setDirection(PinDir::INPUT);
    IOPin::setInterrupt(true);
    mByteReceived = false;
    // Wait until the byte was received correctly
    while(!mByteReceived);
    return mInputByte;
}

void Communication::receiveProtocolHeader(const byte_t *header)
{
    #ifdef DEBUG
    byte_t receivedByte = 0x00;
    char msg[70];
    #endif
    for(uint8_t i=0; i<Protocol::HEADER_LENGTH; i++)
    {
        // Some debugging output
        #ifdef DEBUG
        receivedByte = receiveByte();
        if(receivedByte != header[i])
        {
            sprintf(msg, "Received wrong byte 0x%X instead of 0x%X at sequence position %d.\r\n", receivedByte, header[i], i);
            mLog(msg);
        }
        #else
        receiveByte();
        #endif
    }
}

bit_t Communication::getParity(byte_t byte)
{
    byte ^= byte >> 4;
    byte ^= byte >> 2;
    byte ^= byte >> 1;
    return !((~byte) & 0x01);
}