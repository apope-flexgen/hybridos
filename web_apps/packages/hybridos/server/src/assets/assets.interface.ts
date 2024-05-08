import { Assets } from './dtos/asset.dto';
import { AssetsResponse } from './responses';

export interface IAssetsService {
  getAssets(): Promise<AssetsResponse>;
  postAssets(data: Assets): Promise<AssetsResponse>;
}
