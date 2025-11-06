#include "../Inc/init.h"
#include "init.h"

void RCC_INIT(void)
{

    // // GPIO INIT
    // RCC_AHB1ENR |= RCC_GPIOA_EN | RCC_GPIOB_EN | RCC_GPIOC_EN; // светодиод, выход MCO

    // BIT_SET(GPIOB_MODER, GPIO_PIN_OUT_7);
    // BIT_SET(GPIOB_OTYPER, GPIO_OFF);
    // BIT_SET(GPIOB_OSPEEDR, GPIO_PIN_MED_7);
    // BIT_SET(GPIOB_BSRR, GPIO_PIN_RESET_7);

    SET_BIT(GPIOC->MODER, GPIO_MODER_MODER9_1);           // Настраиваем пин на альтернативный режим
    SET_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9_Msk);    // Настраиваем пин на максимальную скорость работы
    MODIFY_REG(GPIOC->AFR[1], GPIO_AFRH_AFSEL9_Msk, 0x0); // Выбираем тип альтернативной функции – Выход MCO2

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
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);                        // В качестве системного тактирования выбран PLL
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

    //Нужно настроить NVIC
    //см. programming manual 4.3, 
    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,0));
    NVIC_EnableIRQ(EXTI15_10_IRQn); //вкючаем по вектору. Все вектора в ассемблерном файле  (ext interrupts)

    


}