import { User } from '../../users/dtos/user.dto';
import { RestPutResponse } from '../responses/restput.response';

export const REST_API_PREFIX = 'rest/v2';
export const REST_SERVICE = 'RestService';

export interface IRestService {
  post(uri: string, body: any, user: User);

  get(uri: string, user: User);

  set(uri: string, body: any, user: User): Promise<RestPutResponse>;

  delete(uri: string, body: any, user: User);
}
