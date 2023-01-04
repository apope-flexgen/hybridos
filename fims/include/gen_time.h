
/*
 *  gen_time.h
 *
 *  Created on: April 21, 2020
 *      Author: pwilshire
 * General Time Functions
 * uses a float representation of time in seconds from a base time
 */

#ifndef __GEN_TIME_H
#define __GEN_TIME_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define gen_get_timestamp( tts )   ( clock_gettime(CLOCK_MONOTONIC, tts) )

double gen_ltime(int set);
double timeDiffMs(double ltime, int set);
double timeDiffUs(double ltime, int set);


#endif
