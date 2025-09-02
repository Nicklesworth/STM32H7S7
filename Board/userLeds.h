/* SPDX-License-Identifier: Unlicense */

#ifndef USERLEDS_H_
#define USERLEDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	USER_LED_1,
	USER_LED_2,
	USER_LED_3,
	USER_LED_4,
	NUM_USER_LEDs
} user_led_t;

void userLedSet(user_led_t led, bool state);

#ifdef __cplusplus
}
#endif

#endif // USERLEDS_H_
