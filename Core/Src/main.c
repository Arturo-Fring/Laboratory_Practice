#include <stdint.h>
#include "init.h"
#include "Interrupt.h"

volatile uint8_t btnCount = 0;
volatile uint8_t flag = 0;

// Привет всем из первой лабораторной работы!
int main(void)
{
    RCC_INIT();
    ITR_Init();

    /*Настройка тактирования, светодиода */
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN);

    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);
    SET_BIT(GPIOB->MODER, GPIO_MODER_MODE7_0); /* 01 */

    /*-----------MCO2------------------------*/
    // Необхоидимо настроить пины на выход
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODER9_1); // Alt. func

    SET_BIT(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9_Msk); // Нужна высокая ск., 11.
    // CLEAR_BIT(GPIOC->AFR[1], GPIO_AFRH_AFSEL9);
    MODIFY_REG(GPIOC->AFR[1], GPIO_AFRH_AFSEL9_Msk, 0x0); // выход MCO2.

    /*-------------MCO1--------------------*/
    // PA8, AF0 (0000)

    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE8_1);
    SET_BIT(GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED8);
    CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL8);

    while (1)
    {
        if (flag)
        {
            SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS7);
        }

        else
        {
            SET_BIT(GPIOB->BSRR, GPIO_BSRR_BR7);
        }
    }
}