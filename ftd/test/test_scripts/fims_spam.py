import subprocess
import time
import random
import string

NUM_COLUMNS = 20
MAX_STRING_LEN = 16

def build_body(body_type):
    item_dict = {}
    if body_type == 'int':
        item_dict = {"int_%d" % (i+1) : random.randint(1, 100) for i in range(NUM_COLUMNS)}
    elif body_type == 'double':
        item_dict = {"double_%d" % (i+1) : random.uniform(0.0, 100.0) for i in range(NUM_COLUMNS)}
    elif body_type == 'string':
        item_dict = {"string_%d" % (i+1) : ''.join(random.choices(string.ascii_letters + string.digits, k=random.randint(1, MAX_STRING_LEN))) for i in range(NUM_COLUMNS)}
        
    # build JSON compatible FIMS body
    str = '{ '
    for key, value in item_dict.items():
        if body_type == 'string':
            value = '"%s"' % value
        str += '"%s" : %s, ' % (key, value)
    str = str[:-2] # remove last ", "
    str += ' }'

    return str

messages = {
    '/test/int' : [ 'int', 'pub' ],
    '/test/double' : [ 'double', 'pub' ],
    '/test/string' : [ 'string', 'pub' ],
    '/groups/s1' : [ 'int', 'pub' ],
    '/groups/s2' : [ 'double', 'pub' ],
    '/groups/s3' : [ 'string', 'pub' ],
    '/methods/set' : [ 'int', 'set' ],
    '/methods/pub' : [ 'int', 'pub' ],
}

while True:
    for uri, metadata in messages.items():
        subprocess.run(["fims_send", "-m", metadata[1], "-u", uri, build_body(metadata[0])])

    time.sleep(0.005)