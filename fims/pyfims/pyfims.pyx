from cfims cimport fims, fims_message
from libc.stdlib cimport malloc, free
from cpython.mem cimport PyMem_Malloc, PyMem_Realloc, PyMem_Free
import json

cdef const char** to_cstring_array(list_str):
    cdef const char** ret = <const char**>PyMem_Malloc(len(list_str) * sizeof(char*))
    if ret is NULL:
        raise MemoryError()
    for i in xrange(len(list_str)):
        byte_string = list_str[i].encode()
        ret[i] = byte_string # This has to be two steps so Python will keep the pointer
    return ret

cdef dict process_fims_message(fims_message* msg):
    m = {
        "method": msg.method.decode(),
        "uri": msg.uri.decode()
    }
    if msg.replyto is not NULL:
        m["replyto"] = msg.replyto.decode()
    if msg.body is not NULL:
        m["body"] = json.loads(msg.body)
    return m

cdef class pyfims:
    cdef fims c_fims # Hold a C++ instance which we're wrapping

    def __cinit__(self):
        self.c_fims = fims.fims()

    def Connect(self, str pname):
        return self.c_fims.Connect(pname.encode())

    def Subscribe(self, uri_list):
        cdef const char** c_uri_list = to_cstring_array(uri_list)
        success = self.c_fims.Subscribe(c_uri_list, len(uri_list), NULL)
        PyMem_Free(c_uri_list)
        return success
    
    def Send(self, method, uri, replyto=None, body=None, username=None):
        m = method.encode()
        u = uri.encode()
        cdef char* r = NULL
        cdef char* b = NULL
        cdef char* n = NULL
        if replyto is not None:
            replyto = replyto.encode()
            r = replyto
        if body is not None:
            body = json.dumps(body)
            body = body.encode()
            b = body
        if username is not None:
            username = username.encode()
            n = username
        return self.c_fims.Send(m, u, r, b, n)

    def Receive(self):
        cdef fims_message* msg
        msg = self.c_fims.Receive()
        if msg is NULL:
            return None
        m = process_fims_message(msg)
        return m

    def ReceiveTimeout(self, useconds):
        cdef fims_message* msg
        msg = self.c_fims.Receive_Timeout(useconds)
        if msg is NULL:
            return None
        m = process_fims_message(msg)
        return m

    def Connected(self):
        return self.c_fims.Connected()

    def Close(self):
        self.c_fims.Close()