import { DBI_URIs } from './dbi.interface';
import { FimsMsg } from '../fims/responses/fimsMsg.response';

export const DBI_DEST_URI = DBI_URIs.UI_Config_Assets;

export const FIMS_RESPONSE: FimsMsg = {
  method: 'method',
  uri: '/fims/uri',
  replyto: 'replyto',
  body: {},
  username: 'username',
};

export const FIMS_RESPONSE_ERR: FimsMsg = {
  method: 'method',
  uri: '/fims/uri',
  replyto: 'replyto',
  body: 'not found in collection',
  username: 'username',
};
