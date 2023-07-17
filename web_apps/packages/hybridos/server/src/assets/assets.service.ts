import { Injectable, Inject } from '@nestjs/common'
import { DBI_SERVICE, DBI_URIs } from 'src/dbi/dbi.interface'
import { IDBIService } from 'src/dbi/dbi.interface'
import { IAssetsService } from './assets.interface'
import { AssetsResponse } from './responses'
import { AddAssetRequest } from 'shared/types/dtos/assets.dto'
import { User } from 'src/users/dtos/user.dto'
import { Assets } from './dtos/asset.dto'

@Injectable()
export class AssetsService implements IAssetsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService
    ) {}

    getAssets = async (): Promise<AssetsResponse> =>
        this.dbiService.getFromDBI(DBI_URIs.UI_Config_Assets)

    postAssets = async (data: Assets): Promise<AssetsResponse> =>
        this.dbiService.postToDBI(DBI_URIs.UI_Config_Assets, data)
}
