#ifndef GPS_h_
#define GPS_h_

#include "platform.h"

struct gps_pos_t {
	uint32_t lat;
	uint32_t lon;
	uint32_t amsl;
	int32_t wgsdiff;
};

struct gps_extstatus_t {
	uint32_t survey;
	uint32_t sats_used;
	uint32_t dop; // /100
	uint32_t temp; //  /100
	uint32_t unk2;
};

struct gps_status_t {
	uint32_t mhz;
	uint32_t pps;
	uint32_t ant;
	uint32_t holdover;
	uint32_t nsats;
	uint32_t state;
};

struct gps_clock_t {
	uint32_t time;
	uint32_t leap;
	uint32_t tfom;
};

struct gps_sat_t {
	uint32_t prn;
	uint32_t el;
	uint32_t az;
	uint32_t signal;
};

struct pps_dbg_t {
	uint32_t status;
	uint32_t dac_val;
	int32_t phase_offset;
	int32_t pps_offset;
	uint32_t pps_status;
	uint32_t traim_status;
	uint32_t temp;
};

extern struct gps_pos_t gps_pos;
extern struct gps_extstatus_t gps_extstatus;
extern struct gps_status_t gps_status;
extern struct gps_clock_t gps_clock;
extern struct gps_sat_t gps_sat[8]; 
extern uint32_t gps_survey_left;
extern struct pps_dbg_t pps_dbg;

void gps_init(void);
uint32_t gps_getunixtime(void);
void gps_survey_start(void);
void gps_periodic(void);

#endif
