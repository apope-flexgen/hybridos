import { Injectable } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { FimsMsg } from './interface/fimsmsg.interface';
import { UriIsRootOfUri } from './utils';
import { ListenFunc } from './interface/listenerfunc.interface';
import * as fims from '../fims/fimsInstance';
import { FimsMsgResponse } from './responses/fims.response';

@Injectable()
export class ApiService {
  private UIDs: Set<string> = new Set();
  eventEmitter: EventEmitter2 = new EventEmitter2();

  // public method to send a FIMS message and return a response
  send = async (msg: FimsMsg): Promise<FimsMsgResponse> => {
    const uid = this.newUID();
    msg.replyto = msg.replyto + '/' + uid;

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
    this.eventEmitter.addListener('fims.message', respond);

    const ok = fims.send(msg);
    const res = ok
      ? await msgPromise
      : {
          method: 'error',
          uri: msg.uri,
          replyto: msg.replyto,
          body: 'fims.send failed',
          username: msg.username,
        };
    return res;
  };

  // private method to create a new UID and add to set and return if it
  // isn't already in use
  private newUID = (): string => {
    const newID = Math.random().toString(36).substring(2, 9);
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
    const respond = (e) => {
      if (e && e.uri && UriIsRootOfUri(e.uri, uri)) {
        this.eventEmitter.removeListener('fims.message', respond);
        this.UIDs.delete(uid);
        resolve(e);
      }
    };

    return respond;
  };
}
