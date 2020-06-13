#include "platform.h"
#include "stm32f10x.h"

#include "gpio.h"
#include "bsp_i2c.h"

const struct gpio_init_table_t i2c_gpio[] = {
	{ // scl
		.gpio = GPIOB,
		.pin = GPIO_Pin_6,
		.mode = GPIO_Mode_AF_OD,
		.speed = GPIO_Speed_50MHz,
	},
	{ // sda
		.gpio = GPIOB,
		.pin = GPIO_Pin_7,
		.mode = GPIO_Mode_AF_OD,
		.speed = GPIO_Speed_50MHz,
	},
};

const int i2c_gpio_cnt = ARRAY_SIZE(i2c_gpio);
