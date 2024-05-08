/*
 * SocketNamespaceInterceptor is one side of the namespacing (routing) system we
 * use for sockets. The other side is managed in socket.service on the frontend.
 *
 * On request capture:
 * The request is identified by socket (socketID), the gateway
 * name (className), and the request name (handlerName). The namespace for the
 * request is stored in the corresponding location in NamespaceMap.
 *
 * On response capture:
 * Transform the response to
 * {
 *   namespace: [the namespace from the map]
 *   data: [the original response]
 * }
 *
 * On the frontend, namespace is stripped and only the original response is passed
 * to the querying service, meaning no other services on the frontend or backend
 * need to be aware of the namespacing process aside from declaring a namespace
 */

import { Injectable, NestInterceptor, ExecutionContext, CallHandler } from '@nestjs/common';
import { Observable } from 'rxjs';
import { map, tap } from 'rxjs/operators';

type NamespaceMap = {
  [socketID: string]: {
    [className: string]: {
      [handlerName: string]: string;
    };
  };
};

const namespaceMap: NamespaceMap = {};

@Injectable()
export class SocketNamespaceInterceptor implements NestInterceptor {
  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    // TODO: what type is this??? .getClient<T>() would be good
    const socket = context.switchToWs().getClient();
    const message = context.switchToWs().getData();

    const remoteAddress = socket._socket.remoteAddress;
    const remotePort = socket._socket.remotePort;
    const socketID = `${remoteAddress}:${remotePort}`;
    if (!(socketID in namespaceMap)) {
      namespaceMap[socketID] = {};
      socket.on('close', () => {
        console.log(`socket ${socketID} closed, deleting from namespaceMap`);
        delete namespaceMap[socketID];
      });
    }

    const className = context.getClass().name;
    if (!(className in namespaceMap[socketID])) {
      namespaceMap[socketID][className] = {};
    }

    const handlerName = context.getHandler().name;

    if (handlerName in namespaceMap[socketID][className]) {
      console.warn(
        `handler ${handlerName} already exists in namespaceMap, this request will not be namespaced.Are you double subscribing ? `,
      );
    } else {
      const namespace = message.namespace;
      if (namespace === undefined) {
        console.warn(`no namespace found for request to ${handlerName}`);
      }

      namespaceMap[socketID][className][handlerName] = namespace;
    }

    return next.handle().pipe(
      map((data) => ({
        namespace: namespaceMap[socketID][className][handlerName],
        data,
      })),
    );
  }
}
