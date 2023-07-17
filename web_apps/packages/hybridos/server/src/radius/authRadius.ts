import { Injectable } from '@nestjs/common'
import * as dgram from 'dgram'
import * as events from 'events'
import * as radius from 'radius'
import * as path from 'path'

@Injectable()
export class AuthRadius {
    nasIpAddress: string
    secret: string
    port: number
    protocol: string
    em: events.EventEmitter
    connectionTimeout: number
    Events: {
        accepted: string
        rejected: string
        errored: string
        timedout: string
    }
    socket: dgram.Socket
    timeoutId: NodeJS.Timeout
    constructor(nasIpAddress, secret, port, connectionTimeout) {
        if (!nasIpAddress) throw new Error('nasIpAddress is not defined.')

        if (!secret) throw new Error('secret is not defined.')

        if (!port) throw new Error('port is not defined.')

        if (connectionTimeout > 30000)
            // 30 seconds seems like absurdly long time to wait so don't allow it. could make it configurable
            throw new Error('connectionTimeout is higher than expected.')

        this.nasIpAddress = nasIpAddress
        this.secret = secret
        this.port = port
        this.em = new events.EventEmitter({ captureRejections: true })
        this.connectionTimeout = connectionTimeout ? connectionTimeout : 5000
        this.Events = {
            accepted: 'accepted',
            rejected: 'rejected',
            errored: 'errored',
            timedout: 'timedout',
        }

        this.socket
        this.timeoutId
    }
    authenticate(username, password) {
        radius.add_dictionary(path.join(__dirname, 'dictionaries/dictionary.flexgen'))
        const packet = radius.encode({
            code: 'Access-Request',
            secret: this.secret,
            attributes: [
                ['NAS-IP-Address', this.nasIpAddress],
                ['User-Name', username],
                ['User-Password', password],
            ],
        })

        this.socket = dgram.createSocket({ type: 'udp4' })
        this.timeoutId = setTimeout(() => {
            this.close(this.Events.timedout, null) // TODO: what should we do if Radius times out? Try local?
        }, this.connectionTimeout)

        this.socket.on('message', (msg) => {
            const decodedPacket = radius.decode({
                packet: msg,
                secret: this.secret,
            })
            if (decodedPacket.code === 'Access-Accept') {
                this.close(this.Events.accepted, decodedPacket)
            } else if (decodedPacket.code === 'Access-Reject') {
                this.close(this.Events.rejected, decodedPacket)
            } else {
                // Condition unlikely to be hit.
                this.error('connection status undertermined')
            }
        })

        this.socket.send(
            packet,
            0,
            packet.length,
            this.port,
            this.nasIpAddress,
            (err) => {
                if (err) {
                    this.error(err.message)
                }
            }
        )

        return this
    }
    onError(callback) {
        this.addEvent(this.Events.errored, callback)
        return this
    }
    onAccept(callback) {
        this.addEvent(this.Events.accepted, callback)
        return this
    }
    onReject(callback) {
        this.addEvent(this.Events.rejected, callback)
        return this
    }
    onTimeout(callback) {
        this.addEvent(this.Events.timedout, callback)
        return this
    }
    addEvent(name, callback) {
        this.em.on(name, function (data) {
            setImmediate(() => {
                callback(data)
            })
        })
    }
    // close socket clear timeout and emit event
    close(event, obj) {
        clearTimeout(this.timeoutId)
        this.socket.close()
        this.em.emit(event, obj)
    }
    // close socket and emit error event
    error(message) {
        this.close(this.Events.errored, new Error(message))
    }
    getAttributeRole(decodedPacket) {
        if (decodedPacket && decodedPacket.attributes)
            return decodedPacket.attributes['Vendor-Specific']['FlexGen-Role']
        return null
    }
}
