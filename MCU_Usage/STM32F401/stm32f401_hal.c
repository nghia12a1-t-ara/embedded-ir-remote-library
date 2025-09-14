/**
 * stm32f401_hal.c - Hardware Abstraction Layer Implementation for STM32F401
 * 
 * STM32F401-specific implementation using CMSIS
 * Created: New file for STM32F401 hardware abstraction
 * Author: Nghia Taarabt
 */

#include "stm32f401_hal.h"

// Global variables
volatile uint16_t stm32f401_ir_counter = 0;
volatile uint16_t stm32f401_ir_timeout = 0;
volatile uint8_t stm32f401_ir_pin_state = 0;

/**
 * Initialize STM32F401 hardware for IR decoding
 */
void stm32f401_hardware_init(void) {
    // Enable GPIO clock
    RCC->AHB1ENR |= IR_IN_GPIO_CLK;
    
    // Configure PA0 as input with pull-up
    IR_IN_GPIO_PORT->MODER &= ~(3U << (0 * 2));    // Input mode
    IR_IN_GPIO_PORT->PUPDR |= (1U << (0 * 2));     // Pull-up
    IR_IN_GPIO_PORT->OSPEEDR |= (3U << (0 * 2));   // High speed
    
    // Configure PA0 for EXTI0 interrupt
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;           // Enable SYSCFG clock
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;    // PA0 for EXTI0
    EXTI->IMR |= EXTI_IMR_MR0;                      // Enable EXTI0 interrupt
    EXTI->FTSR |= EXTI_FTSR_TR0;                    // Falling edge trigger
    EXTI->RTSR |= EXTI_RTSR_TR0;                    // Rising edge trigger
    
    // Enable EXTI0 interrupt in NVIC
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_SetPriority(EXTI0_IRQn, 1);
    
    // Enable Timer clock
    RCC->APB1ENR |= IR_TIMER_CLK;
    
    // Configure Timer2 for microsecond counting
    IR_TIMER->PSC = IR_TIMER_PRESCALER;             // 1MHz clock (1us resolution)
    IR_TIMER->ARR = 0xFFFF;                         // Maximum count
    IR_TIMER->DIER |= TIM_DIER_UIE;                 // Enable update interrupt
    
    // Enable Timer interrupt in NVIC
    NVIC_EnableIRQ(IR_TIMER_IRQ);
    NVIC_SetPriority(IR_TIMER_IRQ, 2);
}

/**
 * Start the timer for IR timing measurements
 */
void stm32f401_timer_start(void) {
    IR_TIMER->CNT = 0;                              // Reset counter
    IR_TIMER->CR1 |= TIM_CR1_CEN;                   // Enable timer
}

/**
 * Stop the timer
 */
void stm32f401_timer_stop(void) {
    IR_TIMER->CR1 &= ~TIM_CR1_CEN;                  // Disable timer
}

/**
 * Get current timer count in microseconds
 */
uint16_t stm32f401_timer_get_count(void) {
    return (uint16_t)IR_TIMER->CNT;
}

/**
 * Reset timer count to zero
 */
void stm32f401_timer_reset_count(void) {
    IR_TIMER->CNT = 0;
}

/**
 * Read IR input pin state
 */
uint8_t stm32f401_pin_read(void) {
    return (IR_IN_GPIO_PORT->IDR & (1U << 0)) ? 1 : 0;
}

/**
 * Initialize HAL structure with STM32F401 functions
 */
void stm32f401_hal_init(IR_HAL_t* hal) {
    hal->timer_start = stm32f401_timer_start;
    hal->timer_stop = stm32f401_timer_stop;
    hal->timer_get_count = stm32f401_timer_get_count;
    hal->timer_reset_count = stm32f401_timer_reset_count;
    hal->pin_read = stm32f401_pin_read;
}

/**
 * IR pin interrupt handler (call from EXTI0_IRQHandler)
 */
void stm32f401_ir_pin_interrupt(void) {
    stm32f401_ir_pin_state = stm32f401_pin_read();
    stm32f401_ir_counter = stm32f401_timer_get_count();
    stm32f401_timer_reset_count();
}

/**
 * Timer interrupt handler (call from TIM2_IRQHandler)
 */
void stm32f401_timer_interrupt(void) {
    if (IR_TIMER->SR & TIM_SR_UIF) {
        IR_TIMER->SR &= ~TIM_SR_UIF;                // Clear interrupt flag
        stm32f401_ir_timeout++;
    }
}
