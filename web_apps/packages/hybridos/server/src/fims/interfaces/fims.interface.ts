import { EventEmitter2 } from '@nestjs/event-emitter';
import { Observable } from 'rxjs';

export interface FimsMsg {
  method: string;
  uri: string;
  replyto: string;
  body: any; // TODO: Fix this type. Was (string | Record<string, unknown>), but error'd out.
  username: string;
}

export interface ListenFunc {
  (...values: any[]): void;
}

export const FIMS_SERVICE = 'FimsService';
export interface IFimsService {
  get(uri: string, body?: any): Promise<FimsMsg>;

  // public method to send a FIMS message and return a response
  send(msg: FimsMsg): Promise<FimsMsg>;

  // public method to subscribe to a URI
  subscribe(uri: string, req?: WebSocket): Observable<FimsMsg>;

  isUserAlreadySubscribed(uid: string, uri: string): boolean;

  registerSubscription(uri: string, uid: string, obs: Observable<FimsMsg>);
}
