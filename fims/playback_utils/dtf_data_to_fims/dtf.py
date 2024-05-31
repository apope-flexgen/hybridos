#!/bin/python3
import pandas as pd
import time
from threading import Timer

class csvreader(object):
    def __init__(self, fn, timestampField, fields):
        self.df = pd.read_csv(fn,parse_dates=[timestampField],index_col=[timestampField],usecols=[timestampField] + fields)
        self.duration = (self.df.index[-1] - self.df.index[0]).total_seconds()
        self.t0 = self.df.index[0]
        self.lastt = self.t0
        self.terminated = False
        self.values = self.termvalue = 0

    def get(self, t):
        if self.terminated:
            return self.termvalue
        if t < 0:
            raise ValueError("Time requested occurs before start of data (negative seconds input)")
        if t > self.duration:
            raise ValueError("Time requested occurs after end of data, check csvreader.duration")
        t1 = self.t0 + pd.Timedelta(seconds=t)
        try:
            ind = self.df.index.get_loc(t1,method='pad')
        except:
            return False
        self.values = self.df.iloc[ind]
        return self.values

class RepeatedTimer(object):
    # credit to https://stackoverflow.com/a/13151104
    def __init__(self, interval, function, *args, **kwargs):
        self._timer     = None
        self.function   = function
        self.interval   = interval
        self.args       = args
        self.kwargs     = kwargs
        self.is_running = False
        self.start()

    def _run(self):
        self.is_running = False
        self.start()
        self.function(*self.args, **self.kwargs)

    def start(self):
        if not self.is_running:
            self._timer = Timer(self.interval, self._run)
            self._timer.start()
            self.is_running = True

    def stop(self):
        self._timer.cancel()
        self.is_running = False

def loadConfig(filename):
    with open(filename) as json_file:
        config = json.load(json_file)
    fields = []
    for x in config["sets"]:
        if x["src"] not in fields:
            fields.append(x["src"])
    for x in config["pubs"]:
        for y in x["fields"]:
            if y["src"] not in fields:
                fields.append(y["src"])
    config["fields"] = fields
    return config

def getDt(t0,duration,multiplier):
    t1 = time.monotonic()
    return ((t1-t0)*multiplier) % duration

def sendPub(fims,csvdata, t0, config):
    dt = getDt(t0,csvdata.duration,config["timeMultiplier"])
    row = csvdata.get(dt)
    for pub in config["pubs"]:
        body = {}
        if ("naked" in pub) and pub["naked"]:
            body={field["dest"]:row[field["src"]] for field in pub["fields"]}
        else:
            body={field["dest"]:{"value":row[field["src"]]} for field in pub["fields"]}
        fims.Send("pub",pub["uri"],body=body)

def sendSet(fims,csvdata, t0, config):
    dt = getDt(t0,csvdata.duration,config["timeMultiplier"])
    row = csvdata.get(dt)
    for st in config["sets"]:
        body = {}
        if ("naked" in st) and st["naked"]:
            body = row[st["src"]]
        else:
            body = {"value":row[st["src"]]}
        fims.Send("set",st["dest"],body=body)

def processReceive(fims, timeout, subscribes, csvdata, t0, config):
    msg = fims.ReceiveTimeout(timeout)
    if msg is None:
        return
    elif msg["uri"] == "/dtf/config":
        if msg["method"] == "get" and "replyto" in msg:
            fims.Send("set",msg["replyto"],body=config)
    elif msg["method"] == "get" and "replyto" in msg:
        dt = getDt(t0,csvdata.duration,config["timeMultiplier"])
        row = csvdata.get(dt)
        for pub in config["pubs"]:
            body = {}
            if msg["uri"] == pub["uri"]:
                if ("naked" in pub) and pub["naked"]:
                    body={field["dest"]:row[field["src"]] for field in pub["fields"]}
                else:
                    body={field["dest"]:{"value":row[field["src"]]} for field in pub["fields"]}
                fims.Send("set",msg["replyto"],body=body)
            elif msg["uri"].startswith(pub["uri"]):
                field = msg["uri"][len(pub["uri"])+1:]
                for f in pub["fields"]:
                    if field == f["src"]:
                        if ("naked" in pub) and pub["naked"]:
                            body={f["dest"]:row[f["src"]]}
                        else:
                            body={f["dest"]:{"value":row[f["src"]]}}
                    fims.Send("set",msg["replyto"],body=body)



if __name__=="__main__":
    import sys
    import json
    
    if len(sys.argv) < 3:
        raise ValueError("Not enough arguments. Usage: ./dtf.py path/to/csv path/to/json")
    config = loadConfig(sys.argv[2])
    csvdata = csvreader(sys.argv[1],config["timestamp"],config["fields"])

    import pyfims
    fims = pyfims.pyfims()
    fims.Connect("DTF {}".format(sys.argv[1]))
    subscribes = ['/dtf'] + [p["uri"] for p in config["pubs"]]
    fims.Subscribe(subscribes)

    t0 = time.monotonic()
    tpub = RepeatedTimer(config["pubRate"]/1000.0,sendPub,fims,csvdata,t0,config)
    tpub.start()
    tset = RepeatedTimer(config["setRate"]/1000.0,sendSet,fims,csvdata,t0,config)
    tset.start()
    trec = RepeatedTimer(0.01,processReceive,fims,50,subscribes,csvdata,t0,config)
    trec.start()

    # Continually sleep while the timers do their things
    continueLoop = True
    while (fims.Connected() and continueLoop):
        try:
            time.sleep(0.2)
        except KeyboardInterrupt:
            continueLoop = False

    tpub.stop()
    tset.stop()
    trec.stop()
    fims.Close()
