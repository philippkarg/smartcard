#include "logger.h"

void Logger::init()
{
    // Enable 2x speed
    SET_BIT(UCSR0A, U2X0);
    // Set baud rate
    UBRR0H = static_cast<uint8_t>(UBRR0_VALUE>>8);
    UBRR0L = static_cast<uint8_t>(UBRR0_VALUE);
    // Enable transmitter
    UCSR0B = (1<<TXEN0);
    // Set frame format: 8 data-bits, 1 stop-bit
    UCSR0C |= (3<<UCSZ00);
    CLR_BIT(UCSR0C, USBS0);
}

void Logger::operator()(const uint8_t *arr, const uint8_t len) const
{
    char msg[4];
    for(uint8_t i=0; i<len; i++)
    {
        sprintf(msg, " %x", arr[i]);
        sendStr(msg);
    }
    sendStr("\r\n");
}   

void Logger::sendChar(const char data) const
{
    // Wait for empty transmit buffer
    while (!(GET_BIT(UCSR0A, UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

void Logger::sendStr(const char* str) const
{
    while(*str != '\0')
        sendChar(*str++);
}