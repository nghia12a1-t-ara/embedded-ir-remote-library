/**
 * ir_transmitter.h - Generic IR Protocol Transmitter Library
 * 
 * Hardware-independent IR transmitter that supports multiple protocols
 * Created: Complement to ir_decoder.h for IR signal generation
 * Author: Nghia Taarabt
 */

#ifndef IR_TRANSMITTER_H_
#define IR_TRANSMITTER_H_

#include "ir_common.h"

// IR Transmitter States
typedef enum {
    IR_TX_STATE_IDLE        = 0x0U,
    IR_TX_STATE_START_BURST = 0x1U,
    IR_TX_STATE_START_SPACE = 0x2U,
    IR_TX_STATE_DATA_BURST  = 0x3U,
    IR_TX_STATE_DATA_SPACE  = 0x4U,
    IR_TX_STATE_STOP_BURST  = 0x5U,
    IR_TX_STATE_COMPLETE    = 0x6U,
} IR_TX_State_t;

// Hardware Abstraction Layer for Transmitter
typedef struct {
    void (*carrier_on)(void);       // Start 38kHz carrier
    void (*carrier_off)(void);      // Stop carrier
    void (*delay_us)(uint16_t us);  // Microsecond delay
    void (*delay_ms)(uint16_t ms);  // Millisecond delay
} IR_TX_HAL_t;

// IR Transmitter Context
typedef struct {
    IR_TX_State_t state;
    IR_Protocol_t protocol_type;
    IR_TX_HAL_t hal;
    IR_TX_Protocol_Config_t protocol_config;  // Protocol timing configuration
    uint32_t data_to_send;
    uint8_t current_bit;
    uint8_t repeat_counter;
    uint8_t is_transmitting;
} IR_Transmitter_t;

// Function Declarations
void IR_transmitter_init(IR_Transmitter_t* transmitter, IR_Protocol_t protocol, IR_TX_HAL_t* hal);
int8_t IR_transmitter_send(IR_Transmitter_t* transmitter, uint8_t address, uint8_t command);
int8_t IR_transmitter_send_raw(IR_Transmitter_t* transmitter, uint32_t raw_data);
int8_t IR_transmitter_send_repeat(IR_Transmitter_t* transmitter);
uint8_t IR_transmitter_is_busy(IR_Transmitter_t* transmitter);
void IR_transmitter_stop(IR_Transmitter_t* transmitter);

static void IR_transmit_frame(IR_Transmitter_t* transmitter);

#endif /* IR_TRANSMITTER_H_ */
