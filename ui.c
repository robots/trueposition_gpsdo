#include "platform.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

#include "gps.h"
#include "delay.h"
#include "LiquidCrystal_I2C.h"

enum {
	SCREEN_1=0,
	SCREEN_2,
	SCREEN_3,
	SCREEN_4,
	SCREEN_LAST
};

uint32_t ui_screen = 0;

static char line1[17];
static char line2[17];

const char *ui_states[] = {
	"Locked ",
	"Acquisi",
	"Initial",
	"Holdovr",
	"ForcHld",
	"SoftHld",
	"NoGPS  ",
	"TrainXO",
	"Waiting"
};

const char *ui_traim[] = {
	"Not",
	"Det",
	"OK ",
};

const char *ui_traim_pps[] = {
	"OK",
	"KO",
	"--",
};

static const char *state2str(unsigned int state)
{
	if (state <= 7) {
		return ui_states[state];
	}
	return ui_states[8];
}

static const char *traim_str(unsigned int traim)
{
	if (traim < ARRAY_SIZE(ui_traim)) {
		return ui_traim[traim];
	}
	return ui_traim[0];
}

static const char *traim_pps_str(unsigned int traim)
{
	if (traim < ARRAY_SIZE(ui_traim_pps)) {
		return ui_traim_pps[traim];
	}
	return ui_traim_pps[2];
}

void ui_change(int i)
{
	if (i < 0) {
		if (ui_screen == 0) {
			ui_screen = SCREEN_LAST-1;
		} else {
			ui_screen -= 1;
		}
	} else {
		ui_screen += 1;
		if (ui_screen == SCREEN_LAST) {
			ui_screen = 0;
		}
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
void ui_process(void)
{
	line1[0] = 0;
	line2[0] = 0;

	if (ui_screen == SCREEN_1) {
		time_t time = gps_getunixtime();
		struct tm * timeinfo;
		timeinfo = localtime(&time);
		char error = ' ';

		if (gps_status.mhz) {
			error = '~';
		} else if (gps_status.pps) {
			error = 'p';
		} else if (gps_status.ant) {
			error = 'Y';
		}

		if (gps_extstatus.survey == 0) {
			sprintf(line1, "%02d.%02d.%02d%cT=%02d.%02d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year % 100, error, gps_extstatus.temp / 100, gps_extstatus.temp % 100);
			sprintf(line2, "%02d:%02d:%02d %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, state2str(pps_dbg.status));
		} else {
			sprintf(line1, "%02d.%02d.%02d%cSurvey!", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year % 100, error);
			sprintf(line2, "%02d:%02d:%02d t-%05d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, gps_survey_left);
		}
	} else if (ui_screen == SCREEN_2) {
		sprintf(line1, "SAT:%02d DOP:%02d.%02d", gps_extstatus.sats_used, gps_extstatus.dop / 100, gps_extstatus.dop % 100);
		sprintf(line2, "Traim:%s PPS:%s", traim_str(pps_dbg.traim_status), traim_pps_str(pps_dbg.pps_status));
	} else if (ui_screen == SCREEN_3) {
		sprintf(line1, "DAC:%05d dPPS%2d", pps_dbg.dac_val, pps_dbg.pps_offset);
		sprintf(line2, "dPh:%03d  T=%02d.%02d", pps_dbg.phase_offset, pps_dbg.temp / 100, pps_dbg.temp % 100);
	} else if (ui_screen == SCREEN_4) {
		// {lat = 50108055, lon = 14394150, amsl = 217, wgsdiff = 0}}
		sprintf(line1, "%02d.%06d   %4d", gps_pos.lat / 1000000, gps_pos.lat % 1000000, gps_pos.amsl);
		sprintf(line2, "%03d.%06d      ", gps_pos.lon / 1000000, gps_pos.lon % 1000000);
	}

	LCDI2C_setCursor(0,0);
	LCDI2C_write_String(line1);
	LCDI2C_setCursor(0,1);
	LCDI2C_write_String(line2);
}
#pragma GCC diagnostic pop
