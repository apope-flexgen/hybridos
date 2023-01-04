/*
 * FIMS_Send.cpp
 *
 *  Created on: Jun 15, 2018
 *      Author: jcalcagni
 */

#include <string>
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

    char *replyto = NULL;
    char *method  = NULL;
    char *uri     = NULL;
    char *body    = NULL;
    char *fname   = NULL;
    char *uname   = NULL;
    int arg;
    opterr = 0;
    while((arg = getopt (argc, argv, "m:r:u:n:f:")) != -1)
    {
        switch(arg)
        {
        case 'm':
            method = optarg;
            break;
        case 'r':
            replyto = optarg;
            break;
        case 'u':
            uri = optarg;
            break;
        case 'f':
            fname = optarg;
            break;
        case 'n':
            uname = optarg;
            break;
        case '?':
            if(optopt == 'm' || optopt == 'u' || optopt == 'r' || optopt == 'f'|| optopt == 'n')
            {
                FPS_RELEASE_PRINT("Options -%c requires an argument\n\n", optopt);
            }
            /* no break */
        default:
            FPS_RELEASE_PRINT("usage: fims_send <options> [body]\n"
                              " required options are -m and -u\n\n"
                              " options:\n"
                              " -m   sets the message method type\n"
                              " -r   sets the reply to uri of message\n"
                              " -u   sets the uri of the message\n"
                              " -n   sets the user name\n"
                              " -f   sends the file content as a fims message\n" );
            return 1;
            break;
        }
    }

    if(optind < argc)
    {
        body = argv[optind];
    }

    p_fims = new fims();

    if(p_fims->Connect((char *)"fims_send") == false)
    {
        FPS_ERROR_PRINT("Connect failed.\n");
        p_fims->Close();
        return 1;
    }

    if(replyto != NULL)
    {
        if(p_fims->Subscribe((const char**)&replyto, 1) == false)
        {
            FPS_ERROR_PRINT("Subscription to replyto failed.\n");
            p_fims->Close();
            return 1;
        }
    }

    if (fname)
    {
        FILE *f = fopen(fname, "rb");
        if (!f)
        {
            FPS_ERROR_PRINT("input file %s not found \n", fname);
        }
        else
        {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *string = (char *)malloc(fsize + 1);
            body = string;
            fread(string, fsize, 1, f);
            fclose(f);

            string[fsize] = 0;

            FPS_DEBUG_PRINT("file size %ld\n", fsize);
        }
    }

    p_fims->Send(method, uri, replyto, body, uname);

    if(replyto != NULL)
    {
        fims_message* msg = p_fims->Receive_Timeout(FIVE_SECONDS);
        if(msg == NULL)
        {
            FPS_ERROR_PRINT("Receive Timeout.\n");
        }
        else
        {
            FPS_RELEASE_PRINT("%s\n", msg->body);
            p_fims->free_message(msg);
        }
    }
    p_fims->Close();
    return 0;
}
