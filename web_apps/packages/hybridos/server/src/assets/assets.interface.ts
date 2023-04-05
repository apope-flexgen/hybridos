import { User } from "src/users/dtos/user.dto"
import { AddAssetRequest } from "../../../shared/types/dtos/assets.dto"
import { AssetsResponse } from "./responses"

export interface IAssetsService {
  getAssets(): Promise<AssetsResponse>
  postAssets(data: AddAssetRequest, user: User): Promise<AssetsResponse>
}
