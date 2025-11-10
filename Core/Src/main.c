#include <stdint.h>
#include "init.h"
#include "Interrupt.h"
// Посылаю всем привет из второй лабораторной!
extern uint32_t SystemCoreClock;
volatile uint32_t g_msTicks = 0U; // глобальный счётчик миллисекунд

uint32_t millis(void)
{
    return g_msTicks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms)
    {
        /* просто ждём */
    }
}

int main(void)
{
    Clock_Init_HSE_PLL_168MHz(); // настраиваем PLL от HSE → 168 МГц
    SysTick_Init_1ms();          // заводим системный таймер на 1 мс
    LEDs_GPIO_Init();            // твоя инициализация PD1,2,3,4,6,7
    Buttons_GPIO_Init();         // PA0, PA5 как входы с pull-up
    Buttons_EXTI_Init();         // EXTI0 и EXTI9_5
    Interrupts_InitState();      // начальное состояние логики
    //__enable_irq(); // глобально включим прерывания

    /*Настройка тактирования, светодиода */
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN);

    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
    SET_BIT(GPIOB->MODER, GPIO_MODER_MODE7_0); /* 01 */

    /*-----------MCO2------------------------*/

    // Необхоидимо настроить пины на выход

    // Для PC9
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);
    /* MODER: сначала очищаем, потом ставим 10b (AF) */
    CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODE9_Msk);
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODE9_1); // бит MODE9_1 = 1, MODE9_0 = 0 → 10b

    CLEAR_BIT(GPIOC->OTYPER, GPIO_OTYPER_OT9_Msk);
    /* OSPEEDR: 11b = very high speed */
    CLEAR_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9_Msk);
    SET_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9_Msk);
    CLEAR_BIT(GPIOC->PUPDR, GPIO_PUPDR_PUPD9_Msk);
    CLEAR_BIT(GPIOC->AFR[1], 0xFU << ((9U - 8U) * 4U)); // (9-8)*4 = 4, поле для PC9
    /* Сначала очищаем поля MCO2 и MCO2PRE */

    CLEAR_BIT(RCC->CFGR, RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE);
    /* Источник MCO2 = PLLCLK (MCO2[1:0] = 11b) */
    SET_BIT(RCC->CFGR, RCC_CFGR_MCO2_0 | RCC_CFGR_MCO2_1);
    /* Предделитель MCO2PRE = /5 → 111b: ставим все три бита */
    SET_BIT(RCC->CFGR, RCC_CFGR_MCO2PRE_0 |
                           RCC_CFGR_MCO2PRE_1 |
                           RCC_CFGR_MCO2PRE_2);

    while (1)
    {
        __WFI(); // просто ждём прерываний
    }
}
