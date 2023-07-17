import { User } from '../../../users/dtos/user.dto';
import { RestPutResponse } from '../../responses/restput.response';

export const LEGACY_REST_API_PREFIX = 'rest';
export const LEGACY_REST_API_PREFIX_V1 = 'rest/v1';
export const LEGACY_REST_API_PREFIXES = [LEGACY_REST_API_PREFIX_V1, LEGACY_REST_API_PREFIX];
export const LEGACY_REST_SERVICE = 'LegacyRestService';

export interface ILegacyRestService {
  get(uri: string, user: User);
  set(uri: string, value: string, user: User): Promise<RestPutResponse>;
}
