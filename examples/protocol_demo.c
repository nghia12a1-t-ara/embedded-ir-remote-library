/**
 * protocol_demo.c - Demonstration of multiple IR protocol support
 * 
 * Shows how to use different IR protocols with the generic decoder
 * Author: Nghia Taarabt
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../ir_decoder.h"
#include "../attiny13_hal.h"

// Example: Auto-detect protocol by trying multiple decoders
#define MAX_PROTOCOLS 5

static IR_Decoder_t decoders[MAX_PROTOCOLS];
static IR_HAL_t ir_hal;
static IR_Protocol_t protocols[] = {
    IR_PROTOCOL_NEC,
    IR_PROTOCOL_SAMSUNG,
    IR_PROTOCOL_SONY,
    IR_PROTOCOL_LG,
    IR_PROTOCOL_JVC
};

void init_multi_protocol_decoder(void)
{
    attiny13_hal_init(&ir_hal);
    
    // Initialize decoders for different protocols
    for(uint8_t i = 0; i < MAX_PROTOCOLS; i++)
    {
        IR_decoder_init(&decoders[i], protocols[i], &ir_hal);
    }
}

void process_multi_protocol_ir(uint8_t pin_value)
{
    IR_Data_t ir_data;
    
    // Try all protocol decoders
    for(uint8_t i = 0; i < MAX_PROTOCOLS; i++)
    {
        IR_decoder_process(&decoders[i], pin_value);
        
        // Check if this decoder got valid data
        if(IR_decoder_get_data(&decoders[i], &ir_data) == IR_SUCCESS)
        {
            // Found valid data with this protocol
            // Reset other decoders to prevent false positives
            for(uint8_t j = 0; j < MAX_PROTOCOLS; j++)
            {
                if(j != i)
                    IR_decoder_reset(&decoders[j]);
            }
            
            // Process the command based on detected protocol
            switch(ir_data.protocol)
            {
                case IR_PROTOCOL_NEC:
                    // Handle NEC protocol commands
                    break;
                case IR_PROTOCOL_SAMSUNG:
                    // Handle Samsung protocol commands
                    break;
                case IR_PROTOCOL_SONY:
                    // Handle Sony protocol commands
                    break;
                case IR_PROTOCOL_LG:
                    // Handle LG protocol commands
                    break;
                case IR_PROTOCOL_JVC:
                    // Handle JVC protocol commands
                    break;
                default:
                    break;
            }
            break; // Exit loop once we found valid data
        }
    }
}

ISR(INT0_vect)
{
    uint8_t pin_value = attiny13_pin_read();
    process_multi_protocol_ir(pin_value);
}

ISR(TIM0_COMPA_vect)
{
    attiny13_timer_interrupt();
    
    // Handle timeouts for all decoders
    for(uint8_t i = 0; i < MAX_PROTOCOLS; i++)
    {
        IR_decoder_timeout_handler(&decoders[i]);
    }
}

int main(void)
{
    // Initialize hardware and multi-protocol decoder
    DDRB |= _BV(PB0) | _BV(PB2) | _BV(PB3) | _BV(PB4); // LED pins
    init_multi_protocol_decoder();
    
    while(1)
    {
        _delay_ms(10);
    }
    
    return 0;
}
