#ifndef INIT_H
#define INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "../../CMSIS/Devices/STM32F4xx/Inc/STM32F429ZI/stm32f429xx.h"
#include "stm32f4xx.h"

// LB2

void RCC_INIT(void);                  // Для HSE 180 MHz
void ITR_Init(void);                  // Для прерывания PC13 (for test)
void Clock_Init_HSI_PLL_168MHz(void); // Тактирование на 168Mhz от HSI. См. предделители
void SysTick_Init_1ms(void);          // Настройка SysTick на 1мс

void Buttons_GPIO_Init(void); //
void Buttons_EXTI_Init(void);
void LEDs_GPIO_Init(void);

#endif

/* Задание. 1 Вариант

Кнопка 1 (PA0). Каждое нажатие включает следующий светодиод в
заданном режиме работы, предыдущий выключается. Процесс
цикличен.

Кнопка 2 (PA5).

Функция 1: Каждое кратковременное нажатие устанавливает частоту
мерцания для всех светодиодов в диапазоне 0.2 Гц, 0.8 Гц, 1.3 Гц.

Функция 2: Каждое удержание в течении 2-х секунд меняет режим
работы светодиодов с мерцания на простое свечение и обратно, после
повторного удержания.

*/