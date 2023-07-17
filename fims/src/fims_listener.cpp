/*
 * FIMS_Listener.cpp
 *
 *  Created on: Jun 14, 2018
 *      Author: jcalcagni
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include "libfims.h"
#include "fps_utils.h"

volatile bool running = true;
fims fims_gateway;

void signal_handler (int sig)
{
    running = false;
    fims_gateway.Close();
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

static inline char* get_raw_timestamp(char* buffer, int blen)
{
    char time_str[64];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm local_tm;
    strftime(time_str,64,"%Y-%m-%d %T.", localtime_r(static_cast<time_t*>(&tv.tv_sec), &local_tm));
    snprintf(buffer, blen,"%s%06ld",time_str,tv.tv_usec);
    return buffer;
}

int main(int argc, char** argv)
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    char* subs = NULL;
    char* method = NULL;
    int num_msgs_to_listen_for = -1;
    int method_len = 0;
    char  subs_array[128];
    int arg;
    bool publish_only = false;
    opterr = 0;

    while ((arg = getopt (argc, argv, "u:m:n:")) != -1)
    {
        switch(arg)
        {
        case 'u':
            subs = optarg;
            break;
        case 'm': // fims_listen can subscribe to either sets or pubs
            method = optarg;
            method_len = strlen(method); // pre-calculate method length for parsing downstream
            break;
        case 'n':
            num_msgs_to_listen_for = atoi(optarg);
            break;
        case '?':
            if(optopt == 'u' || optopt == 'm' || optopt == 'n')
            {
                FPS_RELEASE_PRINT("Options -%c requires an argument\n\n", optopt);
            }
            /* no break */
        default:
            FPS_RELEASE_PRINT("usage: fims_listen <options> [args]\n"
                              " options:\n"
                              " -u sets the uri to subscribe to\n"
                              " -m sets the message method type to filter\n"
                              " -n set the number of messages to collect before termination (if none provided, will listen indefinitely)\n");
            return 1;
            break;
        }
    }

    if (fims_gateway.Connect((char *)"fims_listen") == false)
    {
        FPS_ERROR_PRINT("Connect failed.\n");
        fims_gateway.Close();
        return 1;
    }

    if (subs == NULL)
    {
        subs = subs_array;
        sprintf(subs,"/");
    }

    // if user selects pub, then change option passed to Subscribe()
    if(method_len > 0)
    {
        if (strncmp(method,"pub", method_len) == 0)
        {
            publish_only = true;
        }
    }
    if (fims_gateway.Subscribe((const char**)&subs, 1, (bool *)&publish_only) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        fims_gateway.Close();
        return 1;
    }

    char buffer[128];
    while (running && fims_gateway.Connected() && num_msgs_to_listen_for != 0)
    {
        fims_message* msg = fims_gateway.Receive();
        if (msg)
        {
            // print all messages if method not specified, or print message if method matches
            if (method == NULL || strncmp(method, msg->method, method_len) == 0)
            {
                FPS_RELEASE_PRINT(R"(
Method:       %s
Uri:          %s
ReplyTo:      %s
Process Name: %s 
Username:     %s
Body:         %s 
Timestamp:    %s
)",
                    msg->method, msg->uri, msg->replyto, msg->process_name, msg->username, msg->body, get_raw_timestamp(buffer, sizeof(buffer)));
                if (num_msgs_to_listen_for > 0)
                {
                    num_msgs_to_listen_for--;
                }
            }
            fims_gateway.free_message(msg);
        }
    }

    return 0;
}
