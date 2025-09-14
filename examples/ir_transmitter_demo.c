/**
 * ir_transmitter_demo.c - IR Transmitter Demo for ATTiny13
 * 
 * Demonstrates how to use the IR transmitter library with different protocols
 * Created: Example for IR transmitter usage
 * Author: Nghia Taarabt
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../ir_transmitter.h"
#include "../attiny13_hal.h"

#define BUTTON_PIN      PB2
#define LED_PIN         PB4

// Global transmitter instance
IR_Transmitter_t ir_transmitter;
IR_TX_HAL_t tx_hal;

// Button debounce variables
volatile uint8_t button_pressed = 0;
volatile uint8_t button_debounce = 0;

void setup_hardware(void)
{
    // Configure button pin
    DDRB &= ~_BV(BUTTON_PIN);       // Button as input
    PORTB |= _BV(BUTTON_PIN);       // Enable pull-up
    
    // Configure LED pin
    DDRB |= _BV(LED_PIN);           // LED as output
    PORTB &= ~_BV(LED_PIN);         // LED off initially
    
    // Initialize transmitter HAL
    attiny13_tx_hal_init(&tx_hal);
    
    // Initialize IR transmitter with NEC protocol
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_NEC, &tx_hal);
    
    sei();  // Enable global interrupts
}

void send_ir_command(uint8_t address, uint8_t command)
{
    // Turn on LED to indicate transmission
    PORTB |= _BV(LED_PIN);
    
    // Send IR command
    if (IR_transmitter_send(&ir_transmitter, address, command) == IR_SUCCESS) {
        // Wait for transmission to complete
        while (IR_transmitter_is_busy(&ir_transmitter)) {
            _delay_ms(1);
        }
    }
    
    // Turn off LED
    PORTB &= ~_BV(LED_PIN);
}

void demo_multiple_protocols(void)
{
    // Demo NEC protocol
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_NEC, &tx_hal);
    send_ir_command(0x01, 0x00);  // Address 1, Command 0
    _delay_ms(500);
    
    // Demo Sony protocol
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_SONY, &tx_hal);
    send_ir_command(0x01, 0x15);  // Sony TV power
    _delay_ms(500);
    
    // Demo Samsung protocol
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_SAMSUNG, &tx_hal);
    send_ir_command(0x07, 0x02);  // Samsung command
    _delay_ms(500);
    
    // Demo RC5 protocol
    IR_transmitter_init(&ir_transmitter, IR_PROTOCOL_RC5, &tx_hal);
    send_ir_command(0x00, 0x0C);  // RC5 power toggle
    _delay_ms(500);
}

int main(void)
{
    setup_hardware();
    
    // Demo sequence on startup
    _delay_ms(1000);
    demo_multiple_protocols();
    
    // Main loop - send command when button is pressed
    while(1)
    {
        // Check button state (active low with pull-up)
        if (!(PINB & _BV(BUTTON_PIN))) {
            if (!button_debounce) {
                button_debounce = 1;
                
                // Send NEC command
                send_ir_command(0x01, 0x00);
                
                // Wait for button release
                while (!(PINB & _BV(BUTTON_PIN))) {
                    _delay_ms(10);
                }
                
                button_debounce = 0;
            }
        }
        
        _delay_ms(10);
    }
    
    return 0;
}
