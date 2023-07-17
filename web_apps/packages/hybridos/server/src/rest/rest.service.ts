import { Inject, Injectable } from '@nestjs/common';
import { FIMS_SERVICE, FimsMsg, IFimsService } from '../fims/interfaces/fims.interface';
import { User } from 'src/users/dtos/user.dto';
import { RestNoReplyResponse } from './responses/restnoreply.response';
import { RestPutResponse } from './responses/restput.response';
import { ILegacyRestService, LEGACY_REST_SERVICE } from './legacy/interfaces/rest.legacy.interface';

const FIMS_POST_METHOD = 'post';
const FIMS_SET_METHOD = 'set';
const FIMS_DEL_METHOD = 'del';
const FIMS_NO_REPLY_TIMEOUT = 9000;
const noReplyMessage = (uri: string) => `Message sent to ${uri}. No reply from ${uri}.`;

@Injectable()
export class RestService {
  constructor(
    @Inject(FIMS_SERVICE) private readonly fimsService: IFimsService,
    @Inject(LEGACY_REST_SERVICE) private readonly legacyRestService: ILegacyRestService,
  ) {}

  async post(uri: string, body: any, user: User) {
    const msg: FimsMsg = {
      method: FIMS_POST_METHOD,
      uri: uri,
      replyto: `/rest${uri}`,
      body: body,
      username: user.username,
    };
    return Promise.race([
      this.fimsService.send(msg),
      new Promise((resolve) =>
        setTimeout(() => {
          resolve(new RestNoReplyResponse(noReplyMessage(uri)));
        }, FIMS_NO_REPLY_TIMEOUT),
      ),
    ]);
  }

  async get(uri: string, user: User) {
    return this.legacyRestService.get(uri, user);
  }

  async set(uri: string, body: any, user: User): Promise<RestPutResponse> {
    const msg = {
      method: FIMS_SET_METHOD,
      uri: uri,
      replyto: `/rest${uri}`,
      body: body,
      username: user.username,
    };
    return Promise.race([
      this.fimsService.send(msg),
      new Promise((resolve) =>
        setTimeout(() => {
          resolve(new RestNoReplyResponse(noReplyMessage(uri)));
        }, FIMS_NO_REPLY_TIMEOUT),
      ),
    ]);
  }

  async delete(uri: string, body: any, user: User) {
    const msg: FimsMsg = {
      method: FIMS_DEL_METHOD,
      uri: uri,
      replyto: `/rest${uri}`,
      body: body,
      username: user.username,
    };
    return Promise.race([
      this.fimsService.send(msg),
      new Promise((resolve) =>
        setTimeout(() => {
          resolve(new RestNoReplyResponse(noReplyMessage(uri)));
        }, FIMS_NO_REPLY_TIMEOUT),
      ),
    ]);
  }
}
