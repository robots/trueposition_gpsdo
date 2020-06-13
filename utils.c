#include "platform.h"

#include "utils.h"

/*
 * converts string in format "YYYY.XX" to int YYYYXX
 * where number of X's is x
 */
uint32_t atoi_float(char **in, int x)
{
	char *p = *in;
	uint32_t out = 0;
	int i = x;
	int dot = 0; // are we beyond the "."

	while ((*p >= '0' && *p <= '9') || *p == '.') {
		if (*p == '.') {
			dot = 1;
			p++;
			continue;
		}

		if (dot == 1) {
			if (i == 0) {
				break;
			}
			i--;
		}

		out *= 10;
		out += *p - '0';
		p++;
	}

	*in = p;

	while (i > 0) {
		out *= 10;
		i--;
	}

	return out;
}
