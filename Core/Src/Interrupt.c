#include "Interrupt.h"

// Выключить все 6 светодиодов
static void LED_AllOff(void)
{
    SET_BIT(GPIOD->BSRR,
            GPIO_BSRR_BR1 |
                GPIO_BSRR_BR2 |
                GPIO_BSRR_BR3 |
                GPIO_BSRR_BR4 |
                GPIO_BSRR_BR6 |
                GPIO_BSRR_BR7);
}
// Включить нужный светодиод (остальные остаются как были)
static void LED_On_Index(uint8_t index)
{
    switch (index)
    {
    case 0:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS1);
        break;
    case 1:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS2);
        break;
    case 2:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS3);
        break;
    case 3:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS4);
        break;
    case 4:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS6);
        break;
    case 5:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BS7);
        break;
    default:
        break;
    }
}
// Выключить конкретный светодиод по индексу
static void LED_Off_Index(uint8_t index)
{
    switch (index)
    {
    case 0:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR1);
        break;
    case 1:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR2);
        break;
    case 2:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR3);
        break;
    case 3:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR4);
        break;
    case 4:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR6);
        break;
    case 5:
        SET_BIT(GPIOD->BSRR, GPIO_BSRR_BR7);
        break;
    default:
        break;
    }
}
// Включаем только один активный светодиод, остальные гасим
static void LED_SetActive(uint8_t index)
{
    LED_AllOff();
    LED_On_Index(index);
}

// Состояния светодиодов
volatile uint8_t g_led_mode = 0; // 0 = мигание, 1 = постоянный свет
                                 // Активный светодиод (0..5)
volatile uint8_t g_active_led = 0;
// Индекс частоты: 0 → медленная, 1 → средняя, 2 → быстрая [0.2 Гц, 0.8 Гц, 1.3 Гц]
volatile uint8_t g_freq_index = 0;

void Interrupts_InitState(void)
{
    g_led_mode = 0;   // мигание
    g_active_led = 0; // первый светодиод
    g_freq_index = 0; // самая медленная частота
    g_blink_timer = 0;
    g_blink_on = 0;

    g_btn1_last_time = 0;
    g_btn2_last_time = 0;
    g_btn2_pressed = 0;
    g_btn2_press_ms = 0;

    LED_AllOff();
}

void SysTick_Handler(void)
{
    g_msTicks++; // общий счётчик времени (1 шаг = 1 мс)

    // Если режим = мигание
    if (g_led_mode == 0)
    {
        g_blink_timer++;

        if (g_blink_timer >= g_half_period_ms[g_freq_index])
        {
            g_blink_timer = 0;

            if (g_blink_on)
            {
                // если сейчас горит — погасить
                LED_Off_Index(g_active_led);
                g_blink_on = 0;
            }
            else
            {
                // если сейчас не горит — зажечь
                LED_On_Index(g_active_led);
                g_blink_on = 1;
            }
        }
    }
}

// Обработчик кнопки PA0
void EXTI0_IRQHandler(void)
{
    // проверяем, есть ли флаг прерывания по линии 0
    if (READ_BIT(EXTI->PR, (1U << 0)) != 0U)
    {
        // сбрасываем флаг (записать 1 в PR)
        SET_BIT(EXTI->PR, (1U << 0));

        uint32_t now = g_msTicks;

        // антидребезг
        if ((now - g_btn1_last_time) < DEBOUNCE_MS)
        {
            return;
        }
        g_btn1_last_time = now;

        // проверяем, что кнопка реально нажата (уровень 0 на PA0)
        if (READ_BIT(GPIOA->IDR, (1U << 0)) == 0U)
        {
            // следующий индeкс 0..5
            g_active_led++;
            if (g_active_led >= 6)
                g_active_led = 0;

            // при переключении: сбрасываем фазу мигания
            g_blink_timer = 0;
            g_blink_on = 0;

            if (g_led_mode == 0)
            {
                // если режим мигания — погасим, дальше SysTick сам начнёт мигание
                LED_AllOff();
            }
            else
            {
                // если режим постоянного света — сразу зажигаем новый
                LED_SetActive(g_active_led);
            }
        }
    }
}

// Обработчик кнопки PA5
void EXTI9_5_IRQHandler(void)
{
    // проверяем линию 5 (PA5)
    if (READ_BIT(EXTI->PR, (1U << 5)) != 0U)
    {
        // сброс флага
        SET_BIT(EXTI->PR, (1U << 5));

        uint32_t now = g_msTicks;

        // антидребезг
        if ((now - g_btn2_last_time) < DEBOUNCE_MS)
        {
            return;
        }
        g_btn2_last_time = now;

        // читаем текущее состояние пина PA5
        uint32_t pin = READ_BIT(GPIOA->IDR, (1U << 5));

        if (pin == 0U)
        {
            // Нажатие (спад): запоминаем время
            g_btn2_pressed = 1;
            g_btn2_press_ms = now;
        }
        else
        {
            // Отпускание (подъём), если до этого была нажата
            if (g_btn2_pressed)
            {
                g_btn2_pressed = 0;
                uint32_t dt = now - g_btn2_press_ms;

                if (dt >= LONG_PRESS_MS)
                {
                    // ДОЛГОЕ удержание: смена режима (мигание / постоянное)
                    if (g_led_mode == 0)
                    {
                        g_led_mode = 1; // теперь просто светится
                        LED_SetActive(g_active_led);
                    }
                    else
                    {
                        g_led_mode = 0; // возвращаемся к миганию
                        g_blink_timer = 0;
                        g_blink_on = 0;
                        LED_AllOff(); // дальше SysTick начнёт мигание
                    }
                }
                else
                {
                    // КРАТКОЕ нажатие: смена частоты
                    g_freq_index++;
                    if (g_freq_index >= 3)
                        g_freq_index = 0;

                    // сбрасываем фазу
                    g_blink_timer = 0;
                    g_blink_on = 0;

                    if (g_led_mode == 1)
                    {
                        // в режиме постоянного света частота ни на что не влияет,
                        // но можно перезапускать LED, чтобы логика была цельной
                        LED_SetActive(g_active_led);
                    }
                }
            }
        }
    }
}