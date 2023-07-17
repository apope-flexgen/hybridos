/* eslint-disable @typescript-eslint/no-var-requires */
import { Injectable } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { filter, fromEvent, Observable } from 'rxjs';

import { UriIsRootOfUri } from '../utils/utils';
import FimsListener from './FimsListener';
import { FimsMsg, IFimsService, ListenFunc } from './interfaces/fims.interface';
import FIMS from '@flexgen/fims';
import { FimsErrorMessages } from './fims.constants';
import { FIMSSendFailedException, FIMSSendTimedOutException } from './exceptions/fims.exceptions';

@Injectable()
export class FimsService implements IFimsService {
  // holds all of the currently registered UIDs to prevent duplicate UIDs
  private UIDs: Set<string> = new Set();
  eventEmitter: EventEmitter2 = new EventEmitter2({ maxListeners: 100 });
  fimsListener: FimsListener = new FimsListener(this.eventEmitter);

  // holds URIs each client is subscribed to to prevent duplicate subscriptions
  userSubscribedUris: Map<string, Map<string, Observable<FimsMsg>>> = new Map();

  constructor() {
    this.openListener();
  }

  get = async (uri: string, body: any = {}): Promise<FimsMsg> => {
    const res = await this.send({
      method: 'get',
      uri,
      replyto: '/web_server',
      body: body,
      username: 'web_server',
    });

    if (res.method === 'error') {
      if (res.body === FimsErrorMessages.SEND_FAILED) {
        throw new FIMSSendFailedException({ method: 'get', uri, body });
      }
      if (res.body === FimsErrorMessages.SEND_TIMED_OUT) {
        throw new FIMSSendTimedOutException({ method: 'get', uri, body });
      }
    }

    return res;
  };

  // public method to send a FIMS message and return a response
  send = async (msg: FimsMsg): Promise<FimsMsg> => {
    const uid = this.newUID();
    const prefix = msg.replyto.charAt(0) === '/' ? '/ui_reply' : '/ui_reply/';
    msg.replyto = `${prefix}${msg.replyto}/${uid}`;

    // msgResolve will hold a "resolve" function for msgPromise
    let msgResolve;
    const msgPromise = new Promise<FimsMsg>((resolve) => {
      msgResolve = resolve;
    });

    // the listening function will be created with msgResolve as the resolve
    // for the internal function that will be called when the message is received.
    // this means when msgResolve resolves, we will have the message
    const respond = this.getListeningFunc(msg.replyto, uid, msgResolve);

    // start the listener, send the message, await the response, and return it
    this.eventEmitter.addListener('fims.reply', respond);

    const ok = FIMS.send(msg);
    const res = ok
      ? await msgPromise
      : {
          method: 'error',
          uri: msg.uri,
          replyto: msg.replyto,
          body: FimsErrorMessages.SEND_FAILED,
          username: msg.username,
        };
    return res;
  };

  // public method to subscribe to a URI
  subscribe = (uri: string, req?: any): Observable<FimsMsg> => {
    // if attempting to subscribe directly, don't allow resubscribing
    // if we are already connected. prevents duplicate messages.
    const uid = req ? req._socket.remoteAddress + req._socket.remotePort : 'web_server';

    if (req && this.isUserAlreadySubscribed(uid, uri)) {
      return;
    }

    const observable = this.createSubscribeObservable(uri, req) as Observable<FimsMsg>;

    if (req) {
      this.registerSubscription(uri, uid, observable);
    }
    // return an observable that will emit messages from the user's URI
    return observable;
  };

  isUserAlreadySubscribed = (uid: string, uri: string): boolean => {
    if (this.userSubscribedUris.has(uid) && this.userSubscribedUris.get(uid).has(uri)) {
      return true;
    }

    return false;
  };

  registerSubscription = (uri: string, uid: string, obs: Observable<FimsMsg>) => {
    // if the user's URI set doesn't exist, create it
    if (!this.userSubscribedUris.has(uid)) {
      this.userSubscribedUris.set(uid, new Map());
    }

    // add the URI to the user's URI set
    this.userSubscribedUris.get(uid).set(uri, obs);

    return;
  };
  // private init method to open a listener for FIMS messages
  private openListener = (): boolean => {
    return this.fimsListener.open();
  };
  // private method to create a new UID and add to set and return if it
  // isn't already in use
  private newUID = (): string => {
    const newID = Math.random().toString(36).substr(2, 9);
    if (this.UIDs.has(newID)) {
      return this.newUID();
    }
    this.UIDs.add(newID);
    return newID;
  };
  // private method to create a special function that will be used as
  // a callback for an event listener when listening for only one message.
  // this function will resolve with the message when it is received
  // and remove the listener immediately
  private getListeningFunc = (uri, uid, resolve): ListenFunc => {
    let hasResolved = false;
    setTimeout(() => {
      if (hasResolved) return;
      this.eventEmitter.removeListener('fims.message', respond);
      this.UIDs.delete(uid);
      resolve({
        method: 'error',
        uri: uri,
        replyto: 'web_server',
        body: FimsErrorMessages.SEND_TIMED_OUT,
        username: 'error',
      });
    }, 5000);
    const respond = (e) => {
      if (!hasResolved && e && e.uri && UriIsRootOfUri(e.uri, uri)) {
        this.eventEmitter.removeListener('fims.message', respond);
        this.UIDs.delete(uid);
        resolve(e);
      }
    };

    return respond;
  };
  // private method to create the observable for a subscription.
  // observable will emit messages from the user's URI.
  // on socket close, this event listener will be removed
  private createSubscribeObservable = (
    uri: string,
    req?: any,
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
          return true;
        }
        return false;
      }),
    );

    if (!req) {
      return newObs;
    }

    req.on('close', () => {
      this.doUnsubscribeCleanup(req._socket.remoteAddress + ':' + req._socket.remotePort, uri);
    });
    req.on('error', () => {
      this.doUnsubscribeCleanup(req._socket.remoteAddress + ':' + req._socket.remotePort, uri);
    });

    return newObs;
  };

  private doUnsubscribeCleanup(uid: string, uri: string) {
    // remove the URI from the user's URI set
    if (this.userSubscribedUris.has(uid)) {
      if (this.userSubscribedUris.get(uid).has(uri)) {
        this.userSubscribedUris.get(uid).delete(uri);
      }
      // if the user's URI set is empty, remove the user
      if (this.userSubscribedUris.get(uid).size == 0) {
        this.userSubscribedUris.delete(uid);
      }
    }
  }
}
