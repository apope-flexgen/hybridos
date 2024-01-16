#!/usr/bin/python3

# Replays the fims message retrieved from the dump of fims messages retrieved
# from a fims listen

import sys
import subprocess
import signal
import time
import datetime

def alarm_handler(signum, frame):
    raise TimeoutExpired

def input_to(prompt, timeout):
    # set signal handler
    signal.signal(signal.SIGALRM, alarm_handler)
    signal.alarm(timeout) # produce SIGALRM in `timeout` seconds

    try:
        return input(prompt)
    finally:
        signal.alarm(0) # cancel alarm
    return "y"    

# Dictionary of component uri's to replace during replay
uri_replacements = {
    "/site/ess_hs": "/replay_site/ess_hs",
    "/site/ess_ls": "/replay_site/ess_ls"
}

def usage():
    print("Usage info:\n    python3 fims_replay.py [fims_output_file]")

# Parses a file containing a dump of fims messages
def parse_fims_dump(file_src):
    try:
        f_r = open(file_src)
    except OSError:
        print("Error: Unable to read/open file: ", file_src)
        sys.exit()
    print("OK running with  file: ", file_src)

    # Every fims message contains the following group of items:
    # Method, Body, Uri, Body, and Timestamp
    # We'll just extract all except the Timestamp and use them for replay
    with f_r:
        lines = f_r.readlines()
        methd, uri, reply_to, body = "", "", "", ""
        for line in lines:
            print(" got line ", line)
            if "Method" in line:
                methd = line.split("Method:", 1)[1].strip()
            elif "Uri" in line:
                uri = line.split("Uri:", 1)[1].strip()
                if uri in uri_replacements:
                    print("Found uri {0} in dictionary. New uri is now {1}".format(uri, uri_replacements[uri]))
                    uri = uri_replacements[uri]
            elif "ReplyTo" in line:
                reply_to = line.split("ReplyTo:", 1)[1].strip()
                if "(null)" in reply_to:
                    reply_to = ""
            elif "Body" in line:
                body = line.split("Body:", 1)[1].strip()
                if "(null)" in body:
                    body = ""
            #"Timestamp":"04-30-2021 05:47:36.99319"}
            elif "Timestamp" in line:
                times = line.split("Timestamp:", 1)[1].strip()
                #import datetime 
                dt_string = times #"2020-12-18 3:11:09" 
                format = "%Y-%m-%d %H:%M:%S.%f"
                dt_object = datetime.datetime.strptime(dt_string, format)
                print("Datetime: ", dt_object)

                # If we have extracted a Method, Uri, and/or ReplyTo, Body, send that out as a fims message
                if methd and uri:
                    if methd == "pub" or methd == "set" or methd == "get":
                        print("\n\nFims message contains the following:\n" \
                                "Method:     {0}\n" \
                                "Uri:        {1}\n" \
                                "ReplyTo:    {2}\n" \
                                "Body:       {3}\n" \
                                "Time:       {4}\n" \
                        .format(methd, uri, reply_to if reply_to else "(null)", body if body else "(null)",times if times else "(null)"))
                        #text = "y"
                        #time.sleep(0.1) 
                        text = input_to("Send out fims message? (y/n)",1)
                        if text.lower() == "y" or text.lower() == "yes":
                            send_fims_msg(methd, uri, reply_to, body)
                methd, uri, reply_to, body = "", "", "", ""

    f_r.close()

# Sends out the fims message retrieved from the dump
def send_fims_msg(methd, uri, reply_to, body):
    if not body and not reply_to:
        subprocess.call(["/usr/local/bin/fims/fims_send", "-m", methd, "-u", uri])
    elif body and not reply_to:
        subprocess.call(["/usr/local/bin/fims/fims_send", "-m", methd, "-u", uri, body])
    elif not body and reply_to:
        subprocess.call(["/usr/local/bin/fims/fims_send", "-m", methd, "-u", uri, "-r", reply_to])
    else:
        subprocess.call(["/usr/local/bin/fims/fims_send", "-m", methd, "-u", uri, "-r", reply_to, body])

def main():
    if len(sys.argv) < 2:
        usage()
        sys.exit()
    parse_fims_dump(sys.argv[1])

if __name__ == "__main__":
    main()