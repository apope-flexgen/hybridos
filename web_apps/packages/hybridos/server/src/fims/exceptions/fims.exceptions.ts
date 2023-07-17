import { FimsMsg } from '../interfaces/fims.interface';

export class FimsException extends Error {
  constructor(msg: string) {
    super(msg);
  }
}

export class FIMSSendFailedException extends FimsException {
  constructor(msg: Pick<FimsMsg, 'method' | 'uri' | 'body'>) {
    super(
      `FIMS Send failed - method: ${msg.method} - uri: ${msg.uri} - body: ${JSON.stringify(
        msg.body,
      )}`,
    );
  }
}

export class FIMSSendTimedOutException extends FimsException {
  constructor(msg: Pick<FimsMsg, 'method' | 'uri' | 'body'>) {
    super(
      `FIMS Send timed out - no response received - method: ${msg.method} - uri: ${
        msg.uri
      } - body: ${JSON.stringify(msg.body)}`,
    );
  }
}
