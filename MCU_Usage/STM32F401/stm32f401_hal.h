/**
 * stm32f401_hal.h - Hardware Abstraction Layer for STM32F401
 * 
 * STM32F401-specific implementation of IR decoder HAL using CMSIS
 * Created: New file for STM32F401 hardware abstraction
 * Author: Nghia Taarabt
 */

#ifndef STM32F401_HAL_H_
#define STM32F401_HAL_H_

#include "stm32f4xx.h"
#include <stdint.h>
#include "ir_decoder.h"

// Hardware Configuration for STM32F401
#define IR_IN_GPIO_PORT     GPIOA
#define IR_IN_GPIO_PIN      GPIO_PIN_0      // PA0 - IR receiver input
#define IR_IN_GPIO_CLK      RCC_AHB1ENR_GPIOAEN

#define IR_TIMER            TIM2            // Use TIM2 for timing measurements
#define IR_TIMER_CLK        RCC_APB1ENR_TIM2EN
#define IR_TIMER_IRQ        TIM2_IRQn

// Timer configuration (84MHz / 84 = 1MHz = 1us resolution)
#define IR_TIMER_PRESCALER  83              // 84MHz / (83+1) = 1MHz
#define IR_TIMEOUT_VALUE    50000           // 50ms timeout

// Global variables for STM32F401 HAL
extern volatile uint16_t stm32f401_ir_counter;
extern volatile uint16_t stm32f401_ir_timeout;
extern volatile uint8_t stm32f401_ir_pin_state;

// HAL Function Declarations
void stm32f401_timer_start(void);
void stm32f401_timer_stop(void);
uint16_t stm32f401_timer_get_count(void);
void stm32f401_timer_reset_count(void);
uint8_t stm32f401_pin_read(void);

// HAL Initialization
void stm32f401_hal_init(IR_HAL_t* hal);
void stm32f401_hardware_init(void);

// Interrupt handlers (to be called from main application)
void stm32f401_ir_pin_interrupt(void);
void stm32f401_timer_interrupt(void);

#endif /* STM32F401_HAL_H_ */
