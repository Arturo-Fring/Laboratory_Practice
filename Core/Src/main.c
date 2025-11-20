#include "init.h"
#include "Interrupt.h"

int main(void)
{
    Clock_Init_HSE_PLL_168MHz();
    GPIO_EXTI_Init();
    TIM2_Init();

    LEDs_AllOff();
    LED_OnIndex(0);

    while (1)
    {
        __WFI(); // сон до прерывания
    }
}
