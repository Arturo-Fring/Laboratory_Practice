#include <stdint.h>
#include <stdbool.h>
#include "../../CMSIS/Devices/STM32F4xx/Inc/STM32F429ZI/stm32f429xx.h"
#include "stm32f4xx.h"

void GPIO_init__Memory(void);
void GPIO_init_With_Myself_Macros(void);
void GPIO_init_With_Myself_Macros2(void);
void GPIO_init_With_Myself_Macros3(void);
void GPIO_init_CMSIS(void);

#define RCC_AHB1ENR *(uint32_t *)(0x40023800UL + 0x30UL)
#define RCC_GPIOA_EN 0x01UL
#define RCC_GPIOB_EN 0x03UL
#define RCC_GPIOC_EN 0x07UL

#define GPIOA_IDR *(uint32_t *)(0x40020000UL + 0x10UL)
#define GPIOA_MODER *(uint32_t *)(0x40020000UL + 0x00UL)
#define GPIOA_OTYPER *(uint32_t *)(0x40020000UL + 0x04UL)
#define GPIOA_OSPEEDR *(uint32_t *)(0x40020000UL + 0x08UL)
#define GPIOA_BSRR *(uint32_t *)(0x40020000UL + 0x18UL)

#define GPIOB_MODER *(uint32_t *)(0x40020400UL + 0x00UL)
#define GPIOB_OTYPER *(uint32_t *)(0x40020400UL + 0x04UL)
#define GPIOB_OSPEEDR *(uint32_t *)(0x40020400UL + 0x08UL)
#define GPIOB_BSRR *(uint32_t *)(0x40020400UL + 0x18UL)

#define GPIOC_IDR *(uint32_t *)(0x40020800UL + 0x10UL)
#define GPIOC_MODER *(uint32_t *)(0x40020800UL + 0x00UL)
#define GPIOC_OTYPER *(uint32_t *)(0x40020800UL + 0x04UL)
#define GPIOC_OSPEEDR *(uint32_t *)(0x40020800UL + 0x08UL)
#define GPIOC_BSRR *(uint32_t *)(0x40020800UL + 0x18UL)

#define GPIO_PIN_MED_14 0x10000000UL
#define GPIO_PIN_OUT_14 0x10000000UL
#define GPIO_PIN_RESET_14 0x40000000UL
#define GPIO_PIN_SET_14 0x4000UL

#define GPIO_PIN_MED_7 0x4000UL
#define GPIO_PIN_OUT_7 0x4000UL
#define GPIO_OFF 0x00UL
#define GPIO_PIN_RESET_7 0x800000UL
#define GPIO_PIN_SET_7 0x80UL
#define GPIO_PIN_IDR_13 0x2000UL

// Для пина PA3(A0) и PC3(A2)
#define GPIO_PIN_MED_3 0x4000UL
#define GPIO_PIN_IN_3 0x00UL
#define GPIO_PIN_OUT_3 0x40UL
#define GPIO_OFF 0x00UL
#define GPIO_PIN_RESET_3 0x80000UL
#define GPIO_PIN_SET_3 0x08UL
#define GPIO_PIN_IDR_3 0x08UL

// Для пина PC0(A1)
#define GPIO_PIN_MED_0 0x01UL
#define GPIO_PIN_OUT_0 0x01UL
#define GPIO_PIN_IN_0 0x00UL
#define GPIO_OFF 0x00UL
#define GPIO_PIN_RESET_0 0x10000UL
#define GPIO_PIN_SET_0 0x01UL
#define GPIO_PIN_IDR_0 0x01UL

#define BIT_SET(REG, BIT) ((REG) |= (BIT))
#define BIT_READ(REG, BIT) ((REG) & (BIT))

// LB 2

void RCC_INIT(void);
void ITR_Init(void);