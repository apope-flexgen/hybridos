import { Injectable, Inject } from '@nestjs/common'
import { DBI_SERVICE } from 'src/dbi/dbi.constants';
import { IDBIService } from 'src/dbi/dbi.interface';
import { User } from 'src/users/dtos/user.dto';
import { AddLayout, ILayoutsService } from './layouts.interface';
import { LayoutsResponse } from './responses';

@Injectable()
export class LayoutsService implements ILayoutsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService, 
    ) { }

    getLayouts = async (): Promise<LayoutsResponse> => this.dbiService.getUIConfigLayouts();

    postLayouts = async (data: AddLayout, user: User): Promise<LayoutsResponse> => this.dbiService.postUIConfigLayouts(data, user);
}
