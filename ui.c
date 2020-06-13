#include "platform.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "gps.h"
#include "delay.h"
#include "LiquidCrystal_I2C.h"

static char line1[16];
static char line2[16];

const char *ui_states[] = {
	"Locked ",
	"Acquisi",
	"Initial",
	"Holdovr",
	"ForcHld",
	"SoftHld",
	"NoGPS",
	"TrainXO",
	"Waiting"
};

static const char *state2str(int state)
{
	if (state <= 7) {
		return ui_states[state];
	}
	return ui_states[8];
}

void ui_change(void)
{

}

void ui_process(void)
{
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
		sprintf(line1, "%02d.%02d.%02d%cT=%02d.%02d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year % 100, error, gps_extstatus.temperature / 100, gps_extstatus.temperature % 100);
		sprintf(line2, "%02d:%02d:%02d %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, state2str(gps_status.state));
	} else {
		sprintf(line1, "%02d.%02d.%02d%cSurvey!", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year % 100, error);
		sprintf(line2, "%02d:%02d:%02d t-%05d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, gps_survey_left);
	}

	LCDI2C_setCursor(0,0);
	LCDI2C_write_String(line1);
	LCDI2C_setCursor(0,1);
	LCDI2C_write_String(line2);
}
