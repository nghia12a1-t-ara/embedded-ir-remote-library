/**
 * main.c - ATTiny13 IR Remote Control LED Controller
 * 
 * Uses generic IR decoder library with ATTiny13 HAL
 * Created: Refactored to use generic IR decoder
 * Author: Nghia Taarabt
 * 
 * FUSE_L=0x7A
 * FUSE_H=0xFF
 * F_CPU=9600000
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ir_decoder.h"
#include "attiny13_hal.h"

// LED Pin Definitions
#define LED1_PIN    PB0
#define LED2_PIN    PB2
#define LED3_PIN    PB3
#define LED4_PIN    PB4

// Remote Control Commands
#define CMD_ALL_OFF     0x01
#define CMD_LED1_TOGGLE 0x00
#define CMD_LED2_TOGGLE 0x07
#define CMD_LED3_TOGGLE 0x06
#define CMD_LED4_TOGGLE 0x04

// Expected Remote Address
#define REMOTE_ADDRESS  0x01

// Protocol Selection - Change this to use different IR protocols
#define SELECTED_PROTOCOL   IR_PROTOCOL_NEC  // Can be changed to any supported protocol

// Protocol-specific command mappings
typedef struct {
    IR_Protocol_t protocol;
    uint8_t expected_address;
    uint8_t cmd_all_off;
    uint8_t cmd_led1;
    uint8_t cmd_led2;
    uint8_t cmd_led3;
    uint8_t cmd_led4;
} Protocol_Commands_t;

static const Protocol_Commands_t protocol_commands[] = {
    // NEC Protocol commands
    {IR_PROTOCOL_NEC, 0x01, 0x01, 0x00, 0x07, 0x06, 0x04},
    // Samsung Protocol commands (example mapping)
    {IR_PROTOCOL_SAMSUNG, 0x07, 0x02, 0x0C, 0x0D, 0x0E, 0x0F},
    // Sony Protocol commands (example mapping)
    {IR_PROTOCOL_SONY, 0x01, 0x15, 0x00, 0x01, 0x02, 0x03},
    // LG Protocol commands (example mapping)
    {IR_PROTOCOL_LG, 0x04, 0x08, 0x00, 0x01, 0x02, 0x03},
    // JVC Protocol commands (example mapping)
    {IR_PROTOCOL_JVC, 0xC1, 0x01, 0x02, 0x03, 0x04, 0x05},
};

static IR_Decoder_t ir_decoder;
static IR_HAL_t ir_hal;
static const Protocol_Commands_t* current_protocol_commands = NULL;

void hardware_init(void)
{
    // Configure LED pins as outputs
    DDRB |= _BV(LED1_PIN) | _BV(LED2_PIN) | _BV(LED3_PIN) | _BV(LED4_PIN);
    
    // Initialize all LEDs to OFF state
    PORTB &= ~(_BV(LED1_PIN) | _BV(LED2_PIN) | _BV(LED3_PIN) | _BV(LED4_PIN));
}

const Protocol_Commands_t* find_protocol_commands(IR_Protocol_t protocol)
{
    for(uint8_t i = 0; i < sizeof(protocol_commands) / sizeof(Protocol_Commands_t); i++)
    {
        if(protocol_commands[i].protocol == protocol)
        {
            return &protocol_commands[i];
        }
    }
    return &protocol_commands[0]; // Default to NEC if not found
}

void process_ir_command(uint8_t address, uint8_t command)
{
    // Only process commands from the expected remote address
    if(!current_protocol_commands || address != current_protocol_commands->expected_address)
        return;
        
    if(command == current_protocol_commands->cmd_all_off)
    {
        // Turn all LEDs off
        PORTB &= ~(_BV(LED1_PIN) | _BV(LED2_PIN) | _BV(LED3_PIN) | _BV(LED4_PIN));
    }
    else if(command == current_protocol_commands->cmd_led1)
    {
        // Toggle LED1
        PORTB ^= _BV(LED1_PIN);
    }
    else if(command == current_protocol_commands->cmd_led2)
    {
        // Toggle LED2
        PORTB ^= _BV(LED2_PIN);
    }
    else if(command == current_protocol_commands->cmd_led3)
    {
        // Toggle LED3
        PORTB ^= _BV(LED3_PIN);
    }
    else if(command == current_protocol_commands->cmd_led4)
    {
        // Toggle LED4
        PORTB ^= _BV(LED4_PIN);
    }
    // Unknown command - do nothing
}

ISR(INT0_vect)
{
    uint8_t pin_value = attiny13_pin_read();
    IR_decoder_process(&ir_decoder, pin_value);
}

ISR(TIM0_COMPA_vect)
{
    attiny13_timer_interrupt();
    IR_decoder_timeout_handler(&ir_decoder);
}

int main(void)
{
    IR_Data_t ir_data;
    
    // Initialize hardware
    hardware_init();
    
    current_protocol_commands = find_protocol_commands(SELECTED_PROTOCOL);
    
    attiny13_hal_init(&ir_hal);
    IR_decoder_init(&ir_decoder, SELECTED_PROTOCOL, &ir_hal);
    
    // Main application loop
    while(1)
    {
        // Check for received IR data
        if(IR_decoder_get_data(&ir_decoder, &ir_data) == IR_SUCCESS)
        {
            // Process the received command
            process_ir_command(ir_data.address, ir_data.command);
        }
        
        // Small delay to prevent excessive polling
        _delay_ms(1);
    }
    
    return 0;
}
