#include "../Inc/init.h"
#include "init.h"

/*
HSI = 16 MHz
Хотим SYSCLCK = 168 MHz

Выбираем коэффициенты PLL:
PLLM = 16 -> VCO_in = 16/16 = 1
PLLN = 336 -> VCO_out = 1x336 = 336
PLLP = 2 -> SYSCLK = 336/2 = 168
PLLQ = 7 -> 336/7 = 48 MHz --

*/

#include "stm32f4xx.h"
void Clock_Init_HSE_PLL_168MHz(void)
{
    /* 1. Включаем тактирование блока питания PWR и ставим Scale 1
          (для частот до 168 МГц) */
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    SET_BIT(PWR->CR, PWR_CR_VOS); // VOS = 11b → Scale 1

    /* 2. Включаем HSE и ждём стабилизации */
    SET_BIT(RCC->CR, RCC_CR_HSEON);
    while (READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0U)
    {
        /* ждём, пока HSE не стабилизируется */
    }

    /* 3. Настраиваем FLASH: кеши + 5 тактов ожидания (для 168 МГц)
          (это безопасно сделать до переключения на быстрый такт) */

    /* Сбрасываем LATENCY и включаем кэши/предвыборку, если нужно */
    MODIFY_REG(FLASH->ACR,
               FLASH_ACR_LATENCY,
               FLASH_ACR_LATENCY_5WS); // 5 wait states

    /* Можно дополнительно включить prefetch / I-cache / D-cache (по желанию):
       SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN);
    */

    /* 4. Настраиваем делители шин: AHB, APB1, APB2

       - AHB (HCLK)  = SYSCLK / 1  = 168 МГц  (максимум)
       - APB1 (PCLK1)= HCLK  / 4   = 42  МГц  (максимум для APB1)
       - APB2 (PCLK2)= HCLK  / 2   = 84  МГц  (максимум для APB2)
    */

    MODIFY_REG(RCC->CFGR,
               RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2,
               RCC_CFGR_HPRE_DIV1 |      // AHB = /1
                   RCC_CFGR_PPRE1_DIV4 | // APB1 = /4
                   RCC_CFGR_PPRE2_DIV2); // APB2 = /2

    /* 5. Настраиваем PLL под 168 МГц от HSE = 8 МГц:
           PLLM = 8, PLLN = 336, PLLP = 2, PLLQ = 7 */

    WRITE_REG(RCC->PLLCFGR,
              (8U << RCC_PLLCFGR_PLLM_Pos) |       // PLLM = 8
                  (336U << RCC_PLLCFGR_PLLN_Pos) | // PLLN = 336
                  (0U << RCC_PLLCFGR_PLLP_Pos) |   // PLLP = 2 (код 00)
                  RCC_PLLCFGR_PLLSRC_HSE |         // источник PLL = HSE
                  (7U << RCC_PLLCFGR_PLLQ_Pos));   // PLLQ = 7 (~48 МГц)

    /* 6. Включаем PLL и ждём готовности */
    SET_BIT(RCC->CR, RCC_CR_PLLON);
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
    {
        /* ждём, пока PLL не поднимется */
    }

    /* 7. Переключаем системное тактирование на PLL */
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
    while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
    {
        /* ждём, пока в статусе не появится "PLL как SYSCLK" */
    }

    /* 8. (Опционально) Выключить HSI, чтобы не жрать лишний ток */
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);

    /* 9. Обновляем глобальную переменную SystemCoreClock */
    SystemCoreClock = 168000000U;
}

