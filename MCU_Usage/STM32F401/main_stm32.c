/**
 * main_stm32.c - STM32F401 IR Decoder Example
 * 
 * Complete example demonstrating IR remote control decoding using STM32F401
 * Supports multiple IR protocols: NEC, RC5, RC6, Sony SIRC, Samsung, LG
 * 
 * Hardware Setup:
 * - PA0: IR receiver data pin (with pull-up)
 * - PA2: UART2 TX for debug output (115200 baud)
 * - PA3: UART2 RX (optional)
 * - Connect IR receiver VCC to 3.3V, GND to GND
 * 
 * Author: Nghia Taarabt
 */

#include "stm32f4xx.h"
#include "ir_decoder.h"
#include "stm32f401_hal.h"
#include <stdio.h>
#include <string.h>

// Global variables
IR_Decoder_t ir_decoder;
IR_HAL_t ir_hal;
volatile uint8_t ir_data_ready = 0;
IR_Data_t received_data;

// UART configuration for debug output
#define UART_BAUDRATE   115200
#define UART_TX_PIN     GPIO_PIN_2  // PA2
#define UART_RX_PIN     GPIO_PIN_3  // PA3

// Function prototypes
void SystemClock_Config(void);
void UART2_Init(void);
void UART2_SendString(const char* str);
void UART2_SendChar(char ch);
void GPIO_Init(void);
void NVIC_Init(void);
const char* get_protocol_name(IR_Protocol_t protocol);
void print_ir_data(IR_Data_t* data);

int main(void)
{
    // System initialization
    SystemClock_Config();
    GPIO_Init();
    UART2_Init();
    NVIC_Init();
    
    // Initialize STM32F401 HAL for IR decoder
    stm32f401_hal_init(&ir_hal);
    stm32f401_hardware_init();
    
    // Initialize IR decoder with NEC protocol (can be changed)
    IR_decoder_init(&ir_decoder, IR_PROTOCOL_NEC, &ir_hal);
    
    UART2_SendString("\r\n=== STM32F401 IR Decoder Demo ===\r\n");
    UART2_SendString("Waiting for IR signals...\r\n");
    UART2_SendString("Supported protocols: NEC, RC5, RC6, Sony SIRC, Samsung, LG\r\n\r\n");
    
    while (1)
    {
        // Check if IR data is ready
        if (ir_data_ready)
        {
            ir_data_ready = 0;
            
            // Get decoded data
            if (IR_decoder_get_data(&ir_decoder, &received_data) == 0)
            {
                print_ir_data(&received_data);
            }
            
            // Reset decoder for next signal
            IR_decoder_reset(&ir_decoder);
        }
        
        // Add small delay to prevent excessive polling
        for (volatile uint32_t i = 0; i < 10000; i++);
    }
}

/**
 * System Clock Configuration
 * Configure system clock to 84MHz using HSE (if available) or HSI
 */
void SystemClock_Config(void)
{
    // Enable HSI oscillator
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    
    // Configure PLL: HSI (16MHz) / 16 * 336 / 4 = 84MHz
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSI |
                   (16 << RCC_PLLCFGR_PLLM_Pos) |
                   (336 << RCC_PLLCFGR_PLLN_Pos) |
                   (0 << RCC_PLLCFGR_PLLP_Pos);  // PLLP = 2
    
    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    
    // Configure Flash latency for 84MHz
    FLASH->ACR = FLASH_ACR_LATENCY_2WS | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    
    // Select PLL as system clock
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    // Update SystemCoreClock variable
    SystemCoreClock = 84000000;
}

/**
 * GPIO Initialization
 */
void GPIO_Init(void)
{
    // Enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    // Configure PA0 as input with pull-up (IR receiver)
    GPIOA->MODER &= ~(3 << (0 * 2));        // Input mode
    GPIOA->PUPDR |= (1 << (0 * 2));         // Pull-up
    
    // Configure PA2 as alternate function (UART2 TX)
    GPIOA->MODER |= (2 << (2 * 2));         // Alternate function
    GPIOA->AFR[0] |= (7 << (2 * 4));        // AF7 (USART2)
    
    // Configure PA3 as alternate function (UART2 RX)
    GPIOA->MODER |= (2 << (3 * 2));         // Alternate function
    GPIOA->AFR[0] |= (7 << (3 * 4));        // AF7 (USART2)
}

