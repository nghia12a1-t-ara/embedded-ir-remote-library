/**
 * attiny13_hal.h - Hardware Abstraction Layer for ATTiny13
 * 
 * ATTiny13-specific implementation of IR decoder HAL
 * Created: New file for hardware abstraction
 * Author: Nghia Taarabt
 */

#ifndef ATTINY13_HAL_H_
#define ATTINY13_HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "ir_decoder.h"

// Hardware Configuration
#define IR_IN_PIN       PB1
#define IR_OCR0A        (122)
#define IR_OUT_PIN      PB0     // IR LED output pin

// Global variables for ATTiny13 HAL
extern volatile uint16_t attiny13_ir_counter;
extern volatile uint16_t attiny13_ir_timeout;

// HAL Function Declarations
void attiny13_timer_start(void);
void attiny13_timer_stop(void);
uint16_t attiny13_timer_get_count(void);
void attiny13_timer_reset_count(void);
uint8_t attiny13_pin_read(void);

// Transmitter HAL Function Declarations
void attiny13_carrier_on(void);
void attiny13_carrier_off(void);
void attiny13_delay_us(uint16_t us);
void attiny13_delay_ms(uint16_t ms);

// HAL Initialization
void attiny13_hal_init(IR_HAL_t* hal);

// Transmitter HAL Initialization
void attiny13_tx_hal_init(IR_TX_HAL_t* tx_hal);

// Interrupt handlers (to be called from main application)
void attiny13_ir_pin_interrupt(void);
void attiny13_timer_interrupt(void);

#endif /* ATTINY13_HAL_H_ */
