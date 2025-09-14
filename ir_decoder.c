/**
 * ir_decoder.c - Generic IR Protocol Decoder Library Implementation
 * 
 * Hardware-independent IR decoder implementation
 * Created: Refactored from ir_nec.c for better portability
 * Author: Nghia Taarabt
 */

#include "ir_decoder.h"

static int8_t IR_process_protocol_data(IR_Decoder_t* decoder, uint16_t counter, uint8_t value);

// Protocol Configuration Functions
void IR_get_nec_config(IR_Protocol_Config_t* config)
{
    // NEC Protocol timing constants (for 38.222kHz carrier)
    config->start_burst_min = 655U;
    config->start_burst_max = 815U;
    config->start_space_min = 330U;
    config->start_space_max = 360U;
    config->repeat_space_min = 155U;
    config->repeat_space_max = 185U;
    config->bit_count = 32U;
    config->timeout = 7400U;
    config->bit_threshold = 90U;  // Threshold for distinguishing 0 and 1
}

void IR_get_rc5_config(IR_Protocol_Config_t* config)
{
    // RC5 Protocol timing constants (placeholder - implement as needed)
    config->start_burst_min = 400U;
    config->start_burst_max = 600U;
    config->start_space_min = 400U;
    config->start_space_max = 600U;
    config->repeat_space_min = 0U;
    config->repeat_space_max = 0U;
    config->bit_count = 14U;
    config->timeout = 5000U;
    config->bit_threshold = 50U;
}

void IR_get_sony_config(IR_Protocol_Config_t* config)
{
    // Sony SIRC Protocol timing constants (placeholder - implement as needed)
    config->start_burst_min = 500U;
    config->start_burst_max = 700U;
    config->start_space_min = 200U;
    config->start_space_max = 400U;
    config->repeat_space_min = 0U;
    config->repeat_space_max = 0U;
    config->bit_count = 12U;
    config->timeout = 6000U;
    config->bit_threshold = 60U;
}

void IR_get_rc6_config(IR_Protocol_Config_t* config)
{
    // RC6 Protocol timing constants (Philips RC6)
    // Leader: 2.666ms pulse + 0.889ms space
    config->start_burst_min = 200U;   // ~2.666ms pulse
    config->start_burst_max = 220U;
    config->start_space_min = 65U;    // ~0.889ms space
    config->start_space_max = 75U;
    config->repeat_space_min = 0U;    // No repeat code
    config->repeat_space_max = 0U;
    config->bit_count = 21U;          // Mode(3) + Toggle(1) + Address(8) + Command(8) + Trailer(1)
    config->timeout = 8000U;
    config->bit_threshold = 50U;      // Manchester encoding threshold
}

void IR_get_samsung_config(IR_Protocol_Config_t* config)
{
    // Samsung Protocol timing constants
    // Similar to NEC but with different timing
    config->start_burst_min = 340U;   // ~4.5ms pulse
    config->start_burst_max = 380U;
    config->start_space_min = 340U;   // ~4.5ms space
    config->start_space_max = 380U;
    config->repeat_space_min = 170U;  // ~2.25ms repeat space
    config->repeat_space_max = 190U;
    config->bit_count = 32U;          // Address(16) + Command(16)
    config->timeout = 7500U;
    config->bit_threshold = 90U;      // Similar to NEC
}

void IR_get_lg_config(IR_Protocol_Config_t* config)
{
    // LG Protocol timing constants
    // 9ms pulse + 4.5ms space for start
    config->start_burst_min = 680U;   // ~9ms pulse
    config->start_burst_max = 720U;
    config->start_space_min = 340U;   // ~4.5ms space
    config->start_space_max = 360U;
    config->repeat_space_min = 170U;  // ~2.25ms repeat
    config->repeat_space_max = 190U;
    config->bit_count = 28U;          // Address(8) + Command(16) + Checksum(4)
    config->timeout = 7000U;
    config->bit_threshold = 85U;
}

