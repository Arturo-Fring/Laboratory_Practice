#ifndef INIT_H
#define INIT_H

#include "stm32f4xx.h"

/* Светодиоды */
#define LED1_PORT   GPIOF
#define LED1_PIN    7
#define LED2_PORT   GPIOF
#define LED2_PIN    8
#define LED3_PORT   GPIOF
#define LED3_PIN    9
#define LED4_PORT   GPIOG
#define LED4_PIN    1

/* Кнопка */
#define BUTTON_PORT GPIOA
#define BUTTON_PIN  0

/* Глобальная частота SystemCoreClock задаётся в clock init */
void Clock_Init_HSE_PLL_168MHz(void);

/* Инициализация GPIO LED + кнопка + EXTI */
void GPIO_EXTI_Init(void);

/* Инициализация TIM2 - без запуска */
void TIM2_Init(void);

/* Обновление частоты таймера (переключение ARR) */
void TIM2_UpdateFrequency(uint8_t index);

/* Светодиоды */
void LEDs_AllOff(void);
void LED_OnIndex(uint8_t index);

#endif
