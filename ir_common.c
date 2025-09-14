/**
 * ir_common.c - Common IR Protocol Definitions and Utilities Implementation
 * 
 * Shared implementations for both IR decoder and transmitter
 * Created: Extracted common code from ir_decoder.c and ir_transmitter.c
 * Author: Nghia Taarabt
 */

#include "ir_common.h"

// Protocol timing definitions (in timer counts for 38.222kHz)
static const IR_Protocol_Info_t protocol_info_table[IR_PROTOCOL_COUNT] = {
    // NEC Protocol
    {
        .type = IR_PROTOCOL_NEC,
        .name = "NEC",
        .timing = {
            .start_burst_min = 655, .start_burst_max = 815,     // 9ms burst
            .start_space_min = 330, .start_space_max = 360,     // 4.5ms space
            .repeat_space_min = 155, .repeat_space_max = 185,   // 2.25ms repeat
            .bit_burst_min = 15, .bit_burst_max = 25,           // 562.5µs burst
            .bit_0_space_min = 15, .bit_0_space_max = 25,       // 562.5µs space (0)
            .bit_1_space_min = 45, .bit_1_space_max = 55,       // 1.6875ms space (1)
            .stop_burst_min = 15, .stop_burst_max = 25,         // 562.5µs stop
            .bit_count = 32,
            .timeout = 7400,
            .carrier_freq = 38000
        }
    },
    // RC5 Protocol
    {
        .type = IR_PROTOCOL_RC5,
        .name = "RC5",
        .timing = {
            .start_burst_min = 25, .start_burst_max = 35,       // 889µs
            .start_space_min = 25, .start_space_max = 35,       // 889µs
            .bit_burst_min = 25, .bit_burst_max = 35,           // 889µs
            .bit_0_space_min = 25, .bit_0_space_max = 35,       // 889µs
            .bit_1_space_min = 25, .bit_1_space_max = 35,       // 889µs (Manchester)
            .bit_count = 14,
            .timeout = 3000,
            .carrier_freq = 36000
        }
    },
    // Sony SIRC Protocol
    {
        .type = IR_PROTOCOL_SONY,
        .name = "Sony",
        .timing = {
            .start_burst_min = 85, .start_burst_max = 105,      // 2.4ms burst
            .start_space_min = 20, .start_space_max = 30,       // 600µs space
            .bit_burst_min = 20, .bit_burst_max = 30,           // 600µs burst
            .bit_0_space_min = 20, .bit_0_space_max = 30,       // 600µs space (0)
            .bit_1_space_min = 40, .bit_1_space_max = 50,       // 1.2ms space (1)
            .bit_count = 12,
            .timeout = 4500,
            .carrier_freq = 40000
        }
    },
    // RC6 Protocol
    {
        .type = IR_PROTOCOL_RC6,
        .name = "RC6",
        .timing = {
            .start_burst_min = 95, .start_burst_max = 115,      // 2.666ms burst
            .start_space_min = 30, .start_space_max = 40,       // 889µs space
            .bit_burst_min = 15, .bit_burst_max = 25,           // 444µs
            .bit_0_space_min = 15, .bit_0_space_max = 25,       // 444µs
            .bit_1_space_min = 30, .bit_1_space_max = 40,       // 889µs
            .bit_count = 21,
            .timeout = 5000,
            .carrier_freq = 36000
        }
    },
    // Samsung Protocol
    {
        .type = IR_PROTOCOL_SAMSUNG,
        .name = "Samsung",
        .timing = {
            .start_burst_min = 155, .start_burst_max = 175,     // 4.5ms burst
            .start_space_min = 155, .start_space_max = 175,     // 4.5ms space
            .bit_burst_min = 20, .bit_burst_max = 30,           // 590µs burst
            .bit_0_space_min = 20, .bit_0_space_max = 30,       // 590µs space (0)
            .bit_1_space_min = 55, .bit_1_space_max = 65,       // 1.69ms space (1)
            .bit_count = 32,
            .timeout = 8000,
            .carrier_freq = 38000
        }
    },
    // LG Protocol
    {
        .type = IR_PROTOCOL_LG,
        .name = "LG",
        .timing = {
            .start_burst_min = 310, .start_burst_max = 330,     // 9ms burst
            .start_space_min = 155, .start_space_max = 175,     // 4.5ms space
            .bit_burst_min = 20, .bit_burst_max = 30,           // 560µs burst
            .bit_0_space_min = 20, .bit_0_space_max = 30,       // 560µs space (0)
            .bit_1_space_min = 55, .bit_1_space_max = 65,       // 1.69ms space (1)
            .bit_count = 28,
            .timeout = 7500,
            .carrier_freq = 38000
        }
    },
    // Panasonic Protocol
    {
        .type = IR_PROTOCOL_PANASONIC,
        .name = "Panasonic",
        .timing = {
            .start_burst_min = 125, .start_burst_max = 145,     // 3.5ms burst
            .start_space_min = 60, .start_space_max = 80,       // 1.75ms space
            .bit_burst_min = 15, .bit_burst_max = 25,           // 435µs burst
            .bit_0_space_min = 15, .bit_0_space_max = 25,       // 435µs space (0)
            .bit_1_space_min = 45, .bit_1_space_max = 55,       // 1.3ms space (1)
            .bit_count = 48,
            .timeout = 9000,
            .carrier_freq = 37000
        }
    },
    // JVC Protocol
    {
        .type = IR_PROTOCOL_JVC,
        .name = "JVC",
        .timing = {
            .start_burst_min = 290, .start_burst_max = 310,     // 8.4ms burst
            .start_space_min = 145, .start_space_max = 165,     // 4.2ms space
            .bit_burst_min = 18, .bit_burst_max = 28,           // 525µs burst
            .bit_0_space_min = 18, .bit_0_space_max = 28,       // 525µs space (0)
            .bit_1_space_min = 55, .bit_1_space_max = 65,       // 1.575ms space (1)
            .bit_count = 16,
            .timeout = 6000,
            .carrier_freq = 38000
        }
    },
    // Denon Protocol
    {
        .type = IR_PROTOCOL_DENON,
        .name = "Denon",
        .timing = {
            .start_burst_min = 120, .start_burst_max = 140,     // 3.5ms burst
            .start_space_min = 60, .start_space_max = 80,       // 1.75ms space
            .bit_burst_min = 12, .bit_burst_max = 22,           // 350µs burst
            .bit_0_space_min = 12, .bit_0_space_max = 22,       // 350µs space (0)
            .bit_1_space_min = 35, .bit_1_space_max = 45,       // 1.05ms space (1)
            .bit_count = 15,
            .timeout = 5500,
            .carrier_freq = 38000
        }
    }
};