/**
 * UART2 Initialization for debug output
 */
void UART2_Init(void)
{
    // Enable USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    
    // Configure UART: 115200 baud, 8N1
    // Baud rate = fCK / (16 * USARTDIV)
    // USARTDIV = 84000000 / (16 * 115200) = 45.57 ≈ 45.625
    // Mantissa = 45, Fraction = 0.625 * 16 = 10
    USART2->BRR = (45 << 4) | 10;
    
    // Enable UART, transmitter and receiver
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/**
 * NVIC Initialization for interrupts
 */
void NVIC_Init(void)
{
    // Enable EXTI0 interrupt (PA0 - IR input)
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_SetPriority(EXTI0_IRQn, 1);
    
    // Enable TIM2 interrupt (IR timing)
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 2);
    
    // Configure EXTI0 for PA0
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;  // PA0
    EXTI->IMR |= EXTI_IMR_MR0;                    // Enable interrupt
    EXTI->RTSR |= EXTI_RTSR_TR0;                  // Rising edge
    EXTI->FTSR |= EXTI_FTSR_TR0;                  // Falling edge
}

/**
 * Send string via UART2
 */
void UART2_SendString(const char* str)
{
    while (*str)
    {
        UART2_SendChar(*str++);
    }
}

/**
 * Send character via UART2
 */
void UART2_SendChar(char ch)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = ch;
}

/**
 * Get protocol name string
 */
const char* get_protocol_name(IR_Protocol_t protocol)
{
    switch (protocol)
    {
        case IR_PROTOCOL_NEC:       return "NEC";
        case IR_PROTOCOL_RC5:       return "RC5";
        case IR_PROTOCOL_RC6:       return "RC6";
        case IR_PROTOCOL_SONY_SIRC: return "Sony SIRC";
        case IR_PROTOCOL_SAMSUNG:   return "Samsung";
        case IR_PROTOCOL_LG:        return "LG";
        default:                    return "Unknown";
    }
}

/**
 * Print decoded IR data
 */
void print_ir_data(IR_Data_t* data)
{
    char buffer[100];
    
    sprintf(buffer, "Protocol: %s\r\n", get_protocol_name(data->protocol));
    UART2_SendString(buffer);
    
    sprintf(buffer, "Address: 0x%04X\r\n", data->address);
    UART2_SendString(buffer);
    
    sprintf(buffer, "Command: 0x%02X\r\n", data->command);
    UART2_SendString(buffer);
    
    sprintf(buffer, "Raw Data: 0x%08lX\r\n", data->raw_data);
    UART2_SendString(buffer);
    
    if (data->repeat)
    {
        UART2_SendString("Type: REPEAT\r\n");
    }
    else
    {
        UART2_SendString("Type: NEW\r\n");
    }
    
    UART2_SendString("------------------------\r\n");
}

/**
 * EXTI0 Interrupt Handler - IR Pin Change
 */
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0)
    {
        EXTI->PR |= EXTI_PR_PR0;  // Clear interrupt flag
        
        // Call STM32 HAL interrupt handler
        stm32f401_ir_pin_interrupt();
        
        // Process IR signal
        uint8_t pin_state = stm32f401_pin_read();
        IR_decoder_process(&ir_decoder, pin_state);
        
        // Check if decoding is complete
        if (ir_decoder.state == IR_STATE_FINISH)
        {
            ir_data_ready = 1;
        }
    }
}

/**
 * TIM2 Interrupt Handler - IR Timeout
 */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;  // Clear interrupt flag
        
        // Call STM32 HAL interrupt handler
        stm32f401_timer_interrupt();
        
        // Handle timeout
        IR_decoder_timeout_handler(&ir_decoder);
        
        // Reset decoder on timeout
        IR_decoder_reset(&ir_decoder);
    }
}

/**
 * Hard Fault Handler for debugging
 */
void HardFault_Handler(void)
{
    UART2_SendString("Hard Fault occurred!\r\n");
    while (1);
}