void IR_get_panasonic_config(IR_Protocol_Config_t* config)
{
    // Panasonic Protocol timing constants
    // 3.5ms pulse + 1.75ms space for start
    config->start_burst_min = 265U;   // ~3.5ms pulse
    config->start_burst_max = 285U;
    config->start_space_min = 130U;   // ~1.75ms space
    config->start_space_max = 150U;
    config->repeat_space_min = 0U;    // No standard repeat
    config->repeat_space_max = 0U;
    config->bit_count = 48U;          // Address(16) + Command(32)
    config->timeout = 9000U;
    config->bit_threshold = 70U;
}

void IR_get_jvc_config(IR_Protocol_Config_t* config)
{
    // JVC Protocol timing constants
    // 8.4ms pulse + 4.2ms space for start
    config->start_burst_min = 635U;   // ~8.4ms pulse
    config->start_burst_max = 665U;
    config->start_space_min = 315U;   // ~4.2ms space
    config->start_space_max = 335U;
    config->repeat_space_min = 0U;    // No repeat code in first transmission
    config->repeat_space_max = 0U;
    config->bit_count = 16U;          // Address(8) + Command(8)
    config->timeout = 6000U;
    config->bit_threshold = 80U;
}

void IR_get_denon_config(IR_Protocol_Config_t* config)
{
    // Denon Protocol timing constants (Sharp variant)
    // 3.2ms pulse + 1.6ms space for start
    config->start_burst_min = 240U;   // ~3.2ms pulse
    config->start_burst_max = 260U;
    config->start_space_min = 120U;   // ~1.6ms space
    config->start_space_max = 140U;
    config->repeat_space_min = 0U;    // No repeat code
    config->repeat_space_max = 0U;
    config->bit_count = 15U;          // Address(5) + Command(8) + Expansion(2)
    config->timeout = 5500U;
    config->bit_threshold = 60U;
}

void IR_decoder_init(IR_Decoder_t* decoder, IR_Protocol_t protocol, IR_HAL_t* hal)
{
    // Initialize decoder state
    decoder->state = IR_STATE_IDLE;
    decoder->event = IR_EVENT_INIT;
    decoder->bit_index = 0;
    decoder->data_buffer = 0;
    decoder->timeout_counter = 0;
    decoder->protocol_type = protocol;
    
    // Copy HAL function pointers
    decoder->hal = *hal;
    
    // Configure protocol-specific parameters
    switch(protocol)
    {
        case IR_PROTOCOL_NEC:
            IR_get_nec_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_RC5:
            IR_get_rc5_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_SONY:
            IR_get_sony_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_RC6:
            IR_get_rc6_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_SAMSUNG:
            IR_get_samsung_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_LG:
            IR_get_lg_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_PANASONIC:
            IR_get_panasonic_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_JVC:
            IR_get_jvc_config(&decoder->protocol_config);
            break;
        case IR_PROTOCOL_DENON:
            IR_get_denon_config(&decoder->protocol_config);
            break;
        default:
            IR_get_nec_config(&decoder->protocol_config);  // Default to NEC
            break;
    }
    
    // Initialize decoded data
    decoder->decoded_data.raw_data = 0;
    decoder->decoded_data.address = 0;
    decoder->decoded_data.command = 0;
    decoder->decoded_data.protocol = protocol;
    decoder->decoded_data.valid = 0;
    
    // Start hardware timer through HAL
    if(decoder->hal.timer_start)
        decoder->hal.timer_start();
}