void Clock_Init_HSI_PLL_168MHz(void)
{
    /* 1. Включаем тактирование PWR и ставим режим питания Scale1 (нужен для 168 МГц) */
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);

    /* VOS — это поле из нескольких бит, очистим и выставим Scale1 */
    CLEAR_BIT(PWR->CR, PWR_CR_VOS);
    SET_BIT(PWR->CR, PWR_CR_VOS); // для F4 это "11", Scale1

    /* 2. Настройка FLASH: число тактов ожидания (LATENCY) под 168 МГц.
       Кеши и prefetch можно добавить чуть позже, чтобы не мешать. */
    CLEAR_BIT(FLASH->ACR, FLASH_ACR_LATENCY);   // очистить поле LATENCY
    SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_5WS); // 5 wait states для 168 МГц

    /* 3. Включаем HSI и ждём готовности (на всякий случай, после reset он и так включен) */
    SET_BIT(RCC->CR, RCC_CR_HSION);
    while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
    {
        /* ждём стабилизации HSI */
    }

    /* 4. Убеждаемся, что SYSCLK сейчас от HSI (чтобы безопасно крутить PLL) */
    CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW);   // сброс поля выбора источника SYSCLK
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_HSI); // выбрать HSI как SYSCLK
    while ((READ_BIT(RCC->CFGR, RCC_CFGR_SWS)) != RCC_CFGR_SWS_HSI)
    {
        /* ждём, пока источник SYSCLK реально не станет HSI */
    }

    /* 5. Выключаем PLL перед перенастройкой */
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
    {
        /* ждём, пока PLL полностью выключится */
    }

    /* 6. Отключаем HSE, bypass и CSS (у тебя всё равно нет HSE) */
    CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_HSEBYP | RCC_CR_CSSON);

    /* 7. Настраиваем делители шин: AHB = /1, APB1 = /4, APB2 = /2 */

    /* Сначала очищаем поля делителей */
    CLEAR_BIT(RCC->CFGR, RCC_CFGR_HPRE |
                             RCC_CFGR_PPRE1 |
                             RCC_CFGR_PPRE2);

    /* HPRE = /1 (ничего не ставим, это 0) */
    /* APB1 = /4 (PPRE1 = 101), APB2 = /2 (PPRE2 = 100) */
    SET_BIT(RCC->CFGR, RCC_CFGR_PPRE1_DIV4);
    SET_BIT(RCC->CFGR, RCC_CFGR_PPRE2_DIV2);

    /* 8. Настраиваем PLLCFGR для получения 168 МГц от HSI

       Формула:
         VCO_in  = HSI / PLLM   = 16 МГц / 16 = 1 МГц
         VCO_out = VCO_in * PLLN = 1 МГц * 336 = 336 МГц
         SYSCLK  = VCO_out / PLLP = 336 / 2 = 168 МГц
         PLLQ    = 7 → 336 / 7 ≈ 48 МГц (для USB, если нужно)

       Значит:
         PLLM = 16
         PLLN = 336
         PLLP = 2  (код 00b)
         PLLQ = 7
         PLLSRC = 0 (HSI)
    */

    /* Источник PLL: PLLSRC = 0 → HSI */
    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

    /* PLLM: сначала очищаем, потом пишем 16 */
    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM_Msk);
    SET_BIT(RCC->PLLCFGR, (16U << RCC_PLLCFGR_PLLM_Pos));

    /* PLLN: очищаем поле и пишем 336 */
    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN_Msk);
    SET_BIT(RCC->PLLCFGR, (336U << RCC_PLLCFGR_PLLN_Pos));

    /* PLLP: очищаем поле; код 00b = деление на 2, так что просто оставляем 0 */
    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLP_Msk);
    /* Ничего не ставим — 00b уже означает /2 */

    /* PLLQ: очищаем поле и пишем 7 */
    CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ_Msk);
    SET_BIT(RCC->PLLCFGR, (7U << RCC_PLLCFGR_PLLQ_Pos));

    /* 9. Включаем PLL и ждём, пока он поднимется */
    SET_BIT(RCC->CR, RCC_CR_PLLON);
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
    {
        /* ждём готовности PLL */
    }

    /* 10. Переключаем SYSCLK на PLL */
    CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW);   // очистить поле SW
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL); // выбрать PLL как SYSCLK

    while ((READ_BIT(RCC->CFGR, RCC_CFGR_SWS)) != RCC_CFGR_SWS_PLL)
    {
        /* ждём, пока источник SYSCLK реально не станет PLL */
    }

    /* 11. Обновляем переменную SystemCoreClock для дальнейших расчётов (SysTick и т.п.) */
    SystemCoreClock = 168000000U;
}

void SysTick_Init_1ms(void)
{
    /* Останавливаем SysTick на время настройки */
    CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);

    /* Счётчик перезагрузки:
       частота / 1000 - 1  → период 1 мс */
    uint32_t reload = SystemCoreClock / 1000U - 1U;

    // CLEAR_REG(SysTick->)
    WRITE_REG(SysTick->LOAD, reload); // значение перезагрузки
    WRITE_REG(SysTick->VAL, 0U);      // сбрасываем текущий счётчик

    /* Источник — системный такт (SYSCLK), включаем прерывания и сам таймер */
    CLEAR_BIT(SysTick->CTRL,
              SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk);

    SET_BIT(SysTick->CTRL,
            SysTick_CTRL_CLKSOURCE_Msk |   // тактировать от ядра (SYSCLK)
                SysTick_CTRL_TICKINT_Msk | // разрешить прерывания
                SysTick_CTRL_ENABLE_Msk);  // включить счётчик
}

