#include "platform.h"

#include <string.h>

#include "utils.h"
#include "uart.h"
#include "exti.h"
#include "gpio.h"

#include "gps.h"

volatile int gps_do_survey = 0;

#define GPS_BUF 100
char gps_buf[GPS_BUF];
int gps_buf_idx = 0;

char *gps_cmd_proceed = "$PROCEED\r\n";
char *gps_cmd_survey = "$SURVEY 1\r\n";

struct gps_pos_t gps_pos;
struct gps_extstatus_t gps_extstatus;
struct gps_status_t gps_status;
struct gps_clock_t gps_clock;
struct gps_sat_t gps_sat[8]; 
uint32_t gps_survey_left;


const struct gpio_init_table_t gps_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = GPIO_Pin_8,
		.mode = GPIO_Mode_IPD,
		.speed = GPIO_Speed_50MHz,
	},
	{
		.gpio = GPIOC,
		.pin = GPIO_Pin_13,
		.mode = GPIO_Mode_Out_PP,
		.speed = GPIO_Speed_50MHz,
	}
};

static void gps_recv(char ch);
static void gps_pps_inthandler(void);

void gps_init(void)
{
	GPIO_InitBatch(gps_gpio, ARRAY_SIZE(gps_gpio));

	exti_set_handler(EXTI_8, gps_pps_inthandler);
	exti_enable(EXTI_8, EXTI_Trigger_Rising, EXTI_PortSourceGPIOA);

	uart_set_cb(gps_recv);
	uart_init();
}

uint32_t gps_getunixtime(void)
{
	uint32_t out = gps_clock.time;
	out += 315964800;
	out -= gps_clock.leap;
	return out;
}

void gps_survey_start(void)
{
	gps_do_survey = 1;
}

static void gps_process(char *msg, int msglen)
{
	if (strncmp(msg, "GETVER", 6) == 0) {
		uart_send(gps_cmd_proceed, strlen(gps_cmd_proceed));
	} else if (strncmp(msg, "STATUS", 6) == 0) {
		char *p = msg + 7;
		gps_status.mhz = atoi_float(&p, 0);
		p++;
		gps_status.pps = atoi_float(&p, 0);
		p++;
		gps_status.ant = atoi_float(&p, 0);
		p++;
		gps_status.holdover = atoi_float(&p, 0);
		p++;
		gps_status.nsats = atoi_float(&p, 0);
		p++;
		gps_status.state = atoi_float(&p, 0);
	} else if (strncmp(msg, "EXTSTATUS", 9) == 0) {
		char *p = msg + 10;
		gps_extstatus.survey = atoi_float(&p, 0);
		p++;
		gps_extstatus.unk3 = atoi_float(&p, 0);
		p++;
		gps_extstatus.pos_dilution = atoi_float(&p, 2);
		p++;
		gps_extstatus.temperature = atoi_float(&p, 2);
		p++;
		gps_extstatus.unk2 = atoi_float(&p, 0);
	} else if (strncmp(msg, "CLOCK", 5) == 0) {
		char *p = msg + 6;
		gps_clock.time = atoi_float(&p, 0);
		p++;
		gps_clock.leap = atoi_float(&p, 0);
		p++;
		gps_clock.tfom = atoi_float(&p, 0);
	} else if ((strncmp(msg, "SAT", 3) == 0) || (strncmp(msg, "WSAT", 4) == 0)) {
		int idx;
		char *p = strstr(msg, " ") + 1;
		idx = atoi_float(&p, 0);
		if (idx < 8) {
			p++;
			gps_sat[idx].prn = atoi_float(&p, 0);
			p++;
			gps_sat[idx].az = atoi_float(&p, 0);
			p++;
			gps_sat[idx].el = atoi_float(&p, 0);
			p++;
			gps_sat[idx].signal = atoi_float(&p, 0);
		}
	} else if (strncmp(msg, "GETPOS", 6) == 0) {
		char *p;

		p = strstr(msg, " ") + 1; // lat
		gps_pos.lat = atoi_float(&p, 0);
		p++;
		gps_pos.lon = atoi_float(&p, 0);
		p++;
		gps_pos.amsl = atoi_float(&p, 0);
		p++;
		p = strstr(msg, " ") + 1;
	} else if (strncmp(msg, "SURVEY", 6) == 0) {
		char *p;

		p = strstr(msg, " ") + 1; // lat
		gps_pos.lat = atoi_float(&p, 0);
		p++;
		gps_pos.lon = atoi_float(&p, 0);
		p++;
		gps_pos.amsl = atoi_float(&p, 0);
		p++;
		p = strstr(p, " ") + 1;

		gps_survey_left = atoi_float(&p, 0);
	}


	if (gps_do_survey == 1) {
		gps_do_survey = 0;

		uart_send(gps_cmd_survey, strlen(gps_cmd_survey));
	}
}

static void gps_recv(char ch)
{
	if (ch == '\n') {
		gps_buf[gps_buf_idx] = 0;
		gps_process(gps_buf, gps_buf_idx);
		gps_buf_idx = 0;
	} else if (ch == '$') {
		gps_buf_idx = 0;
	} else {
		gps_buf[gps_buf_idx] = ch;
		gps_buf_idx ++;
		if (gps_buf_idx > GPS_BUF) {
			gps_buf_idx = 0;
		}
	}
}

static void gps_pps_inthandler(void)
{
	if (GPIOC->ODR & GPIO_Pin_13) {
		GPIOC->BRR = GPIO_Pin_13;
	} else {
		GPIOC->BSRR = GPIO_Pin_13;
	}
}
