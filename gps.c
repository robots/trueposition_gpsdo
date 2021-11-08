#include "platform.h"

#include <string.h>

#include "utils.h"
#include "uart1.h"
#include "exti.h"
#include "gpio.h"
#include "systime.h"

#include "gps.h"

volatile int gps_do_survey = 0;
volatile int gps_do_init = 0;
volatile uint32_t gps_do_init_t = 0;

#define SURVEY_TIME 3600
#define GPS_BUF 100
char gps_buf[GPS_BUF];
int gps_buf_idx = 0;


char *gps_cmd_proceed = "$PROCEED\r\n";
char *gps_cmd_getpos = "$GETPOS\r\n";
char *gps_cmd_report = "$REPORT 1 1 5\r\n";
char *gps_cmd_ppsdbg = "$PPSDBG 1\r\n";
char *gps_cmd_survey = "$SURVEY 1\r\n";

struct gps_pos_t gps_pos;
struct gps_extstatus_t gps_extstatus;
struct gps_status_t gps_status;
struct gps_clock_t gps_clock;
struct gps_sat_t gps_sat[8];
uint32_t gps_survey_left;
struct pps_dbg_t pps_dbg;


const struct gpio_init_table_t gps_gpio[] = {
	{ // pps pin
		.gpio = GPIOA,
		.pin = GPIO_Pin_8,
		.mode = GPIO_MODE_IPD,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // LED pin
		.gpio = GPIOC,
		.pin = GPIO_Pin_13,
		.mode = GPIO_MODE_OUT_PP,
		.speed = GPIO_SPEED_HIGH,
	}
};

static void gps_recv(char *ch, uint32_t x);
static void gps_pps_inthandler(void);

