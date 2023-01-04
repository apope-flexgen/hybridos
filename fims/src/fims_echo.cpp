/*
 * FIMS_Echo.cpp
 *
 *  Created on: Jun 19, 2018
 *      Author: jcalcagni
 */


#include <string.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "libfims.h"
#include "fps_utils.h"

volatile bool running = true;
fims *p_fims;


void signal_handler (int sig)
{
    running = false;
    p_fims->Close();
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

int main(int argc, char** argv)
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    char *uri     = NULL;
    char *body    = NULL;
    int arg;
    opterr = 0;
    while((arg = getopt (argc, argv, "u:b:")) != -1)
    {
        switch(arg)
        {
        case 'u':
            uri = optarg;
            break;
        case 'b':
            body = optarg;
            break;
        case '?':
            if(optopt == 'u')
            {
                FPS_RELEASE_PRINT("Option -%c requires an argument\n\n", optopt);
            }
            /* no break */
        default:
            FPS_RELEASE_PRINT("usage: fims_echo <options> [body]\n"
                              " required option -u\n\n"
                              " options:\n"
                              " -u   sets the uri to echo for\n" );
            return 1;
            break;
        }
    }

    if(uri == NULL)
    {
        FPS_ERROR_PRINT("No uri given to echo on.\n");
        return 1;
    }

    FPS_ERROR_PRINT("Listening on %s replying with %s.\n", uri, body != NULL ? body : "body from message");

    
    p_fims = new fims();

    if(p_fims->Connect((char *)"fims_echo") == false)
    {
        FPS_ERROR_PRINT("Connect failed.\n");
        p_fims->Close();
        return 1;
    }

    if(p_fims->Subscribe((const char**)&uri, 1) == false)
    {
        FPS_ERROR_PRINT("Subscription to uri %s failed.\n", uri);
        p_fims->Close();
        return 1;
    }

    while(running == true && p_fims->Connected())
    {
        fims_message* msg = p_fims->Receive();

        if(msg != NULL)
        {
            if(msg->replyto != NULL && strcmp(msg->uri, uri) == 0 && strcmp(msg->method, "pub") != 0)
            {
                p_fims->Send("set", msg->replyto, NULL, body != NULL ? body : msg->body, msg->username);
            }
            p_fims->free_message(msg);
        }
    }
    p_fims->Close();
    return 0;
}
