#include "platform.h"

#include "gpio.h"
#include "bsp_uart.h"

const struct gpio_init_table_t uart1_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_9,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_10,
		.mode = GPIO_MODE_IN_FLOATING,
		.speed = GPIO_SPEED_HIGH,
	},
};

const int uart1_gpio_cnt = ARRAY_SIZE(uart1_gpio);
