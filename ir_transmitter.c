/**
 * ir_transmitter.c - Generic IR Protocol Transmitter Implementation
 * 
 * Hardware-independent IR transmitter implementation
 */

#include "ir_transmitter.h"

static void IR_transmit_frame(IR_Transmitter_t* transmitter);

// Protocol configuration functions
void IR_get_nec_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 9000;
    config->start_space_us = 4500;
    config->repeat_space_us = 2250;
    config->bit_burst_us = 562;
    config->bit_0_space_us = 562;
    config->bit_1_space_us = 1687;
    config->stop_burst_us = 562;
    config->bit_count = 32;
    config->repeat_count = 1;
    config->carrier_freq = 38000;
}

void IR_get_rc5_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 889;  // Half bit time
    config->start_space_us = 889;
    config->repeat_space_us = 0;   // No repeat for RC5
    config->bit_burst_us = 889;
    config->bit_0_space_us = 889;
    config->bit_1_space_us = 889;
    config->stop_burst_us = 0;
    config->bit_count = 14;
    config->repeat_count = 0;
    config->carrier_freq = 36000;
}

void IR_get_sony_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 2400;
    config->start_space_us = 600;
    config->repeat_space_us = 0;
    config->bit_burst_us = 600;
    config->bit_0_space_us = 600;
    config->bit_1_space_us = 1200;
    config->stop_burst_us = 0;
    config->bit_count = 12;
    config->repeat_count = 2;  // Sony sends 3 times
    config->carrier_freq = 40000;
}

void IR_get_rc6_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 2666;
    config->start_space_us = 889;
    config->repeat_space_us = 0;
    config->bit_burst_us = 444;
    config->bit_0_space_us = 444;
    config->bit_1_space_us = 444;
    config->stop_burst_us = 0;
    config->bit_count = 16;
    config->repeat_count = 0;
    config->carrier_freq = 36000;
}

void IR_get_samsung_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 4500;
    config->start_space_us = 4500;
    config->repeat_space_us = 2250;
    config->bit_burst_us = 560;
    config->bit_0_space_us = 560;
    config->bit_1_space_us = 1690;
    config->stop_burst_us = 560;
    config->bit_count = 32;
    config->repeat_count = 1;
    config->carrier_freq = 38000;
}

void IR_get_lg_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 9000;
    config->start_space_us = 4500;
    config->repeat_space_us = 2250;
    config->bit_burst_us = 560;
    config->bit_0_space_us = 560;
    config->bit_1_space_us = 1690;
    config->stop_burst_us = 560;
    config->bit_count = 28;
    config->repeat_count = 1;
    config->carrier_freq = 38000;
}

void IR_get_panasonic_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 3502;
    config->start_space_us = 1750;
    config->repeat_space_us = 0;
    config->bit_burst_us = 502;
    config->bit_0_space_us = 400;
    config->bit_1_space_us = 1244;
    config->stop_burst_us = 502;
    config->bit_count = 48;
    config->repeat_count = 0;
    config->carrier_freq = 35000;
}

void IR_get_jvc_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 8400;
    config->start_space_us = 4200;
    config->repeat_space_us = 0;  // JVC has no repeat
    config->bit_burst_us = 525;
    config->bit_0_space_us = 525;
    config->bit_1_space_us = 1575;
    config->stop_burst_us = 525;
    config->bit_count = 16;
    config->repeat_count = 0;
    config->carrier_freq = 38000;
}

void IR_get_denon_tx_config(IR_TX_Protocol_Config_t* config) {
    config->start_burst_us = 275;
    config->start_space_us = 775;
    config->repeat_space_us = 0;
    config->bit_burst_us = 275;
    config->bit_0_space_us = 775;
    config->bit_1_space_us = 1900;
    config->stop_burst_us = 275;
    config->bit_count = 15;
    config->repeat_count = 1;
    config->carrier_freq = 38000;
}

// Initialize transmitter
void IR_transmitter_init(IR_Transmitter_t* transmitter, IR_Protocol_t protocol, IR_TX_HAL_t* hal) {
    transmitter->state = IR_TX_STATE_IDLE;
    transmitter->protocol_type = protocol;
    transmitter->hal = *hal;
    transmitter->is_transmitting = 0;
    transmitter->repeat_counter = 0;
    
    // Load protocol configuration
    switch(protocol) {
        case IR_PROTOCOL_NEC:
            IR_get_nec_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_RC5:
            IR_get_rc5_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_SONY:
            IR_get_sony_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_RC6:
            IR_get_rc6_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_SAMSUNG:
            IR_get_samsung_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_LG:
            IR_get_lg_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_PANASONIC:
            IR_get_panasonic_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_JVC:
            IR_get_jvc_tx_config(&transmitter->protocol_config);
            break;
        case IR_PROTOCOL_DENON:
            IR_get_denon_tx_config(&transmitter->protocol_config);
            break;
        default:
            IR_get_nec_tx_config(&transmitter->protocol_config);
            break;
    }
}

