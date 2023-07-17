/*
 * fims_trigger.cpp
 *
 *  Created on: April 22, 2020
 *      Author: kbrezina
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "libfims.h"
#include "fps_utils.h"
#include <cjson/cJSON.h>

volatile bool running = true;
fims *p_fims;

void get_URI(char method_flag, char* uri_buffer, char* key_buffer, const char* raw_uri)
{
    // Parse pub URI so subscription is to correct URI and body can be searched for desired value
    if(method_flag == 'p')
    {
        std::string uri_arg = raw_uri;
        std::size_t found = uri_arg.find_last_of("/");      //Finds where the last element of the raw URI is
        std::string element = uri_arg.substr(found+1);      //Isolate the last element of the raw URI, which is the key being searched for within the pub body
        snprintf(key_buffer, 256, "%s", element.c_str());   //Move the element name into a C-style string
        uri_arg = uri_arg.substr(0,found);                  //Isolate the actual URI that will be pubbed
        snprintf(uri_buffer, 256, "%s", uri_arg.c_str());           //Move the actual URI into a C-style string
    }
    // Get set URI without manipulating it since there will only be one value in the body
    else
    {
        snprintf(uri_buffer, 256, "%s", raw_uri);
        sprintf(key_buffer, "value");
    }
}

void signal_handler (int sig)
{
    running = false;
    p_fims->Close();
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

/**
 * @brief Attempts to parse the value with the given key from the FIMS message body.
 * @param msg FIMS message from which to parse the value.
 * @param key Key of the value inside the FIMS message body.
 * @returns double-bool pair, where the double is the parsed value and the bool is a flag that will be true if the value could not be found.
*/
inline std::pair<double, bool> parse_value(fims_message *msg, char key[256])
{
    cJSON* msg_body = cJSON_Parse(msg->body);
    cJSON* clothed_value = cJSON_GetObjectItem(msg_body, key);
    std::pair<double, bool> return_value(0.0, false);
    if (strcmp(msg->method, "set") == 0)
        return_value.first = clothed_value != NULL ? clothed_value->valuedouble : msg_body->valuedouble;
    // if not SET then PUB
    else if (clothed_value == NULL)
        return_value.second = true;
    else
        return_value.first = clothed_value->valuedouble;
    cJSON_Delete(msg_body);
    return return_value;
}

