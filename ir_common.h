/**
 * ir_common.h - Common IR Protocol Definitions and Utilities
 * 
 * Shared definitions and utilities for both IR decoder and transmitter
 * Created: Extracted common code from ir_decoder.h and ir_transmitter.h
 * Author: Nghia Taarabt
 */

#ifndef IR_COMMON_H_
#define IR_COMMON_H_

#include <stdint.h>

// Return Values
#define IR_SUCCESS  (0)
#define IR_ERROR    (1)

// Logic Levels
#define IR_LOW      (0)
#define IR_HIGH     (1)

// IR Protocol Types
typedef enum {
    IR_PROTOCOL_NEC     = 0,
    IR_PROTOCOL_RC5     = 1,
    IR_PROTOCOL_SONY    = 2,
    IR_PROTOCOL_RC6     = 3,
    IR_PROTOCOL_SAMSUNG = 4,
    IR_PROTOCOL_LG      = 5,
    IR_PROTOCOL_PANASONIC = 6,
    IR_PROTOCOL_JVC     = 7,
    IR_PROTOCOL_DENON   = 8,
    IR_PROTOCOL_COUNT   = 9
} IR_Protocol_t;

// IR Data Structure (used by both decoder and transmitter)
typedef struct {
    uint32_t raw_data;
    uint8_t address;
    uint8_t command;
    uint8_t protocol;
    uint8_t valid;
} IR_Data_t;

// Common Protocol Timing Structure (in timer counts for decoder, microseconds for transmitter)
typedef struct {
    uint16_t start_burst_min;
    uint16_t start_burst_max;
    uint16_t start_space_min;
    uint16_t start_space_max;
    uint16_t repeat_space_min;
    uint16_t repeat_space_max;
    uint16_t bit_burst_min;
    uint16_t bit_burst_max;
    uint16_t bit_0_space_min;
    uint16_t bit_0_space_max;
    uint16_t bit_1_space_min;
    uint16_t bit_1_space_max;
    uint16_t stop_burst_min;
    uint16_t stop_burst_max;
    uint8_t bit_count;
    uint16_t timeout;
    uint32_t carrier_freq;  // Carrier frequency in Hz
} IR_Protocol_Timing_t;

// Protocol Information Structure
typedef struct {
    IR_Protocol_t type;
    const char* name;
    IR_Protocol_Timing_t timing;
} IR_Protocol_Info_t;

// Function Declarations
const IR_Protocol_Info_t* IR_get_protocol_info(IR_Protocol_t protocol);
const char* IR_get_protocol_name(IR_Protocol_t protocol);
uint32_t IR_get_carrier_frequency(IR_Protocol_t protocol);

// Data encoding/decoding utilities
uint32_t IR_encode_nec_data(uint8_t address, uint8_t command);
uint32_t IR_encode_sony_data(uint8_t address, uint8_t command);
uint32_t IR_encode_rc5_data(uint8_t address, uint8_t command);
uint32_t IR_encode_samsung_data(uint8_t address, uint8_t command);
uint32_t IR_encode_lg_data(uint8_t address, uint8_t command);
uint32_t IR_encode_panasonic_data(uint8_t address, uint8_t command);
uint32_t IR_encode_jvc_data(uint8_t address, uint8_t command);
uint32_t IR_encode_rc6_data(uint8_t address, uint8_t command);
uint32_t IR_encode_denon_data(uint8_t address, uint8_t command);

void IR_decode_nec_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_sony_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_rc5_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_samsung_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_lg_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_panasonic_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_jvc_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_rc6_data(uint32_t raw_data, uint8_t* address, uint8_t* command);
void IR_decode_denon_data(uint32_t raw_data, uint8_t* address, uint8_t* command);

// Utility functions
uint8_t IR_validate_protocol_data(IR_Protocol_t protocol, uint32_t raw_data);
uint16_t IR_calculate_checksum(uint32_t data);

// IR Transmitter Protocol Configuration (timing in microseconds)
typedef struct {
    uint16_t start_burst_us;    // Start burst duration in microseconds
    uint16_t start_space_us;    // Start space duration in microseconds
    uint16_t repeat_space_us;   // Repeat space duration in microseconds (0 if no repeat)
    uint16_t bit_burst_us;      // Bit burst duration in microseconds
    uint16_t bit_0_space_us;    // Logic 0 space duration in microseconds
    uint16_t bit_1_space_us;    // Logic 1 space duration in microseconds
    uint16_t stop_burst_us;     // Stop burst duration in microseconds (0 if no stop)
    uint8_t bit_count;          // Number of data bits
    uint8_t repeat_count;       // Number of repeat transmissions
    uint32_t carrier_freq;      // Carrier frequency in Hz
} IR_TX_Protocol_Config_t;

#endif /* IR_COMMON_H_ */