// Прерывания от HSE
void RCC_INIT(void)
{

    /* 1) Включаем тактирование блока питания (PWR) и выставляем масштаб питания Scale1 */
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    /* VOS = 11: Scale 1 mode (максимальная частота ядра) */
    SET_BIT(PWR->CR, PWR_CR_VOS);

    /* 2) Настраиваем FLASH: кеши + задержки (5 тактов ожидания для 168 МГц) */
    // Сначала сбросим латентность
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_5WS);

    MODIFY_REG(RCC->CR, RCC_CR_HSITRIM, 0x80UL);
    CLEAR_REG(RCC->CFGR);
    while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RESET)
        ;
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != RESET)
        ;
    CLEAR_BIT(RCC->CR, RCC_CR_HSEON | RCC_CR_CSSON);
    while (READ_BIT(RCC->CR, RCC_CR_HSERDY) != RESET)
        ;
    CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);

    CLEAR_BIT(RCC->CR, RCC_CR_HSION);

    SET_BIT(RCC->CR, RCC_CR_HSEON); // Включение внешнего источника тактирования
    while (READ_BIT(RCC->CR, RCC_CR_HSERDY) == RESET)
        ;
    SET_BIT(RCC->CR, RCC_CR_CSSON); // Включение Clock Security

    // PLL configurator
    CLEAR_REG(RCC->PLLCFGR);
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_HSE);                                                            // Источник тактирования HSE
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLM_2);                                                                // деление тактирования на 4
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_5 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_8); // число 360 в  bin
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLP_0);                                                                // Деление после умножения на 4 (PLLP). теперь нужно делить на 4. Для этого передать 01
    SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ_0 | RCC_PLLCFGR_PLLQ_1 | RCC_PLLCFGR_PLLQ_2 | RCC_PLLCFGR_PLLQ_3); // Настроили PLLQ (деление после умножения на 15)

    // tact configurator
    // SET_BIT(RCC->CFGR, RCC_CFGR_SW_1);

    /* while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS_1) == RESET); */  // не запустится pll
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);                         // В качестве системного тактирования выбран PLL
    SET_BIT(RCC->CFGR, RCC_CFGR_HPRE_DIV1);                      // предделитель шины AHB1 настроен на 1 без деления
    SET_BIT(RCC->CFGR, RCC_CFGR_PPRE1_DIV4);                     // предделитель шины AHB1 настроен на 4 ОНА от 45
    SET_BIT(RCC->CFGR, RCC_CFGR_PPRE2_DIV2);                     // предделитель шины APB2 настроен на 2 ОНА от 90
    SET_BIT(RCC->CFGR, RCC_CFGR_MCO1);                           // настройка вывода на MCO1
    CLEAR_BIT(RCC->CFGR, RCC_CFGR_MCO2);                         // Настройка вывода частоты SYSCLOCK на MSO2
    SET_BIT(RCC->CFGR, RCC_CFGR_MCO1PRE_2 | RCC_CFGR_MCO1PRE_1); // Предделитель 2 для вывода на MCO1
    SET_BIT(RCC->CFGR, RCC_CFGR_MCO2PRE_2 | RCC_CFGR_MCO2PRE_1); //

    SET_BIT(FLASH->ACR, FLASH_ACR_LATENCY_5WS); // Утановка 5 циклов ожидания для FLASH памяти
    SET_BIT(RCC->CR, RCC_CR_PLLON);             // Включение PLL (?)
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == RESET)
        ;
}

// Прерывания на PC13
void ITR_Init(void)

{
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    /*//Это шутка, которая позволяет настроить мультиплексоры. Разного рода
    регистры: SYSCFG. Объединение нулевой линии (нулевые порты контроллера)
    EXTI1 - первая линия и т.д.
    Мы подключаем кнопку на PC13, на уроке PC12. Настраиваем для 12 линии
    SYSCFG разделили на 2 области, 16 битов старш другому, 16 битов младшему именно EXTI
    Там разделение на 4 линии. Во второй от 4 до 7, в третьем от 8 до 11, в четвёртом от 12 до ...
    Мы будем брать PC12, следовательно значение для PC12.
    APB2ENR (записали 1 для тактирвоания)
    */
    SET_BIT(SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI13_PC);
    /*то что на 13pc, будет выходить на контроллер прерываний*/

    /*Пропишем сами регистры EXTI, 12.3*/
    // Не хотим маскировать
    SET_BIT(EXTI->IMR, EXTI_IMR_MR13);
    // EMR пропускаем

    // Rising cl. rising trigger selection reg. по фронту, з. на RT 1
    SET_BIT(EXTI->RTSR, EXTI_RTSR_TR13);

    // Теперь спад.
    CLEAR_BIT(EXTI->FTSR, EXTI_FTSR_TR13);

    // Нужно настроить NVIC
    // см. programming manual 4.3,
    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(EXTI15_10_IRQn); // вкючаем по вектору. Все вектора в ассемблерном файле  (ext interrupts)
}

