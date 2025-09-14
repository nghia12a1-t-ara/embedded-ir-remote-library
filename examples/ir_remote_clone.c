/**
 * ir_remote_clone.c - IR Remote Control Cloner
 * 
 * Demonstrates receiving IR commands and retransmitting them
 * Combines both decoder and transmitter functionality
 * Created: Example for IR cloning functionality
 * Author: Nghia Taarabt
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../ir_decoder.h"
#include "../ir_transmitter.h"
#include "../attiny13_hal.h"

#define LEARN_BUTTON_PIN    PB2
#define SEND_BUTTON_PIN     PB3
#define STATUS_LED_PIN      PB4

// Global instances
IR_Decoder_t ir_decoder;
IR_Transmitter_t ir_transmitter;
IR_HAL_t rx_hal;
IR_TX_HAL_t tx_hal;

// Learned command storage
typedef struct {
    uint8_t address;
    uint8_t command;
    IR_Protocol_t protocol;
    uint8_t valid;
} Learned_Command_t;

Learned_Command_t learned_commands[4];  // Store up to 4 commands
uint8_t current_slot = 0;

void setup_hardware(void)
{
    // Configure button pins
    DDRB &= ~(_BV(LEARN_BUTTON_PIN) | _BV(SEND_BUTTON_PIN));
    PORTB |= _BV(LEARN_BUTTON_PIN) | _BV(SEND_BUTTON_PIN);  // Enable pull-ups
    
    // Configure LED pin
    DDRB |= _BV(STATUS_LED_PIN);
    PORTB &= ~_BV(STATUS_LED_PIN);
    
    // Initialize HALs
    attiny13_hal_init(&rx_hal);
    attiny13_tx_hal_init(&tx_hal);
    
    // Initialize decoder and transmitter
    IR_decoder_init(&ir_decoder, IR_PROTOCOL_NEC, &rx_hal);
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_NEC, &tx_hal);
    
    // Clear learned commands
    for (uint8_t i = 0; i < 4; i++) {
        learned_commands[i].valid = 0;
    }
    
    sei();
}

void learn_mode(void)
{
    // Indicate learning mode with fast blinking LED
    for (uint8_t i = 0; i < 10; i++) {
        PORTB ^= _BV(STATUS_LED_PIN);
        _delay_ms(100);
    }
    
    PORTB |= _BV(STATUS_LED_PIN);  // LED on during learning
    
    // Wait for IR command (timeout after 10 seconds)
    uint16_t timeout = 10000;
    IR_Data_t received_data;
    
    while (timeout-- > 0) {
        if (IR_decoder_get_data(&ir_decoder, &received_data) == IR_SUCCESS) {
            // Store learned command
            learned_commands[current_slot].address = received_data.address;
            learned_commands[current_slot].command = received_data.command;
            learned_commands[current_slot].protocol = received_data.protocol;
            learned_commands[current_slot].valid = 1;
            
            // Indicate successful learning with 3 quick blinks
            PORTB &= ~_BV(STATUS_LED_PIN);
            for (uint8_t i = 0; i < 3; i++) {
                PORTB ^= _BV(STATUS_LED_PIN);
                _delay_ms(200);
                PORTB ^= _BV(STATUS_LED_PIN);
                _delay_ms(200);
            }
            
            current_slot = (current_slot + 1) % 4;  // Move to next slot
            return;
        }
        _delay_ms(1);
    }
    
    // Timeout - indicate failure with long blink
    PORTB &= ~_BV(STATUS_LED_PIN);
    _delay_ms(1000);
}

void send_learned_command(uint8_t slot)
{
    if (slot >= 4 || !learned_commands[slot].valid) {
        return;  // Invalid slot or no command learned
    }
    
    // Reconfigure transmitter for the learned protocol
    IR_transmitter_init(&ir_transmitter, learned_commands[slot].protocol, &tx_hal);
    
    // Send the learned command
    PORTB |= _BV(STATUS_LED_PIN);  // LED on during transmission
    
    IR_transmitter_send(&ir_transmitter, 
                       learned_commands[slot].address, 
                       learned_commands[slot].command);
    
    while (IR_transmitter_is_busy(&ir_transmitter)) {
        _delay_ms(1);
    }
    
    PORTB &= ~_BV(STATUS_LED_PIN);  // LED off
}

// Interrupt handlers
ISR(INT0_vect)
{
    uint8_t pin_value = ir_decoder.hal.pin_read();
    IR_decoder_process(&ir_decoder, pin_value);
}

ISR(TIM0_COMPA_vect)
{
    IR_decoder_timeout_handler(&ir_decoder);
}

int main(void)
{
    setup_hardware();
    
    while(1)
    {
        // Check learn button
        if (!(PINB & _BV(LEARN_BUTTON_PIN))) {
            _delay_ms(50);  // Debounce
            if (!(PINB & _BV(LEARN_BUTTON_PIN))) {
                learn_mode();
                while (!(PINB & _BV(LEARN_BUTTON_PIN)));  // Wait for release
                _delay_ms(50);
            }
        }
        
        // Check send button
        if (!(PINB & _BV(SEND_BUTTON_PIN))) {
            _delay_ms(50);  // Debounce
            if (!(PINB & _BV(SEND_BUTTON_PIN))) {
                send_learned_command(0);  // Send first learned command
                while (!(PINB & _BV(SEND_BUTTON_PIN)));  // Wait for release
                _delay_ms(50);
            }
        }
        
        _delay_ms(10);
    }
    
    return 0;
}
