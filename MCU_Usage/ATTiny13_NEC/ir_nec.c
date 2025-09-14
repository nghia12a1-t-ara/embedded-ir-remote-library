/**
 * ir_nec.c - NEC IR Protocol Decoder Library Implementation
 * 
 * Created: Refactored from ATTiny13_IR_NEC_Decoder.c
 * Author: Nghia Taarabt
 */

#include "ir_nec.h"
#include <avr/interrupt.h>

// Hardware Configuration
#define IR_IN_PIN       PB1
#define IR_OCR0A        (122)

// Global Variables
volatile uint16_t IR_timeout = 0U;
volatile uint16_t IR_Counter = 0U;
volatile uint32_t IR_rawdata = 0U;

// Static Variables
static IR_State_t IR_State = IR_STATE_IDLE;
static uint8_t IR_proto_event = 0U;
static uint8_t IR_index = 0U;
static uint32_t IR_data = 0U;

void IR_init(void)
{
    // Configure IR input pin
    DDRB &= ~_BV(IR_IN_PIN);    // set IR IN pin as INPUT
    PORTB &= ~_BV(IR_IN_PIN);   // set LOW level to IR IN pin
    
    // Configure Timer0 for 38.222kHz
    TCCR0A |= _BV(WGM01);       // set timer counter mode to CTC
    TCCR0B |= _BV(CS00);        // set prescaler to 1
    TIMSK0 |= _BV(OCIE0A);      // enable Timer COMPA interrupt
    OCR0A = IR_OCR0A;           // set OCR0n to get ~38.222kHz timer frequency
    
    // Configure External Interrupt INT0
    GIMSK |= _BV(INT0);         // enable INT0 interrupt handler
    MCUCR &= ~_BV(ISC01);       // trigger INT0 interrupt on raising and falling edge
    MCUCR |= _BV(ISC00);
    
    sei();                      // enable global interrupts
}

int8_t IR_NEC_process(uint16_t counter, uint8_t value)
{
    int8_t retval = IR_ERROR;

    switch(IR_proto_event)
    {
        case IR_PROTO_EVENT_INIT:
            IR_data = IR_index = 0U;
            retval = IR_SUCCESS;
            break;
            
        case IR_PROTO_EVENT_DATA:
            /* Reading 4 octets (32bits) of data:
             1) the 8-bit address for the receiving device
             2) the 8-bit logical inverse of the address
             3) the 8-bit command
             4) the 8-bit logical inverse of the command
            Logical '0' – a 562.5µs pulse burst followed by a 562.5µs (<90 IR counter cycles) space
            Logical '1' – a 562.5µs pulse burst followed by a 1.6875ms(>=90 IR counter cycles) space */
            if(IR_index < IR_NEC_BIT_NUM_MAX)
            {
                if(value == HIGH)
                {
                    IR_data |= ((uint32_t)((counter < 90U) ? 0U : 1U) << IR_index++);
                    if(IR_index == IR_NEC_BIT_NUM_MAX)
                    {
                        IR_proto_event = IR_PROTO_EVENT_HOOK;
                    }
                }
                retval = IR_SUCCESS;
            }
            break;
            
        case IR_PROTO_EVENT_HOOK:
            /* expecting a final 562.5µs pulse burst to signify the end of message transmission */
            if(value == LOW)
            {
                IR_proto_event = IR_PROTO_EVENT_FINISH;
                retval = IR_SUCCESS;
            }
            break;
            
        case IR_PROTO_EVENT_FINISH:
            /* copying data to volatile variable; raw data is ready */
            IR_rawdata = IR_data;
            break;
            
        default:
            break;
    }

    return retval;
}

void IR_process(uint8_t pinIRValue)
{
    /* load IR counter value to local variable, then reset counter */
    uint16_t counter = IR_Counter;
    IR_Counter = 0;

    switch(IR_State)
    {
        case IR_STATE_IDLE:
            /* awaiting for an initial signal */
            if(pinIRValue == HIGH)
            {
                IR_State = IR_STATE_INIT;
            }
            break;
            
        case IR_STATE_INIT:
            /* consume leading pulse burst */
            if(pinIRValue == LOW)
            {
                if(!isNEC_START_BURST(counter))
                {
                    IR_State = IR_STATE_FINISH;
                }
                IR_timeout = IR_NEC_TIMEOUT;
            }
            else    /* pinIRValue == HIGH */
            {
                if(isNEC_START_SPACE(counter))
                {
                    IR_State = IR_STATE_PROCESS;
                    IR_proto_event = IR_PROTO_EVENT_INIT;
                }
                else if(isNEC_REPEAT_SPACE(counter))
                {
                    IR_proto_event = IR_PROTO_EVENT_FINISH;
                }
                else
                {
                    IR_State = IR_STATE_FINISH;
                }
            }
            break;
            
        case IR_STATE_PROCESS:
            /* read and decode NEC Protocol data */
            if(IR_SUCCESS != IR_NEC_process(counter, pinIRValue))
            {
                IR_State = IR_STATE_FINISH;
            }
            break;
            
        case IR_STATE_FINISH:
            /* clear timeout and set idle mode */
            IR_State = IR_STATE_IDLE;
            IR_timeout = 0U;
            break;
            
        default:
            break;
    }
}

int8_t IR_read(uint8_t *address, uint8_t *command)
{
    if(!IR_rawdata)
        return IR_ERROR;

    *address = IR_rawdata;
    *command = IR_rawdata >> 16;
    IR_rawdata = 0;

    return IR_SUCCESS;
}

// Interrupt Service Routines
ISR(INT0_vect)
{
    uint8_t pinIRValue;
    /* read IR_IN_PIN digital value (NOTE: logical inverse value = value ^ 1 due to sensor used) */
    pinIRValue = ((PINB & (1 << IR_IN_PIN)) > 0) ? LOW : HIGH;
    IR_process(pinIRValue);
}

ISR(TIM0_COMPA_vect)
{
    /* When transmitting or receiving remote control codes using the NEC IR transmission protocol,
    the communications performs optimally when the carrier frequency (used for modulation/demodulation)
    is set to 38.222kHz. */
    if(IR_Counter++ > 10000)
        IR_State = IR_STATE_IDLE;
    if(IR_timeout && --IR_timeout == 0)
        IR_State = IR_STATE_IDLE;
}
