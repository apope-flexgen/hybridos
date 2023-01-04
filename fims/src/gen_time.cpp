/*
 *  gen_time.c
 *
 *  Created on: April 21, 2020
 *      Author: pwilshire
 * General Time Functions
 */
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h> 
#include <sys/types.h>
#include <pthread.h>

#include "gen_time.h"



double g_base_time = -1.0;
double g_saved_time = 0.0;
pthread_mutex_t ltime_lock = PTHREAD_MUTEX_INITIALIZER;

static inline double get_raw_timestamp(void)
{
    struct timespec now;
    int rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc != 0) return 0.0;
    return (double)(now.tv_sec) + ((double)now.tv_nsec) / 1000000000.0;
}


// get a time for logging
// do not always get a new time
// the lock does not use a system call if uncontested
double gen_ltime(int set)
{
    double val;
    pthread_mutex_lock(&ltime_lock);

    if(g_base_time < 0.0) {
        g_base_time = get_raw_timestamp();
        set = 1;
    }
    if(set == 1 ) {
        g_saved_time = (double)(get_raw_timestamp()-g_base_time);
    }
    val = g_saved_time;
    pthread_mutex_unlock(&ltime_lock);
    return val;
}


double timeDiffMs(double ltime, int set)
{
    return ((gen_ltime(set) - ltime) * 1000.0);
}

double timeDiffUs(double ltime, int set)
{
    return ((gen_ltime(set) - ltime) * 1000000.0);
}
