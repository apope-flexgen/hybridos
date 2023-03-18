/*
 * This test code creates a new fims connection and generates 
 * "post" based fims messages to fims_server.
 * Feel free to change the body of fims message as you desire
 * ---------g++ command to build this code-----------
 * g++ --std=c++11 -L/usr/local/lib -o tester testEvents.cpp -Wl,-rpath,/usr/local/lib -lcjson -lfims
 * make sure cjson library rpm is installed
 * make sure fims library is built and installed
 * 
 * The above g++ command creates binary called tester.
 * All you need is to run the binary, and it should be posting some fims messages
 * to fims_server(if running)
 * 
 * on the other command window, run "./fims_listen -s /events"
 * It should start printing events with source as "events-test"
 */

#include <stdio.h>
#include <iostream>
#include <cjson/cJSON.h>
#include <fims/fps_utils.h>
#include <fims/libfims.h>
#include <unistd.h>

using namespace std;
void emit_event(fims* p_fims, const char* source, const char* message, int severity);

int main()
{
    fims *p_fims;
    if ((p_fims = new fims()) == NULL)
    {
        cout<<"Failed to initialize fims class."<<endl;
        return (1);
    }

    if ( !p_fims->Connect((char *)"events_testscript") )
    {
        cout<< "Failed to connect to fims server." << endl;
        delete p_fims;
        return 1;
    }
    if (!p_fims) {
        cout<<"fims object creation error"<<endl;
        return 1;
    }
    cout<<"Starting events generation"<<endl;
    while(true){
        emit_event(p_fims, "events-test", "Site Manager state changed to Startup",
                    2);
        sleep(1);
        emit_event(p_fims, "events-test", "Site Manager state changed to Start ESS",
                    1);
        sleep(1);
        emit_event(p_fims, "events-test", "Site Manager state changed to Stop ESS",
                    1);
        sleep(1);
        emit_event(p_fims, "events-test", "Site Manager state changed to Shutdown",
                    3);
        sleep(1);
    }
    cout<<"end of test script"<<endl;
    p_fims->Close();
    delete p_fims;
    return 0;
}

void emit_event(fims* p_fims, const char* source, const char* message, int severity)
{
    cJSON* JSON_object;
    JSON_object = cJSON_CreateObject();
    cJSON_AddStringToObject(JSON_object, "source", source);
    cJSON_AddStringToObject(JSON_object, "message", message);
    cJSON_AddNumberToObject(JSON_object, "severity", severity);
    char* body = cJSON_PrintUnformatted(JSON_object);
    cJSON_Delete(JSON_object);
    p_fims->Send("post", "/events", NULL, body);
    free(body);
}