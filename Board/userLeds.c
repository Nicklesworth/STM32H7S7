/* SPDX-License-Identifier: Unlicense */

#include <assert.h>
#include "userLeds.h"
#include "main.h"
#include "stm32h7rsxx_hal_gpio.h"

typedef struct
{
	GPIO_TypeDef* port;
	uint32_t pin;
} led_info_t;

static led_info_t led_info[NUM_USER_LEDs] = 
{
	{LED1_GPIO_Port, LED1_Pin},
	{LED2_GPIO_Port, LED2_Pin},
	{LED3_GPIO_Port, LED3_Pin},
	{LED4_GPIO_Port, LED4_Pin},
};

void userLedSet(user_led_t led, bool state)
{
	assert(led < NUM_USER_LEDs);
	HAL_GPIO_WritePin(led_info[led].port, led_info[led].pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}