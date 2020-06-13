#include "platform.h"
#include "stm32f10x.h"

#include "gpio.h"
#include "bsp_uart.h"

const struct gpio_init_table_t uart_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_9,
		.mode = GPIO_Mode_AF_PP,
		.speed = GPIO_Speed_50MHz,
	},
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_10,
		.mode = GPIO_Mode_IN_FLOATING,
		.speed = GPIO_Speed_50MHz,
	},
};

const int uart_gpio_cnt = ARRAY_SIZE(uart_gpio);
