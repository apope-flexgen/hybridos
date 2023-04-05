import { Injectable, Inject } from '@nestjs/common'
import { DBI_SERVICE } from 'src/dbi/dbi.constants';
import { IDBIService } from 'src/dbi/dbi.interface';
import { IAssetsService } from './assets.interface';
import { AssetsResponse } from './responses';
import { AddAssetRequest } from 'shared/types/dtos/assets.dto';
import { User } from 'src/users/dtos/user.dto';

@Injectable()
export class AssetsService implements IAssetsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) { }

    getAssets = async (): Promise<AssetsResponse> => this.dbiService.getUIConfigAssets();

    postAssets = async (data: AddAssetRequest, user: User): Promise<AssetsResponse> => this.dbiService.postUIConfigAssets(data, user);
}
