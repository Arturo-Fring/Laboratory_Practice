#include "init.h"

/* Частоты для таймера (ARR): 1, 5, 10, 20 Гц */
static const uint16_t g_arrValues[4] = {
    9999, // 1 Гц
    1999, // 5 Гц
    999,  // 10 Гц
    499   // 20 Гц
};

/* ===== Светодиоды ===== */

void LEDs_AllOff(void)
{
    GPIOF->BSRR = (1U << (LED1_PIN + 16)) |
                  (1U << (LED2_PIN + 16)) |
                  (1U << (LED3_PIN + 16));
    GPIOG->BSRR = (1U << (LED4_PIN + 16));
}

void LED_OnIndex(uint8_t index)
{
    switch (index)
    {
    case 0:
        LED1_PORT->BSRR = (1U << LED1_PIN);
        break;
    case 1:
        LED2_PORT->BSRR = (1U << LED2_PIN);
        break;
    case 2:
        LED3_PORT->BSRR = (1U << LED3_PIN);
        break;
    case 3:
        LED4_PORT->BSRR = (1U << LED4_PIN);
        break;
    }
}

/* ===== GPIO + EXTI ====== */

void GPIO_EXTI_Init(void)
{
    /* Включаем тактирование GPIO и SYSCFG */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOFEN |
                    RCC_AHB1ENR_GPIOGEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* ==== LED PF7, PF8, PF9 ==== */
    GPIOF->MODER &= ~((3U << (LED1_PIN * 2)) |
                      (3U << (LED2_PIN * 2)) |
                      (3U << (LED3_PIN * 2)));
    GPIOF->MODER |= ((1U << (LED1_PIN * 2)) |
                     (1U << (LED2_PIN * 2)) |
                     (1U << (LED3_PIN * 2)));

    GPIOF->OTYPER &= ~((1U << LED1_PIN) | (1U << LED2_PIN) | (1U << LED3_PIN));
    GPIOF->PUPDR &= ~((3U << (LED1_PIN * 2)) |
                      (3U << (LED2_PIN * 2)) |
                      (3U << (LED3_PIN * 2)));

    /* ==== LED PG1 ==== */
    GPIOG->MODER &= ~(3U << (LED4_PIN * 2));
    GPIOG->MODER |= (1U << (LED4_PIN * 2));
    GPIOG->OTYPER &= ~(1U << LED4_PIN);
    GPIOG->PUPDR &= ~(3U << (LED4_PIN * 2));

    /* ==== Кнопка PA0 input + pull-up ==== */
    GPIOA->MODER &= ~(3U << (BUTTON_PIN * 2));
    GPIOA->PUPDR &= ~(3U << (BUTTON_PIN * 2));
    GPIOA->PUPDR |= (1U << (BUTTON_PIN * 2)); // Pull-up

    /* ==== EXTI0 ==== */
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0; // PA0

    EXTI->IMR |= EXTI_IMR_MR0;   // разрешаем линию 0
    EXTI->FTSR |= EXTI_FTSR_TR0; // по спаду (кнопка нажата)
    EXTI->RTSR &= ~EXTI_RTSR_TR0;

    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

/* ===== TIM2 ===== */

void TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 = 0;
    TIM2->CNT = 0;

    /* PCLK1=42 MHz -> TIMCLK=84 MHz
       PSC=8399 -> 10 kHz */
    TIM2->PSC = 8399;
    TIM2->ARR = g_arrValues[0]; // начальная частота 1 Гц

    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM2_IRQn, 2);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN; // старт
}

/* Смена частоты */
void TIM2_UpdateFrequency(uint8_t index)
{
    TIM2->ARR = g_arrValues[index];
    TIM2->EGR = TIM_EGR_UG; // обновить
}

/* ================= Clock Init ================== */

void Clock_Init_HSE_PLL_168MHz(void)
{
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    SET_BIT(PWR->CR, PWR_CR_VOS);

    SET_BIT(RCC->CR, RCC_CR_HSEON);
    while (!(RCC->CR & RCC_CR_HSERDY))
        ;

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_5WS);

    MODIFY_REG(RCC->CFGR,
               RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2,
               RCC_CFGR_HPRE_DIV1 |
                   RCC_CFGR_PPRE1_DIV4 |
                   RCC_CFGR_PPRE2_DIV2);

    WRITE_REG(RCC->PLLCFGR,
              (8U << RCC_PLLCFGR_PLLM_Pos) |
                  (336U << RCC_PLLCFGR_PLLN_Pos) |
                  (0U << RCC_PLLCFGR_PLLP_Pos) |
                  RCC_PLLCFGR_PLLSRC_HSE |
                  (7U << RCC_PLLCFGR_PLLQ_Pos));

    SET_BIT(RCC->CR, RCC_CR_PLLON);
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        ;

    CLEAR_BIT(RCC->CR, RCC_CR_HSION);

    SystemCoreClock = 168000000;
}
