/**
 * attiny13_hal.c - Hardware Abstraction Layer Implementation for ATTiny13
 * 
 * ATTiny13-specific implementation of IR decoder and transmitter HAL
 * Created: New file for hardware abstraction
 * Author: Nghia Taarabt
 */

#include "attiny13_hal.h"
#include <util/delay.h>

volatile uint16_t attiny13_ir_counter = 0U;
volatile uint16_t attiny13_ir_timeout = 0U;

void attiny13_timer_start(void)
{
    // Configure Timer0 for 38.222kHz
    TCCR0A |= _BV(WGM01);       // set timer counter mode to CTC
    TCCR0B |= _BV(CS00);        // set prescaler to 1
    TIMSK0 |= _BV(OCIE0A);      // enable Timer COMPA interrupt
    OCR0A = IR_OCR0A;           // set OCR0n to get ~38.222kHz timer frequency
}

void attiny13_timer_stop(void)
{
    TCCR0B &= ~_BV(CS00);       // stop timer
    TIMSK0 &= ~_BV(OCIE0A);     // disable Timer COMPA interrupt
}

uint16_t attiny13_timer_get_count(void)
{
    return attiny13_ir_counter;
}

void attiny13_timer_reset_count(void)
{
    attiny13_ir_counter = 0;
}

uint8_t attiny13_pin_read(void)
{
    // Read IR_IN_PIN digital value (logical inverse due to sensor used)
    return ((PINB & (1 << IR_IN_PIN)) > 0) ? IR_LOW : IR_HIGH;
}

// Transmitter HAL Functions
void attiny13_carrier_on(void)
{
    // Configure Timer0 for 38kHz PWM on OC0A (PB0)
    DDRB |= _BV(IR_OUT_PIN);        // Set IR output pin as OUTPUT
    TCCR0A |= _BV(COM0A0) | _BV(WGM01);  // Toggle OC0A on compare match, CTC mode
    TCCR0B |= _BV(CS00);            // No prescaler
    OCR0A = 125;                    // 9.6MHz / (2 * 126) = ~38kHz
}

void attiny13_carrier_off(void)
{
    TCCR0A &= ~_BV(COM0A0);         // Disconnect OC0A
    PORTB &= ~_BV(IR_OUT_PIN);      // Set IR output pin LOW
}

void attiny13_delay_us(uint16_t us)
{
    while(us--) {
        _delay_us(1);
    }
}

void attiny13_delay_ms(uint16_t ms)
{
    while(ms--) {
        _delay_ms(1);
    }
}

void attiny13_hal_init(IR_HAL_t* hal)
{
    // Configure IR input pin
    DDRB &= ~_BV(IR_IN_PIN);    // set IR IN pin as INPUT
    PORTB &= ~_BV(IR_IN_PIN);   // set LOW level to IR IN pin
    
    // Configure External Interrupt INT0
    GIMSK |= _BV(INT0);         // enable INT0 interrupt handler
    MCUCR &= ~_BV(ISC01);       // trigger INT0 interrupt on raising and falling edge
    MCUCR |= _BV(ISC00);
    
    // Initialize HAL function pointers
    hal->timer_start = attiny13_timer_start;
    hal->timer_stop = attiny13_timer_stop;
    hal->timer_get_count = attiny13_timer_get_count;
    hal->timer_reset_count = attiny13_timer_reset_count;
    hal->pin_read = attiny13_pin_read;
    
    sei();  // enable global interrupts
}

void attiny13_tx_hal_init(IR_TX_HAL_t* tx_hal)
{
    // Configure IR output pin
    DDRB |= _BV(IR_OUT_PIN);        // Set IR OUT pin as OUTPUT
    PORTB &= ~_BV(IR_OUT_PIN);      // Set LOW level initially
    
    // Initialize transmitter HAL function pointers
    tx_hal->carrier_on = attiny13_carrier_on;
    tx_hal->carrier_off = attiny13_carrier_off;
    tx_hal->delay_us = attiny13_delay_us;
    tx_hal->delay_ms = attiny13_delay_ms;
}

void attiny13_ir_pin_interrupt(void)
{
    // This function should be called from the actual ISR in main.c
    // The pin reading is handled by the HAL pin_read function
}

void attiny13_timer_interrupt(void)
{
    // Timer interrupt handler for 38.222kHz carrier frequency
    if(attiny13_ir_counter++ > 10000)
        attiny13_ir_counter = 0;  // Prevent overflow
        
    if(attiny13_ir_timeout && --attiny13_ir_timeout == 0)
        attiny13_ir_timeout = 0;  // Timeout expired
}
