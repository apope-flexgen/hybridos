import json
import os
import socket
import sys
import struct

FIMS_CONN_CLOSED = -1
socketName = "/tmp/FlexGen_FIMS_Server.socket"
maxMessageSize = 917518
debug = False


class pyfims:
    connection = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET, 0)
    name = ''
    xt = ("method", "uri", "replyto", "userName", "senderName", "length")
    debug = False
    order = "little"

    def __init__(self) -> None:
        pass

    def Connect(self, pname):
        self.name = pname
        # create socket
        if (self.connection) == FIMS_CONN_CLOSED:
            print("Failed to create a socket\n", file=sys.stderr)

        # connect to server
        try:

            r = self.connection.connect(socketName)
            if self.debug:
                print("Socket Connected")
            if r != None:
                if self.debug:
                    print(r)

            m = self.connection.recvmsg(maxMessageSize)

            vers, msize = struct.unpack('@hi', m[0])
            if self.debug:
                dmesg = '"vers: %d  size: %ld\n"' % (vers, msize)
                print(dmesg)

            ba = bytearray()
            dummy = 37356
            ba += bytearray(vers.to_bytes(2, self.order))
            ba += bytearray(dummy.to_bytes(2, self.order))
            ba += bytearray(msize.to_bytes(4, self.order))
            r = self.connection.send(ba)
            if self.debug:
                print("sent ack")
            suri = '%s' % (self.name)
            ba = suri.encode()
            r = self.connection.send(ba)

            #m = self.connection.recvmsg(maxMessageSize)
            if self.debug:
                print(" After send name")
                print(r)

            method = "sub"
            uri = ""
            pubOnly = 0
            numSubs = 1
            suri = '/%s' % (self.name)
            surilen = len(suri)
            ba = self.GetHeader(method, uri, "", self.name, "", surilen+3)

            ba += bytearray(numSubs.to_bytes(1, self.order))
            ba += bytearray(pubOnly.to_bytes(1, self.order))
            ba += bytearray(surilen.to_bytes(1, self.order))

            if self.debug:
                print(" sending sub :")
            ba += suri.encode()
            if self.debug:
                print(ba)
            r = self.connection.send(ba)
            if self.debug:
                print(" Connect OK")
            m = self.connection.recvmsg(maxMessageSize)
            if self.debug:
                print(" After send sub")
                print(m)

            return [r, 0, "Connect OK"]

        except socket.error as msg:
            # print(" Connect failed #1")
            # print(msg) #test with docker
            return [None, -1, msg]
            # print >>sys.stderr, msg #test with docker
        except:
            if self.debug:
                print(" Connect failed #2", file=sys.stderr)
            return [None, -1, "Connect Failed"]

    def GetHeader(self, method, uri, replyto, pname, uname, body_len):
        ba = bytearray()
        method_len = len(method)
        uri_len = len(uri)
        replyto_len = 0
        if replyto != None:
            replyto_len = len(replyto)
        pname_len = len(pname)
        uname_len = len(uname)

        if self.debug:
            total_bytes = method_len + uri_len + replyto_len + pname_len + uname_len + body_len
            dmesg = '"method_len": %d, "total_bytes": %d \n"' % (method_len, total_bytes)
            print(dmesg)

        dummy = 1
        ba = bytearray(method_len.to_bytes(1, self.order))
        ba += bytearray(uri_len.to_bytes(1, self.order))
        ba += bytearray(replyto_len.to_bytes(1, self.order))
        ba += bytearray(pname_len.to_bytes(1, self.order))
        ba += bytearray(uname_len.to_bytes(1, self.order))
        ba += bytearray(dummy.to_bytes(3, self.order))
        ba += bytearray(body_len.to_bytes(4, self.order))

        ba += bytearray(method.encode())
        if uri_len > 0:
            ba += bytearray(uri.encode())
        if replyto_len > 0:
            ba += bytearray(replyto.encode())
        if pname_len > 0:
            ba += bytearray(pname.encode())
        if uname_len > 0:
            ba += bytearray(uname.encode())

        if self.debug:
            print(ba)

        return ba

    def Send(self, method, uri, replyto=None, body=None, timeout=-1):
        if (self.connection) == FIMS_CONN_CLOSED:
            print("Failed to send, socket closed\n", file=sys.stderr)

        #replyto = None
        message = None
        if method is 'get':
            # create the message and send
            if self.debug:
                print("get")
            ba = self.GetHeader(method, uri, replyto, self.name, "", 0)
            if self.debug:
                print(" sending get :")
                print(message)
                print(ba)
            r = self.connection.send(ba)
            # return [r, 0, "Success"]

            # receive success message
            if replyto != None:
                ret = self.Receive(timeout)
                if self.debug:
                    print("got reply \n")
                    print(ret)
                return [r, 0, ret]
            return [r, 0, "Success"]

        elif method is 'set':
            if self.debug:
                print("set")
            msg = body
            try:
                foo = len(body)
            except:
                msg = f"{body}"
            ba = self.GetHeader(method, uri, replyto, self.name, "", len(msg))
            ba += msg.encode()
            r = self.connection.send(ba)
            return [r, 0, "Success"]

    # Send a get and return only err code and json encoded body
    def SendGet(self, uri, replyto):
        response = self.Send("get", uri, replyto, body=None, timeout=5)
        return [response[0], json.loads(response[2][0])["body"]]

    # Send a set and return only the err code
    def SendSet(self, uri, body):
        response = self.Send("set", uri, None, json.dumps(body))
        return response[0]

    # TODO: post and del when supported

    def Subscribe(self, uri):
        if self.debug:
            print("Calling Subscribe uri:")
            print(uri)
            print("\n")

        method = "sub"
        pubOnly = 0
        numSubs = 0
        surilen = 0
        suri = ""
        sa = []
        if isinstance(uri,  list):
            for s in uri:
                sa.append(s)
                surilen += len(s)
                numSubs += 1
                suri += s
        else:
            sa.append(uri)
            surilen = len(uri)
            numSubs += 1
            suri = uri
        if self.debug:
            print(surilen)
            print(sa)
            print("\n")
        ba = self.GetHeader(method, "", "", self.name, "", surilen+(numSubs * 2) + 1)

        ba += bytearray(numSubs.to_bytes(1, self.order))

        # repeat this for each sub
        for s in sa:
            ba += bytearray(pubOnly.to_bytes(1, self.order))
            slen = len(s)
            ba += bytearray(slen.to_bytes(1, self.order))
            #ba += s.encode()
        ba += suri.encode()

        if self.debug:
            print(" sending sub :")
            print(ba)
        r = self.connection.send(ba)

        r = self.Receive()
        if self.debug:
            print(r)
        return r

    def Receive(self, timeout=-1):
        if self.debug:
            print("Calling Receive")
        if timeout >= 0:
            self.connection.settimeout(timeout)
        else:
            self.connection.settimeout(None)
        try:
            x = self.connection.recvmsg(maxMessageSize)
        except socket.timeout:
            if self.debug:
                print("Socket Timeout", file=sys.stderr)
            return [None, 0, "Socket Timeout"]
        except:
            if self.debug:
                print("Socket Error", file=sys.stderr)
            return [None, -1, "Socket Error"]

        xt = self.xt
        if self.debug:
            print(x)
        self.connection.settimeout(None)
        xx = struct.unpack('@BBBBBxxxi', x[0][:12])
        xti = 0
        ix = 12
        iy = 0
        comma = ''
        xm = '{'
        for xxx in xx:
            if xxx > 0 and xti < 5:
                iy = ix + xxx
                if comma != '':
                    xm += '%s "%s":"%s"' % (comma, xt[xti], x[0][ix:iy].decode())
                else:
                    xm += '"%s":"%s"' % (xt[xti], x[0][ix:iy].decode())

                comma = ','
                ix = iy
            xti += 1
        if xx[5] > 0:
            xm += ', "body": '
            #xm += "\"" + x[0][iy:].decode() + "\""
            xm += x[0][iy:].decode()

        xm += '}'
        return [xm, 0, "Success"]

    def Close(self):
        self.connection.close()
        if self.debug:
            print("Socket Closed")
        return [None, 0, "Success"]

# p = pyfims()
# p.Connect('pyfims_test')

# val = p.Send("get", "/components/bms_9/cap")
# print(val)
# val = p.Send("set", "/components/bms_9/cap", None, "99")

# val = p.Send("get", "/components/bms_9/cap")
# print(val)

# p.Close()


# python3
# >>> import pyfims_rewrite as py
# >>> mp = py.pyfims()
# >>> mp.Connect("mypi")
#[27, 0, 'Connect OK']
# >>> mp.Subscribe("/myname")
#['{"userName":"fims_server", "body": SUCCESS}', 0, 'Success']
# >>> mp.Send("get","/some/uri", None,"/myname")
#[35, 0, ['{"method":"set", "uri":"/myname", "userName":"fims_send", "body": hello}', 0, 'Success']]
