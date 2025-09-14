/**
 * stm32f401_ir_demo.c - STM32F401 IR Decoder Demo
 * 
 * Example showing how to use IR decoder library with STM32F401
 * Created: New example for STM32F401
 * Author: Nghia Taarabt
 */

#include "stm32f4xx.h"
#include "ir_decoder.h"
#include "stm32f401_hal.h"
#include <stdio.h>

// Global IR decoder instance
IR_Decoder_t ir_decoder;
IR_HAL_t ir_hal;

// UART for debug output (optional)
void uart_init(void);
void uart_send_string(const char* str);
void uart_send_hex(uint32_t value);

int main(void) {
    // Initialize system clock (optional - use default for now)
    SystemCoreClockUpdate();
    
    // Initialize UART for debug output
    uart_init();
    uart_send_string("STM32F401 IR Decoder Demo\r\n");
    
    // Initialize STM32F401 hardware
    stm32f401_hardware_init();
    
    // Initialize HAL
    stm32f401_hal_init(&ir_hal);
    
    // Initialize IR decoder with NEC protocol
    IR_decoder_init(&ir_decoder, IR_PROTOCOL_NEC, &ir_hal);
    
    uart_send_string("IR Decoder initialized. Waiting for IR signals...\r\n");
    
    // Main loop
    while (1) {
        IR_Data_t ir_data;
        
        // Check if IR data is available
        if (IR_decoder_get_data(&ir_decoder, &ir_data) == 0) {
            uart_send_string("IR Data Received:\r\n");
            uart_send_string("  Protocol: ");
            
            switch (ir_data.protocol) {
                case IR_PROTOCOL_NEC:
                    uart_send_string("NEC\r\n");
                    break;
                case IR_PROTOCOL_RC5:
                    uart_send_string("RC5\r\n");
                    break;
                case IR_PROTOCOL_SONY:
                    uart_send_string("SONY\r\n");
                    break;
                default:
                    uart_send_string("UNKNOWN\r\n");
                    break;
            }
            
            uart_send_string("  Address: 0x");
            uart_send_hex(ir_data.address);
            uart_send_string("\r\n  Command: 0x");
            uart_send_hex(ir_data.command);
            uart_send_string("\r\n  Repeat: ");
            uart_send_string(ir_data.repeat ? "YES" : "NO");
            uart_send_string("\r\n\r\n");
        }
        
        // Handle timeout
        if (stm32f401_ir_timeout > IR_TIMEOUT_VALUE) {
            IR_decoder_timeout_handler(&ir_decoder);
            stm32f401_ir_timeout = 0;
        }
        
        // Small delay
        for (volatile int i = 0; i < 1000; i++);
    }
}

/**
 * EXTI0 Interrupt Handler - IR Pin Change
 */
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR |= EXTI_PR_PR0;                    // Clear interrupt flag
        
        // Call HAL interrupt handler
        stm32f401_ir_pin_interrupt();
        
        // Process IR signal
        IR_decoder_process(&ir_decoder, stm32f401_ir_pin_state);
    }
}

/**
 * TIM2 Interrupt Handler - Timer Overflow
 */
void TIM2_IRQHandler(void) {
    stm32f401_timer_interrupt();
}

/**
 * Simple UART initialization for debug output
 */
void uart_init(void) {
    // Enable GPIOA and USART2 clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    
    // Configure PA2 (TX) and PA3 (RX) for USART2
    GPIOA->MODER |= (2U << (2 * 2)) | (2U << (3 * 2));     // Alternate function
    GPIOA->AFR[0] |= (7U << (2 * 4)) | (7U << (3 * 4));    // AF7 for USART2
    
    // Configure USART2: 115200 baud, 8N1
    USART2->BRR = 84000000 / 115200;                        // Baud rate
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

/**
 * Send string via UART
 */
void uart_send_string(const char* str) {
    while (*str) {
        while (!(USART2->SR & USART_SR_TXE));
        USART2->DR = *str++;
    }
}

/**
 * Send hex value via UART
 */
void uart_send_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    uart_send_string(buffer);
}