void gps_init(void)
{
	gps_status.state = 10;

	gpio_init(gps_gpio, ARRAY_SIZE(gps_gpio));

	exti_set_handler(EXTI_8, gps_pps_inthandler);
	exti_enable(EXTI_8, EXTI_Trigger_Rising, EXTI_PortSourceGPIOA);

	uart1_set_rx_cb(gps_recv);
	uart1_init();
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

void gps_periodic(void)
{
	if (gps_do_survey == 1) {
		gps_survey_left = SURVEY_TIME;
		gps_do_survey = 0;

		uart1_send(gps_cmd_survey, strlen(gps_cmd_survey));
	}

	if (gps_do_init == 1) {
		uart1_send(gps_cmd_proceed, strlen(gps_cmd_proceed));

		gps_do_init_t = systime_get();
		gps_do_init = 2;
	} else if (gps_do_init == 2) {
		if (systime_get() - gps_do_init_t < SYSTIME_SEC(1)) {
			// bla
		} else {
			uart1_send(gps_cmd_ppsdbg, strlen(gps_cmd_ppsdbg));

			gps_do_init_t = systime_get();
			gps_do_init = 3;
		}
	} else if (gps_do_init == 3) {
		if (systime_get() - gps_do_init_t < SYSTIME_SEC(1)) {
			// bla
		} else {
			uart1_send(gps_cmd_report, strlen(gps_cmd_report));

			gps_do_init_t = systime_get();
			gps_do_init = 4;
		}
	} else if (gps_do_init == 4) {
		if (systime_get() - gps_do_init_t < SYSTIME_SEC(1)) {
			// bla
		} else {
			uart1_send(gps_cmd_getpos, strlen(gps_cmd_getpos));

			gps_do_init_t = systime_get();
			gps_do_init = 5;
		}
	}
}

static void gps_process(char *msg, int msglen)
{
	(void)msglen;

	if (strncmp(msg, "GETVER", 6) == 0) {
		gps_do_init = 1;
		gps_do_init_t = systime_get();
	} else if (strncmp(msg, "STATUS", 7) == 0) {
		char *p = msg + 7;
		gps_status.mhz = atou_float(&p, 0);
		p++;
		gps_status.pps = atou_float(&p, 0);
		p++;
		gps_status.ant = atou_float(&p, 0);
		p++;
		gps_status.holdover = atou_float(&p, 0);
		p++;
		gps_status.nsats = atou_float(&p, 0);
		p++;
		gps_status.state = atou_float(&p, 0);
	} else if (strncmp(msg, "EXTSTATUS", 9) == 0) {
		char *p = msg + 10;
		gps_extstatus.survey = atou_float(&p, 0);
		p++;
		gps_extstatus.sats_used = atou_float(&p, 0);
		p++;
		gps_extstatus.dop = atou_float(&p, 2);
		p++;
		gps_extstatus.temp = atou_float(&p, 2);
		p++;
		gps_extstatus.unk2 = atou_float(&p, 0);
	} else if (strncmp(msg, "CLOCK", 5) == 0) {
		char *p = msg + 6;
		gps_clock.time = atou_float(&p, 0);
		p++;
		gps_clock.leap = atou_float(&p, 0);
		p++;
		gps_clock.tfom = atou_float(&p, 0);
	} else if ((strncmp(msg, "SAT", 3) == 0) || (strncmp(msg, "WSAT", 4) == 0)) {
		int idx;
		char *p = strstr(msg, " ") + 1;
		idx = atou_float(&p, 0);
		if (idx < 8) {
			p++;
			gps_sat[idx].prn = atou_float(&p, 0);
			p++;
			gps_sat[idx].az = atou_float(&p, 0);
			p++;
			gps_sat[idx].el = atou_float(&p, 0);
			p++;
			gps_sat[idx].signal = atou_float(&p, 0);
		}
	} else if (strncmp(msg, "GETPOS", 6) == 0) {
		char *p;

		p = strstr(msg, " ") + 1; // lat
		gps_pos.lat = atou_float(&p, 0);
		p++;
		gps_pos.lon = atou_float(&p, 0);
		p++;
		gps_pos.amsl = atou_float(&p, 0);
		p++;
		gps_pos.wgsdiff = atoi_float(&p, 0);

		p = strstr(msg, " ") + 1;
	} else if (strncmp(msg, "SURVEY", 6) == 0) {
		char *p;

		p = strstr(msg, " ") + 1; // lat
		gps_pos.lat = atou_float(&p, 0);
		p++;
		gps_pos.lon = atou_float(&p, 0);
		p++;
		gps_pos.amsl = atou_float(&p, 0);
		p++;
		p = strstr(p, " ") + 1;

		gps_survey_left = atou_float(&p, 0);
	} else if (strncmp(msg, "PPSDBG", 6) == 0) {
		// "PPSDBG 1320353265 1 29.97253e3 -2 -10 0 0 31.69\r"
		char *p;

		p = strstr(msg, " ") + 1;
		/*pps_dbg.clock =*/ atou_float(&p, 0);
		p++;
		pps_dbg.status = atou_float(&p, 0);
		p++;
		pps_dbg.dac_val = atou_float(&p, 3);
		p = strstr(p, " ") + 1;
		pps_dbg.phase_offset = atoi_float(&p, 0);
		p++;
		pps_dbg.pps_offset = atoi_float(&p, 0);
		p++;
		pps_dbg.pps_status = atou_float(&p, 0);
		p++;
		pps_dbg.traim_status = atou_float(&p, 0);
		p++;
		pps_dbg.temp = atou_float(&p, 2);
		p++;
	}
}

static void gps_recv(char *ch, uint32_t x)
{
	(void)x;
	if (ch[0] == '\n') {
		gps_buf[gps_buf_idx] = 0;
		gps_process(gps_buf, gps_buf_idx);
		gps_buf_idx = 0;
	} else if (ch[0] == '$') {
		gps_buf_idx = 0;
	} else {
		gps_buf[gps_buf_idx] = ch[0];
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
