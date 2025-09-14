/**
 * ir_decoder.h - Generic IR Protocol Decoder Library
 * 
 * Hardware-independent IR decoder that supports multiple protocols
 * Created: Refactored from ir_nec.h for better portability
 * Author: Nghia Taarabt
 */

#ifndef IR_DECODER_H_
#define IR_DECODER_H_

#include "ir_common.h"

// Generic IR Protocol States
typedef enum {
    IR_STATE_IDLE       = 0x0U,
    IR_STATE_INIT       = 0x1U,
    IR_STATE_FINISH     = 0x2U,
    IR_STATE_PROCESS    = 0x3U,
} IR_State_t;

// Generic IR Protocol Events
typedef enum {
    IR_EVENT_INIT       = 0,
    IR_EVENT_DATA       = 1,
    IR_EVENT_FINISH     = 2,
    IR_EVENT_HOOK       = 3,
} IR_Event_t;

typedef struct {
    uint16_t start_burst_min;
    uint16_t start_burst_max;
    uint16_t start_space_min;
    uint16_t start_space_max;
    uint16_t repeat_space_min;
    uint16_t repeat_space_max;
    uint8_t bit_count;
    uint16_t timeout;
    uint16_t bit_threshold;  // Threshold for distinguishing 0 and 1
} IR_Protocol_Config_t;

// Hardware Abstraction Layer - Function Pointers for Decoder
typedef struct {
    void (*timer_start)(void);
    void (*timer_stop)(void);
    uint16_t (*timer_get_count)(void);
    void (*timer_reset_count)(void);
    uint8_t (*pin_read)(void);
} IR_HAL_t;

// IR Decoder Context Structure
typedef struct {
    IR_State_t state;
    IR_Event_t event;
    uint8_t bit_index;
    uint32_t data_buffer;
    uint16_t timeout_counter;
    IR_Protocol_t protocol_type;
    IR_HAL_t hal;
    IR_Data_t decoded_data;
    IR_Protocol_Config_t protocol_config;  // Added missing protocol config field
} IR_Decoder_t;

// Function Declarations
void IR_decoder_init(IR_Decoder_t* decoder, IR_Protocol_t protocol, IR_HAL_t* hal);
void IR_decoder_process(IR_Decoder_t* decoder, uint8_t pin_value);
int8_t IR_decoder_get_data(IR_Decoder_t* decoder, IR_Data_t* data);
void IR_decoder_timeout_handler(IR_Decoder_t* decoder);
void IR_decoder_reset(IR_Decoder_t* decoder);

static int8_t IR_process_protocol_data(IR_Decoder_t* decoder, uint16_t counter, uint8_t value);

#endif /* IR_DECODER_H_ */