const IR_Protocol_Info_t* IR_get_protocol_info(IR_Protocol_t protocol)
{
    if (protocol >= IR_PROTOCOL_COUNT) {
        return NULL;
    }
    return &protocol_info_table[protocol];
}

const char* IR_get_protocol_name(IR_Protocol_t protocol)
{
    const IR_Protocol_Info_t* info = IR_get_protocol_info(protocol);
    return info ? info->name : "Unknown";
}

uint32_t IR_get_carrier_frequency(IR_Protocol_t protocol)
{
    const IR_Protocol_Info_t* info = IR_get_protocol_info(protocol);
    return info ? info->timing.carrier_freq : 38000;
}

// Data encoding functions
uint32_t IR_encode_nec_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)address) | 
           (((uint32_t)(~address)) << 8) | 
           (((uint32_t)command) << 16) | 
           (((uint32_t)(~command)) << 24);
}

uint32_t IR_encode_sony_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)command) | (((uint32_t)address) << 7);
}

uint32_t IR_encode_rc5_data(uint8_t address, uint8_t command)
{
    return (((uint32_t)address) << 6) | ((uint32_t)command) | 0x3000;
}

uint32_t IR_encode_samsung_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)address) | 
           (((uint32_t)address) << 8) | 
           (((uint32_t)command) << 16) | 
           (((uint32_t)(~command)) << 24);
}

uint32_t IR_encode_lg_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)address) | (((uint32_t)command) << 8) | 
           (((uint32_t)IR_calculate_checksum(((uint32_t)address) | (((uint32_t)command) << 8))) << 16);
}

uint32_t IR_encode_panasonic_data(uint8_t address, uint8_t command)
{
    return 0x40040100UL | (((uint32_t)address) << 8) | (((uint32_t)command) << 16);
}

uint32_t IR_encode_jvc_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)address) | (((uint32_t)command) << 8);
}

uint32_t IR_encode_rc6_data(uint8_t address, uint8_t command)
{
    return 0x100000UL | (((uint32_t)address) << 8) | ((uint32_t)command);
}

uint32_t IR_encode_denon_data(uint8_t address, uint8_t command)
{
    return ((uint32_t)address) | (((uint32_t)command) << 5);
}

// Data decoding functions
void IR_decode_nec_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)(raw_data & 0xFF);
    *command = (uint8_t)((raw_data >> 16) & 0xFF);
}

void IR_decode_sony_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *command = (uint8_t)(raw_data & 0x7F);
    *address = (uint8_t)((raw_data >> 7) & 0x1F);
}

void IR_decode_rc5_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *command = (uint8_t)(raw_data & 0x3F);
    *address = (uint8_t)((raw_data >> 6) & 0x1F);
}

void IR_decode_samsung_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)(raw_data & 0xFF);
    *command = (uint8_t)((raw_data >> 16) & 0xFF);
}

void IR_decode_lg_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)(raw_data & 0xFF);
    *command = (uint8_t)((raw_data >> 8) & 0xFF);
}

void IR_decode_panasonic_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)((raw_data >> 8) & 0xFF);
    *command = (uint8_t)((raw_data >> 16) & 0xFF);
}

void IR_decode_jvc_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)(raw_data & 0xFF);
    *command = (uint8_t)((raw_data >> 8) & 0xFF);
}

void IR_decode_rc6_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *command = (uint8_t)(raw_data & 0xFF);
    *address = (uint8_t)((raw_data >> 8) & 0xFF);
}

void IR_decode_denon_data(uint32_t raw_data, uint8_t* address, uint8_t* command)
{
    *address = (uint8_t)(raw_data & 0x1F);
    *command = (uint8_t)((raw_data >> 5) & 0x3FF);
}

// Utility functions
uint8_t IR_validate_protocol_data(IR_Protocol_t protocol, uint32_t raw_data)
{
    switch (protocol) {
        case IR_PROTOCOL_NEC:
            // Check if address and command inverse are correct
            return ((raw_data & 0xFF) == (~((raw_data >> 8) & 0xFF) & 0xFF)) &&
                   (((raw_data >> 16) & 0xFF) == (~((raw_data >> 24) & 0xFF) & 0xFF));
        
        case IR_PROTOCOL_SAMSUNG:
            // Check if command inverse is correct
            return (((raw_data >> 16) & 0xFF) == (~((raw_data >> 24) & 0xFF) & 0xFF));
        
        default:
            return 1; // Assume valid for other protocols
    }
}

uint16_t IR_calculate_checksum(uint32_t data)
{
    uint16_t checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += (data >> (i * 8)) & 0xFF;
    }
    return checksum & 0xFF;
}