//subscribe to two URIs.  trigger a timer when a set or pub occurs on the first URI 
//and print the elapsed time when a pub or set occurs on second URI
//third arg is a value to watch for the sets or pubs
int main(int argc, char** argv)
{
    //handle invalid # of arguments
    if(argc != 9)
    {
        FPS_ERROR_PRINT("Invalid # of arguments.\nExpected Usage: ./fims_trigger -setOrPubFlag1 /uri1 -e|g|l targetValue1 -setOrPubFlag2 /uri2 -e|g|l targetValue2\n");
        p_fims->Close();
        return 1;
    }
    
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    //allocate memory for storing two URIs.  they are passed in when program is called
    //example: './fims_trigger -setOrPubFlag1 /uri1 -e|g|l targetValue1 -setOrPubFlag2 /uri2 -e|g|l targetValue2'
    char* subscriptions[2];
    memset(subscriptions, 0, sizeof(char*) * 2);
    char URI1[256];
    char URI2[256];
    char KEY1[256];
    char KEY2[256];
    char methodFlag1 = argv[1][1];
    char methodFlag2 = argv[5][1];
    // Parse the correct URIs/keys
    get_URI(methodFlag1, URI1, KEY1, argv[2]);
    get_URI(methodFlag2, URI2, KEY2, argv[6]);

    subscriptions[0] = URI1;
    subscriptions[1] = URI2;
    char operator1 = argv[3][1];
    char operator2 = argv[7][1];
    double val1 = atof(argv[4]);
    double val2 = atof(argv[8]);

    bool timer_active = false;  //boolean to track when timer is active
    timespec current_time, target_time;
    //FPS_RELEASE_PRINT("args %s, %s \n", URI1, URI2);

    p_fims = new fims();  //create new fims object

    //connect to fims
    if(p_fims->Connect((char *)"fims_trigger") == false)
    {
        FPS_ERROR_PRINT("Connect failed.\n");
        p_fims->Close();
        return 1;
    }

    //subscribe to both URIs and check that they did not fail
    if(p_fims->Subscribe((const char**)subscriptions, 2) == false)
    {
        FPS_ERROR_PRINT("Subscription failed.\n");
        p_fims->Close();
        return 1;
    }

    FPS_RELEASE_PRINT("Subscribed to URI 1: '%s' and URI 2: '%s' \n", URI1, URI2);
    if(methodFlag1 == 's')
    {
        FPS_RELEASE_PRINT("Awaiting sets on URI 1: %s. \n", URI1);
    }
    else
    {
        FPS_RELEASE_PRINT("Awaiting pubs on URI 1: %s for %s \n", URI1, KEY1);
    }

    //main loop here - check for sets/pubs on each URI.  start timer when URI 1 receives a set/pub. print elapsed timer when URI 2 receives a set/pub
    while(running && p_fims->Connected())
    {
        fims_message* msg = p_fims->Receive();
        if(msg != NULL)  //fims message received
        {
            //process URI 1 message
            if (timer_active == false && strncmp(msg->uri, URI1, strlen(URI1)) == 0)
            {
                // Check for pub or set
                if ((methodFlag1 == 'p' && strcmp(msg->method, "pub") == 0) || (methodFlag1 == 's' && strcmp(msg->method, "set") == 0))
                {
                    std::pair<double, bool> parsed_value = parse_value(msg, KEY1);
                    if (parsed_value.second)
                    {
                        FPS_ERROR_PRINT("Did not find key %s in message body. Continuing to listen...\n", KEY1);
                    }
                    else
                    {
                        double val = ((double)((int)(parsed_value.first*1000)))/1000; // These type conversions and multiply/divide simply round number to 3rd decimal place
                        if ((operator1 == 'e' && val == val1) || (operator1 == 'g' && val >= val1) || (operator1 == 'l' && val <= val1))
                        {
                            //start timer
                            if (timer_active == false)
                            {
                                FPS_RELEASE_PRINT("URI 1 received - START TIMER \n");
                                //set timer
                                clock_gettime(CLOCK_MONOTONIC, &current_time);
                                timer_active = true;  //latch timer bool
                            }
                        }
                    }
                }
            }
            //process URI 2 message
            else if (timer_active == true && strncmp(msg->uri, URI2, strlen(URI2)) == 0)
            {
                // Check for pub or set
                if ((methodFlag2 == 'p' && strcmp(msg->method, "pub") == 0) || (methodFlag2 == 's' && strcmp(msg->method, "set") == 0))
                {
                    std::pair<double, bool> parsed_value = parse_value(msg, KEY2);
                    if (parsed_value.second)
                    {
                        FPS_ERROR_PRINT("Did not find key %s in message body. Continuing to listen...\n", KEY2);
                    }
                    else
                    {
                        double val = ((double)((int)(parsed_value.first*1000)))/1000; // These type conversions and multiply/divide simply round number to 3rd decimal place
                        if ((operator2 == 'e' && val == val2) || (operator2 == 'g' && val >= val2) || (operator2 == 'l' && val <= val2))
                        {
                            //print timer
                            if (timer_active == true)
                            {
                                FPS_RELEASE_PRINT("URI 2 received - STOP TIMER \n");
                                //get and print elapsed time
                                clock_gettime(CLOCK_MONOTONIC, &target_time);
                                long int elapsed_time = ((target_time.tv_nsec - current_time.tv_nsec) / 1000000) + ((target_time.tv_sec - current_time.tv_sec) * 1000);
                                FPS_RELEASE_PRINT("Elapsed Time: %li ms \n", elapsed_time);
                                timer_active = false;  //reset timer
                                return 0;
                            }
                        }
                    }
                }
            }

            p_fims->free_message(msg);
        }
    }

    return 0;
}
