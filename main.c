#include "platform.h"

#include "gps.h"
#include "delay.h"
#include "ui.h"
#include "button.h"

#include "LiquidCrystal_I2C.h"

extern uint32_t _isr_vectorsflash_offs;

void main(void)
{
	SystemInit();

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	delay_init();
	gps_init();
	button_init();

	LCDI2C_init(0x3f, 16, 2);
	LCDI2C_backlight();

	LCDI2C_clear();

	LCDI2C_write_String("Hello");
	delay_ms(600);

	while(1) {
		ui_process();
		button_process();

		delay_ms(200);
	}
}
