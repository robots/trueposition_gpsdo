#include "platform.h"

#include "gps.h"
#include "delay.h"
#include "systime.h"
#include "ui.h"
#include "button.h"

#include "LiquidCrystal_I2C.h"

extern uint32_t _isr_vectorsflash_offs;

static void button_handler(uint32_t btn, uint32_t ev)
{
	if (btn == 0) {
		if (ev == BUTTON_EV_LONG) {
			gps_survey_start();
		} else if (ev == BUTTON_EV_SHORT) {
			ui_change(-1);
		}
	}

	if (btn == 1) {
		if (ev == BUTTON_EV_SHORT) {
			ui_change(1);
		}
	}
}

int main(void)
{
	SystemInit();

	SCB->VTOR = (uint32_t)&_isr_vectorsflash_offs;

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= (0x02 << 24); // only swj
//	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;

	systime_init();
	delay_init();
	gps_init();
	button_init();
	button_set_ev_handler(button_handler);

	LCDI2C_init(0x27, 16, 2);
	LCDI2C_backlight();

	LCDI2C_clear();

	LCDI2C_write_String("GPSDO v0.2");
	//delay_ms(600);
	systime_delay(600);

	while(1) {
		ui_process();
		button_process();
		gps_periodic();
	}
}
