/* eslint-disable @typescript-eslint/no-var-requires */
import { Injectable } from '@nestjs/common'
import { EventEmitter2 } from '@nestjs/event-emitter'
import { Request } from 'express'
import { filter, fromEvent, Observable } from 'rxjs'
import { WebSocket } from 'ws'

import { UriIsRootOfUri } from '../utils/utils'
import FimsListener from './FimsListener'
import { FimsMsg, IFimsService, ListenFunc } from './interfaces/fims.interface'
import FIMS from '@flexgen/fims'

@Injectable()
export class FimsService implements IFimsService {
    // holds all of the currently registered UIDs to prevent duplicate UIDs
    private UIDs: Set<string> = new Set()
    eventEmitter: EventEmitter2 = new EventEmitter2()
    fimsListener: FimsListener = new FimsListener(this.eventEmitter)
    // holds URIs each client is subscribed to to prevent duplicate subscriptions
    userSubscribedUris: Map<string, Map<string, Observable<FimsMsg>>> = new Map()
    constructor() {
        this.openListener()
    }
    get = async (uri: string, body: any = {}): Promise<FimsMsg> => {
        return await this.send({
            method: 'get',
            uri,
            replyto: 'nest_web_server',
            body: body,
            username: 'nest_web_server',
        })
    }
    // public method to send a FIMS message and return a response
    send = async (msg: FimsMsg): Promise<FimsMsg> => {
        const uid = this.newUID()
        msg.replyto = msg.replyto + '/' + uid

        // msgResolve will hold a "resolve" function for msgPromise
        let msgResolve
        const msgPromise = new Promise<FimsMsg>((resolve) => {
            msgResolve = resolve
        })

        // the listening function will be created with msgResolve as the resolve
        // for the internal function that will be called when the message is received.
        // this means when msgResolve resolves, we will have the message
        const respond = this.getListeningFunc(msg.replyto, uid, msgResolve)

        // start the listener, send the message, await the response, and return it
        this.eventEmitter.addListener('fims.message', respond)

        const ok = FIMS.send(msg)
        const res = ok
            ? await msgPromise
            : {
                  method: 'error',
                  uri: msg.uri,
                  replyto: msg.replyto,
                  body: 'fims.send failed',
                  username: msg.username,
              }
        return res
    }
    // public method to subscribe to a URI
    subscribe = (uri: string, req?: any): Observable<FimsMsg> => {
        // if attempting to subscribe directly, don't allow resubscribing
        // if we are already connected. prevents duplicate messages.
        const uid = req ? req._socket.remoteAddress + req._socket.remotePort : 'nest_web_server'

        if (req && this.isUserAlreadySubscribed(uid, uri)) {
            return
        }

        const observable = this.createSubscribeObservable(uri, req) as Observable<FimsMsg>

        if (req) {
            this.registerSubscription(uri, uid, observable)
        }
        // return an observable that will emit messages from the user's URI
        return observable
    }
    isUserAlreadySubscribed = (uid: string, uri: string): boolean => {
        if (this.userSubscribedUris.has(uid) && this.userSubscribedUris.get(uid).has(uri)) {
            return true
        }

        return false
    }
    registerSubscription = (uri: string, uid: string, obs: Observable<FimsMsg>) => {
        // if the user's URI set doesn't exist, create it
        if (!this.userSubscribedUris.has(uid)) {
            this.userSubscribedUris.set(uid, new Map())
        }

        // add the URI to the user's URI set
        this.userSubscribedUris.get(uid).set(uri, obs)

        return
    }
    // private init method to open a listener for FIMS messages
    private openListener = (): boolean => {
        return this.fimsListener.open()
    }
    // private method to create a new UID and add to set and return if it
    // isn't already in use
    private newUID = (): string => {
        const newID = Math.random().toString(36).substr(2, 9)
        if (this.UIDs.has(newID)) {
            return this.newUID()
        }
        this.UIDs.add(newID)
        return newID
    }
    // private method to create a special function that will be used as
    // a callback for an event listener when listening for only one message.
    // this function will resolve with the message when it is received
    // and remove the listener immediately
    private getListeningFunc = (uri, uid, resolve): ListenFunc => {
        const respond = (e) => {
            if (e && e.uri && UriIsRootOfUri(e.uri, uri)) {
                this.eventEmitter.removeListener('fims.message', respond)
                this.UIDs.delete(uid)
                resolve(e)
            }
        }

        return respond
    }
    // private method to create the observable for a subscription.
    // observable will emit messages from the user's URI.
    // on socket close, this event listener will be removed
    private createSubscribeObservable = (
        uri: string,
        req?: any
        // FIXME: unknown
    ): Observable<unknown> => {
        const newObs = fromEvent(this.eventEmitter, 'fims.message').pipe(
            filter((e) => {
                if (
                    e &&
                    e.hasOwnProperty('method') &&
                    e['method'] === 'pub' &&
                    e.hasOwnProperty('uri') &&
                    UriIsRootOfUri(e['uri'], uri)
                ) {
                    return true
                }
                return false
            })
        )

        if (!req) {
            return newObs
        }

        req.on('close', () => {
            this.doUnsubscribeCleanup(req._socket.remoteAddress + ':' + req._socket.remotePort, uri)
        })
        req.on('error', () => {
            this.doUnsubscribeCleanup(req._socket.remoteAddress + ':' + req._socket.remotePort, uri)
        })

        return newObs
    }
    private doUnsubscribeCleanup(uid: string, uri: string) {
        // remove the URI from the user's URI set
        if (this.userSubscribedUris.has(uid)) {
            if (this.userSubscribedUris.get(uid).has(uri)) {
                this.userSubscribedUris.get(uid).delete(uri)
            }
            // if the user's URI set is empty, remove the user
            if (this.userSubscribedUris.get(uid).size == 0) {
                this.userSubscribedUris.delete(uid)
            }
        }
    }
}
