#include "Interrupt.h"

volatile uint8_t g_currentLed = 0; // 0..3
volatile uint8_t g_speedIndex = 0; // 0..3

/* ===== TIM2 ===== */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;

        LEDs_AllOff();

        g_currentLed = (g_currentLed + 1) & 0x03;

        LED_OnIndex(g_currentLed);
    }
}

/* ===== EXTI0 (кнопка PA0) ===== */
void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR0)
    {
        EXTI->PR = EXTI_PR_PR0;

        g_speedIndex = (g_speedIndex + 1) & 0x03;
        TIM2_UpdateFrequency(g_speedIndex);
    }
}
