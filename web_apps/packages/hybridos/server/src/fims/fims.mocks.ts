import { BodyDTO, GetDTO } from './dto/fims.dto';
import { FimsMsg } from './responses/fimsMsg.response';
import { FimsMsgDTO } from './dto/fims.dto';
import { FimsErrorMessages } from './fims.constants';
import { Observable } from 'rxjs';
import { User } from '../../../shared/types/dtos/auth.dto';
import { Roles } from '../../../shared/types/api/Users/Users.types';

export const AUTH_REQ = { user: 'username' };

export const FIMS_GET: GetDTO = {
  uri: '/fims/get/uri',
  replyto: '/fims/get/reply',
};

export const FIMS_SEND: BodyDTO = {
  uri: '/fims/set/uri',
  replyto: '/fims/set/reply',
  body: 'body',
};

export const FIMS_MSG: FimsMsgDTO = {
  method: 'get',
  uri: '/example/uri',
  replyto: '/example/reply',
  body: {},
  username: '',
};

export const FIMS_RESPONSE: FimsMsg = {
  method: 'get',
  uri: '/example/uri',
  replyto: '/example/reply',
  body: {},
  username: 'username',
};

export const FIMS_URI = '/example/uri';

export const FIMS_BODY = { example: 'body' };

export const FIMS_UID = 'exampleUid';

export const FIMS_RESP_FAILED: FimsMsg = {
  method: 'error',
  uri: '/example/uri',
  replyto: '/example/reply',
  body: FimsErrorMessages.SEND_FAILED,
  username: '',
};

export const FIMS_RESP_TIMED: FimsMsg = {
  method: 'error',
  uri: '/example/uri',
  replyto: '/example/reply',
  body: FimsErrorMessages.SEND_TIMED_OUT,
  username: '',
};

export const SUB_URI = '/example/uri';

export const SUB_UID = 'ExampleUserID';

export const SUB_REQ = {};

export const SUB_OBS: Observable<FimsMsg> = new Observable();

export const LIMIT = 5;

export const TTL = 10;

export const TOKEN = 'myExampleToken123';

export const USER: User = {
  username: 'username',
  role: Roles.Admin,
};

export const USER_NO_UNAME: User = {
  role: Roles.Admin,
};

export const EXCEPTION_MSG = 'example exception message';
