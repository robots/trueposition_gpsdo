#include "platform.h"

#include "gpio.h"
#include "ui.h"
#include "gps.h"
#include "button.h"

#define BUTTON_LONG 40
#define BUTTON_SHORT 2

const struct gpio_init_table_t button_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_15,
		.mode = GPIO_Mode_IPU,
		.speed = GPIO_Speed_50MHz,
	},
};

static int button_counter;

void button_init(void)
{
	GPIO_InitBatch(button_gpio, ARRAY_SIZE(button_gpio));
	button_counter = 0;
}

void button_process(void)
{
	if (GPIO_Get(&button_gpio[0]) == 0) {
		button_counter ++;
	} else {
		if (button_counter >= BUTTON_LONG) {
			gps_survey_start();
		} else if (button_counter >= BUTTON_SHORT) {
			ui_change();
		}


		button_counter = 0;
	}
}