// Включение тактирования на портах
#define BTN_PORT GPIOA
#define BTN_PORT_CLK_EN() SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN)

#define BTN1_PIN 0U // PA0 → EXTI0
#define BTN2_PIN 5U // PA5 → EXTI5

void Buttons_GPIO_Init(void)
{
    BTN_PORT_CLK_EN(); // Вкл. тактирования

    /* MODER: вход (00) */
    CLEAR_BIT(BTN_PORT->MODER,
              (3U << (BTN1_PIN * 2U)) |
                  (3U << (BTN2_PIN * 2U)));

    /* OTYPER — сброс */
    CLEAR_BIT(BTN_PORT->OTYPER,
              (1U << BTN1_PIN) | (1U << BTN2_PIN));

    /* PUPDR: 01 = pull-up */
    CLEAR_BIT(BTN_PORT->PUPDR,
              (3U << (BTN1_PIN * 2U)) |
                  (3U << (BTN2_PIN * 2U)));
    SET_BIT(BTN_PORT->PUPDR,
            (1U << (BTN1_PIN * 2U)) |
                (1U << (BTN2_PIN * 2U)));
}

// Настройка прерываний кнопки: назначение EXTI на линии, прерывание по спаду 1->0. Приоритет у PA0 выше чем PA5
void Buttons_EXTI_Init(void)
{
    /* 1. Тактирование SYSCFG */
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

    /* EXTI0 (PA0): очистить в EXTICR[0] */
    CLEAR_BIT(SYSCFG->EXTICR[0], SYSCFG_EXTICR1_EXTI0); // 0000: PA[x] pin
    /* 0000 = порт A — ничего не ставим */

    /* EXTI5 (PA5): очистить в EXTICR[1], тоже порт A */
    CLEAR_BIT(SYSCFG->EXTICR[1], SYSCFG_EXTICR2_EXTI5); // 0000: PA[x] pin
    /* тоже ничего не ставим, 0000 = PAx */

    /* 2. Настроим триггеры */
    CLEAR_BIT(EXTI->RTSR, (1U << BTN1_PIN) | (1U << BTN2_PIN));
    CLEAR_BIT(EXTI->FTSR, (1U << BTN1_PIN) | (1U << BTN2_PIN));

    SET_BIT(EXTI->FTSR, (1U << BTN1_PIN) | (1U << BTN2_PIN)); // По спаду

    /* 3. Разрешим линии в IMR */
    SET_BIT(EXTI->IMR, (1U << BTN1_PIN) | (1U << BTN2_PIN));

    /* 4. NVIC — у EXTI5 свой вектор */
    NVIC_SetPriority(EXTI0_IRQn, 5);
    NVIC_EnableIRQ(EXTI0_IRQn);

    NVIC_SetPriority(EXTI9_5_IRQn, 6); // общий обработчик для EXTI5–9
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// Настройка GPIO портов. PD1, PD2, PD3, PD4, PD6, PD7
void LEDs_GPIO_Init(void)
{
    // 2 кнопки: PA0, PA5
    // 6 светодиодов
    /*
    1. PD1
    2. PD2
    3. PD3
    4. PD4
    5. PD6
    6. PD7
    */

    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);

    // 1. PD1
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE1_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT1_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR1_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD1_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR1);

    // 2. PD2
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE2_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT2_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR2_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD2_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR2);

    // 3. PD3
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE3_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT3_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR3_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD3_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR3);

    // 4. PD4
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE4_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT4_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR4_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD4_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR4);

    // 5. PD6
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE6_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT6_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR6_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD6_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR6);

    // 6. PD7
    SET_BIT(GPIOD->MODER, GPIO_MODER_MODE7_0);         // MODER: 01. выход
    CLEAR_BIT(GPIOD->OTYPER, GPIO_OTYPER_OT7_Msk);     // push-pull.
    SET_BIT(GPIOD->OSPEEDR, GPIO_OSPEEDER_OSPEEDR7_1); // 10. High speed
    CLEAR_BIT(GPIOD->PUPDR, GPIO_PUPDR_PUPD7_Msk);
    SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR7);
}
