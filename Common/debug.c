/* SPDX-License-Identifier: Unlicense */
#include "common.h"

int __io_putchar(int ch)
{
    uint8_t c = ch & 0xFF;
    HAL_UART_Transmit(&huart4, &c, 1, HAL_MAX_DELAY);
    return ch;
}