/**
 * ir_nec.h - NEC IR Protocol Decoder Library
 * 
 * Created: Refactored from ATTiny13_IR_NEC_Decoder.c
 * Author: Nghia Taarabt
 */

#ifndef IR_NEC_H_
#define IR_NEC_H_

#include <avr/io.h>
#include <stdint.h>

// IR Protocol States
typedef enum {
    IR_STATE_IDLE       = 0x0U,
    IR_STATE_INIT       = 0x1U,
    IR_STATE_FINISH     = 0x2U,
    IR_STATE_PROCESS    = 0x3U,
} IR_State_t;

// IR Protocol Events
#define IR_PROTO_EVENT_INIT     (0)
#define IR_PROTO_EVENT_DATA     (1)
#define IR_PROTO_EVENT_FINISH   (2)
#define IR_PROTO_EVENT_HOOK     (3)

// NEC Protocol Timing Constants (for 38.222kHz)
#define IR_NEC_START_BURST_MIN      (655U)
#define IR_NEC_START_BURST_MAX      (815U)
#define IR_NEC_START_SPACE_MIN      (330U)
#define IR_NEC_START_SPACE_MAX      (360U)
#define IR_NEC_REPEAT_SPACE_MIN     (155U)
#define IR_NEC_REPEAT_SPACE_MAX     (185U)
#define IR_NEC_BIT_NUM_MAX          (32U)
#define IR_NEC_TIMEOUT              (7400U)

// NEC Protocol Macros
#define isNEC_START_BURST(time)     (time > IR_NEC_START_BURST_MIN && time < IR_NEC_START_BURST_MAX)
#define isNEC_START_SPACE(time)     (time > IR_NEC_START_SPACE_MIN && time < IR_NEC_START_SPACE_MAX)
#define isNEC_REPEAT_SPACE(time)    (time > IR_NEC_REPEAT_SPACE_MIN && time < IR_NEC_REPEAT_SPACE_MAX)

// Return Values
#define IR_SUCCESS  (0)
#define IR_ERROR    (1)

// Logic Levels
#define LOW     (0)
#define HIGH    (1)

// Global Variables (extern declarations)
extern volatile uint16_t IR_timeout;
extern volatile uint16_t IR_Counter;
extern volatile uint32_t IR_rawdata;

// Function Declarations
void IR_init(void);
void IR_process(uint8_t pinIRValue);
int8_t IR_read(uint8_t *address, uint8_t *command);
int8_t IR_NEC_process(uint16_t counter, uint8_t value);

#endif /* IR_NEC_H_ */