// Send IR command
int8_t IR_transmitter_send(IR_Transmitter_t* transmitter, uint8_t address, uint8_t command) {
    if (transmitter->is_transmitting) {
        return IR_ERROR;  // Busy
    }
    
    // Encode data based on protocol
    switch(transmitter->protocol_type) {
        case IR_PROTOCOL_NEC:
            transmitter->data_to_send = IR_encode_nec_data(address, command);
            break;
        case IR_PROTOCOL_SONY:
            transmitter->data_to_send = IR_encode_sony_data(address, command);
            break;
        case IR_PROTOCOL_RC5:
            transmitter->data_to_send = IR_encode_rc5_data(address, command);
            break;
        case IR_PROTOCOL_SAMSUNG:
            transmitter->data_to_send = IR_encode_samsung_data(address, command);
            break;
        case IR_PROTOCOL_LG:
            transmitter->data_to_send = IR_encode_lg_data(address, command);
            break;
        case IR_PROTOCOL_PANASONIC:
            transmitter->data_to_send = IR_encode_panasonic_data(address, command);
            break;
        case IR_PROTOCOL_JVC:
            transmitter->data_to_send = IR_encode_jvc_data(address, command);
            break;
        case IR_PROTOCOL_RC6:
            transmitter->data_to_send = IR_encode_rc6_data(address, command);
            break;
        case IR_PROTOCOL_DENON:
            transmitter->data_to_send = IR_encode_denon_data(address, command);
            break;
        default:
            transmitter->data_to_send = IR_encode_nec_data(address, command);
            break;
    }
    
    return IR_transmitter_send_raw(transmitter, transmitter->data_to_send);
}

// Send raw IR data
int8_t IR_transmitter_send_raw(IR_Transmitter_t* transmitter, uint32_t raw_data) {
    if (transmitter->is_transmitting) {
        return IR_ERROR;
    }
    
    transmitter->data_to_send = raw_data;
    transmitter->current_bit = 0;
    transmitter->is_transmitting = 1;
    transmitter->repeat_counter = 0;
    
    // Start transmission
    IR_transmit_frame(transmitter);
    
    return IR_SUCCESS;
}

static void IR_transmit_frame(IR_Transmitter_t* transmitter) {
    IR_TX_Protocol_Config_t* config = &transmitter->protocol_config;
    
    // Send start burst
    transmitter->hal.carrier_on();
    transmitter->hal.delay_us(config->start_burst_us);
    transmitter->hal.carrier_off();
    transmitter->hal.delay_us(config->start_space_us);
    
    // Send data bits
    for (uint8_t i = 0; i < config->bit_count; i++) {
        uint8_t bit = (transmitter->data_to_send >> i) & 1;
        
        // Send bit burst
        transmitter->hal.carrier_on();
        transmitter->hal.delay_us(config->bit_burst_us);
        transmitter->hal.carrier_off();
        
        // Send bit space
        if (bit) {
            transmitter->hal.delay_us(config->bit_1_space_us);
        } else {
            transmitter->hal.delay_us(config->bit_0_space_us);
        }
    }
    
    // Send stop burst if needed
    if (config->stop_burst_us > 0) {
        transmitter->hal.carrier_on();
        transmitter->hal.delay_us(config->stop_burst_us);
        transmitter->hal.carrier_off();
    }
    
    transmitter->is_transmitting = 0;
}

// Send repeat signal
int8_t IR_transmitter_send_repeat(IR_Transmitter_t* transmitter) {
    if (transmitter->is_transmitting) {
        return IR_ERROR;
    }
    
    IR_TX_Protocol_Config_t* config = &transmitter->protocol_config;
    
    if (config->repeat_space_us == 0) {
        return IR_ERROR;  // Protocol doesn't support repeat
    }
    
    transmitter->is_transmitting = 1;
    
    // Send repeat signal
    transmitter->hal.carrier_on();
    transmitter->hal.delay_us(config->start_burst_us);
    transmitter->hal.carrier_off();
    transmitter->hal.delay_us(config->repeat_space_us);
    transmitter->hal.carrier_on();
    transmitter->hal.delay_us(config->stop_burst_us);
    transmitter->hal.carrier_off();
    
    transmitter->is_transmitting = 0;
    return IR_SUCCESS;
}

// Check if transmitter is busy
uint8_t IR_transmitter_is_busy(IR_Transmitter_t* transmitter) {
    return transmitter->is_transmitting;
}

// Stop transmission
void IR_transmitter_stop(IR_Transmitter_t* transmitter) {
    transmitter->hal.carrier_off();
    transmitter->is_transmitting = 0;
    transmitter->state = IR_TX_STATE_IDLE;
}