static int8_t IR_process_protocol_data(IR_Decoder_t* decoder, uint16_t counter, uint8_t value)
{
    int8_t retval = IR_ERROR;
    IR_Protocol_Config_t* config = &decoder->protocol_config;

    switch(decoder->event)
    {
        case IR_EVENT_INIT:
            decoder->data_buffer = decoder->bit_index = 0U;
            retval = IR_SUCCESS;
            break;
            
        case IR_EVENT_DATA:
            if(decoder->bit_index < config->bit_count)
            {
                if(value == IR_HIGH)
                {
                    // Use protocol-specific bit threshold
                    uint8_t bit_value = (counter < config->bit_threshold) ? 0U : 1U;
                    decoder->data_buffer |= ((uint32_t)bit_value << decoder->bit_index++);
                    
                    if(decoder->bit_index == config->bit_count)
                    {
                        decoder->event = IR_EVENT_HOOK;
                    }
                }
                retval = IR_SUCCESS;
            }
            break;
            
        case IR_EVENT_HOOK:
            if(value == IR_LOW)
            {
                decoder->event = IR_EVENT_FINISH;
                retval = IR_SUCCESS;
            }
            break;
            
        case IR_EVENT_FINISH:
            // Store decoded data
            decoder->decoded_data.raw_data = decoder->data_buffer;
            decoder->decoded_data.address = decoder->data_buffer & 0xFF;
            decoder->decoded_data.command = (decoder->data_buffer >> 16) & 0xFF;
            decoder->decoded_data.valid = 1;
            break;
            
        default:
            break;
    }

    return retval;
}

void IR_decoder_process(IR_Decoder_t* decoder, uint8_t pin_value)
{
    // Get counter value through HAL and reset it
    uint16_t counter = 0;
    if(decoder->hal.timer_get_count)
        counter = decoder->hal.timer_get_count();
    if(decoder->hal.timer_reset_count)
        decoder->hal.timer_reset_count();

    IR_Protocol_Config_t* config = &decoder->protocol_config;

    switch(decoder->state)
    {
        case IR_STATE_IDLE:
            if(pin_value == IR_HIGH)
            {
                decoder->state = IR_STATE_INIT;
            }
            break;
            
        case IR_STATE_INIT:
            if(pin_value == IR_LOW)
            {
                if(!(counter > config->start_burst_min && counter < config->start_burst_max))
                {
                    decoder->state = IR_STATE_FINISH;
                }
                decoder->timeout_counter = config->timeout;
            }
            else  // pin_value == IR_HIGH
            {
                if(counter > config->start_space_min && counter < config->start_space_max)
                {
                    decoder->state = IR_STATE_PROCESS;
                    decoder->event = IR_EVENT_INIT;
                }
                else if(counter > config->repeat_space_min && counter < config->repeat_space_max)
                {
                    decoder->event = IR_EVENT_FINISH;
                }
                else
                {
                    decoder->state = IR_STATE_FINISH;
                }
            }
            break;
            
        case IR_STATE_PROCESS:
            if(IR_SUCCESS != IR_process_protocol_data(decoder, counter, pin_value))
            {
                decoder->state = IR_STATE_FINISH;
            }
            break;
            
        case IR_STATE_FINISH:
            decoder->state = IR_STATE_IDLE;
            decoder->timeout_counter = 0U;
            break;
            
        default:
            break;
    }
}

int8_t IR_decoder_get_data(IR_Decoder_t* decoder, IR_Data_t* data)
{
    if(!decoder->decoded_data.valid)
        return IR_ERROR;

    *data = decoder->decoded_data;
    decoder->decoded_data.valid = 0;  // Clear valid flag after reading

    return IR_SUCCESS;
}

void IR_decoder_timeout_handler(IR_Decoder_t* decoder)
{
    // Reset counter through HAL
    if(decoder->hal.timer_get_count && decoder->hal.timer_get_count() > 10000)
        decoder->state = IR_STATE_IDLE;
        
    if(decoder->timeout_counter && --decoder->timeout_counter == 0)
        decoder->state = IR_STATE_IDLE;
}

void IR_decoder_reset(IR_Decoder_t* decoder)
{
    decoder->state = IR_STATE_IDLE;
    decoder->event = IR_EVENT_INIT;
    decoder->bit_index = 0;
    decoder->data_buffer = 0;
    decoder->timeout_counter = 0;
    decoder->decoded_data.valid = 0;
}
