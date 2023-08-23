import { Injectable, Inject } from '@nestjs/common'
import { DBI_SERVICE, DBI_URIs } from '../../dbi/dbi.interface'
import { IDBIService } from '../../dbi/dbi.interface'
import { LayoutsResponse } from './responses'
import { ILayoutsService } from './layouts.interface'
import { AddLayout } from './dto/layout.dto'
import { Observable } from 'rxjs'

@Injectable()
export class LayoutsService implements ILayoutsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService
    ) {}

    getLayouts = async (): Promise<Observable<LayoutsResponse>> =>
        this.dbiService.getFromDBI(DBI_URIs.UI_Config_Layout)

    postLayouts = async (data: AddLayout): Promise<LayoutsResponse> =>
        this.dbiService.postToDBI(DBI_URIs.UI_Config_Layout, data)
}
