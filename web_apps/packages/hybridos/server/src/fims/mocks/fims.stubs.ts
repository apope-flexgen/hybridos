/* eslint-disable @typescript-eslint/no-var-requires */
const zmq = require('zeromq')

const SOCK_OPT = {
    reconnect_ivl: 5000,
}

let sub
let push

const SUB_ADDRESS = 'tcp://127.0.0.1:4000'
const SUB_RETRY = `Retrying connection to fims_relay (${SUB_ADDRESS}) in ${
    SOCK_OPT.reconnect_ivl / 1000
} seconds.`
const SUB_CONNECTED = `Connected to fims_relay (${SUB_ADDRESS}).`
const PUSH_ADDRESS = 'tcp://127.0.0.1:4001'
const PUSH_RETRY = `Retrying connection to fims_relay (${PUSH_ADDRESS}) in ${
    SOCK_OPT.reconnect_ivl / 1000
} seconds.`
const PUSH_CONNECTED = `Connected to fims_relay (${PUSH_ADDRESS}).`

module.exports = {
    connect: function (pname: string) {
        if (sub == null) {
            sub = zmq.socket('sub', SOCK_OPT).monitor()
        }

        if (push == null) {
            push = zmq.socket('push', SOCK_OPT).monitor()
        }

        sub.on('connect', () => {
            console.log(SUB_CONNECTED)
        })

        push.on('connect', () => {
            console.log(PUSH_CONNECTED)
        })

        sub.connect(SUB_ADDRESS)
        push.connect(PUSH_ADDRESS)
        return true
    },
    receiveWithTimeout: function (timeout: any, callback: (data: any) => void) {
        sub.on('message', (uri: object, data: object) => {
            callback(data)
        })
        return true
    },
    send: function (msg: any) {
        push.send(JSON.stringify(msg))
        return true
    },
    subscribeTo: function (uri: string) {
        if (uri && uri !== '/') {
            sub.subscribe(uri)
        } else {
            sub.subscribe('')
        }
        return true
    },
    unsubscribeFrom: function (uri: string) {
        if (uri && uri !== '/') {
            sub.unsubscribe(uri)
        } else {
            sub.unsubscribe('')
        }
        return true
    },
    closeConnection: function () {
        try {
            sub.close()
        } catch (e) {
            console.log(`close ${SUB_ADDRESS} error: ${e}.`)
        }

        try {
            push.close()
        } catch (e) {
            console.log(`close ${PUSH_ADDRESS} error: ${e}.`)
        }

        return true
    },
}
