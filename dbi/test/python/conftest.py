import pytest
import subprocess
import pymongo
import json
import random
import string
import os
import shutil

###
# SESSION STARTUP

# commandline args
def pytest_addoption(parser):
    parser.addoption("--colls", action="store", default=1, help="number of collections")
    parser.addoption("--docs", action="store", default=1, help="number of documents per collection")
    parser.addoption("--depth", action="store", default=2, help="maximum depth of nested maps per document")

# these run to verify the testing infrastructure (dbi, mongo, user-defined content) works before starting the tests
def pytest_sessionstart(session):
    pid, err = subprocess.Popen(["pidof", "dbi"], stdout=subprocess.PIPE).communicate()
    if err != None:
        pytest.exit(err.decode())
    if len(pid) == 0:
        pytest.exit("dbi is not running")

    mongo = Mongo()
    mongo.client.drop_database('dbi') # clear out database
    mongo.client.close()

    print("startup check successful")


def pytest_sessionfinish(session, exitstatus):
    tmp_path = '%s/args/tmp' % os.getcwd()
    if os.path.exists(tmp_path):
        shutil.rmtree(tmp_path)
    print("\ntesting complete! executed %d queries to DBI." % pytest.send_count)

# globalized fields that can be accessed by all tests
def pytest_configure(config):
    # document generation parameters
    pytest.numcollections = int(config.getoption("colls"))
    pytest.numdocuments = int(config.getoption("docs"))
    pytest.numlayers = int(config.getoption("depth"))

    pytest.send_count = 0

@pytest.fixture(scope="class", autouse=True)
def class_setup_teardown(request):
    # SETUP
    # do stuff?
    yield
    # TEARDOWN
    print("%s complete." % request.node.name)

@pytest.fixture
def save_restore_state(request):
    uri = request.getfixturevalue("uri")
    frags = uri.split("/")[2:]

    # get previous state
    state_path = '%s/args/tmp/%s_prev' % (os.getcwd(), frags[0])
    if os.path.exists(state_path) and os.path.isfile(state_path): # check if already exists
        file = open(state_path)
        state =  json.load(file)
        file.close()
    else: # else save it
        state = send("get", "/dbi/%s/show_map" % frags[0])
        state_path = tmp_json(state, '%s_prev' % frags[0])

    yield state # handoff

    # reset to previous state
    send("set", '/dbi/%s' % frags[0], state_path, True)

###
# HELPER FUNCTIONS/CLASSES

class Command:
    def __init__(self, method, uri, body="", file=False):
        self.method = method
        self.uri = uri
        self.body = body
        self.file = file

    # returns fims command as an array so that subprocess understands it
    def parse(self):
        # format:
        # fims_send -m <method> -u <uri> <body> -r /me
        args = ["fims_send", "-r", "/me"]
        if self.method == "get" or self.method == "del":
            return args + ["-m", self.method, "-u", self.uri]
        elif self.file:
            return args + ["-m", self.method, "-u", self.uri, "-f", self.body]
        return args + ["-m", self.method, "-u", self.uri, self.body ]
        
    def execute(self):
        process = subprocess.Popen(self.parse(), stdout=subprocess.PIPE)
        out, err = process.communicate() # execute
        if err != None:
            err = err.decode()
        return out, err

# wrapper for sending a command
def send(method, uri, body="", file=False):
    cmd = Command(method, uri, body, file)
    response, err = cmd.execute()
    assert err is None
    assert len(response) != 0
    try:
        response = json.loads('{\n\t"%s" : %s}' % ("value", response.decode('utf-8')))["value"] # converts to properly typed value
    except json.decoder.JSONDecodeError:
            raise Exception('failed to decode %s -> %s\n%s' % (body, uri, response))
    pytest.send_count+=1
    return response


class Mongo():
    def __init__(self):
        self.client = pymongo.MongoClient("mongodb://localhost:27017/")

    def grab(self, collection, document):
        cursor = self.client["dbi"][collection].find({"_id" : document})
        try:
            doc = cursor.next()
        except StopIteration:
            doc = False # error, doc DNE
        return doc

    # clears the dbi database and resyncs down to an active dbi process
    def add_documents(self, class_name, numcollections, numdocuments, numlayers):
        file = open('template.json')
        template = json.load(file)
        file.close()

        for c in range(numcollections):
            col_random_key = class_name + "%d" % c # colls sort by name and #
            for d in range(numdocuments):
                doc = recurse_add_layer(template.copy(), "map", numlayers)
                doc["_id"] = ''.join(random.choices(string.ascii_lowercase + string.ascii_uppercase, k=d+5)) # assign random name (_id)
                self.client["dbi"][col_random_key].insert_one(doc)

        cmd = Command("set", "/dbi/resync")
        return cmd.execute()

# helper function for creating docs
def recurse_add_layer(template, key, layers):
    if layers == 0:
        return template.copy()
    template[key] = recurse_add_layer(template.copy(), key, layers-1)
    return template
    
# dumps a dictionary into a json in ./args/tmp
def tmp_json(body, filename):
    tmp_path = "%s/args/tmp" % os.getcwd()
    if not os.path.exists(tmp_path):
        os.makedirs(tmp_path)

    complete_path = os.path.join(tmp_path, "%s.json" % filename)
    with open(complete_path, 'w', encoding="utf8") as dest:
        json.dump(body, dest)

    return complete_path