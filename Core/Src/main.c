#include "init.h"
#include "Interrupt.h"

int main(void)
{
    /* 1. Тактирование на 168 МГц */
    Clock_Init_HSE_PLL_168MHz();

    /* 2. SysTick 1 мс для времени и антидребезга */
    SysTick_Init_1ms();

    /* 3. GPIO (LED + кнопка) + EXTI0 */
    GPIO_EXTI_Init();

    /* 4. TIM2 (ещё НЕ запущен) */
    TIM2_Init();

    /* 5. По умолчанию всё выключено, гирлянда не крутится */
    LEDs_AllOff();
    g_running = 0U;
    g_currentLed = 0U;
    g_speedIndex = 0U;

    while (1)
    {
        /* Спим до прерывания (TIM2 / EXTI / SysTick) */
        __WFI();
    }
}
