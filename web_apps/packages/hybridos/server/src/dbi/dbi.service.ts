import { Inject } from '@nestjs/common'
import { AssetsResponse } from 'src/assets/responses';
import { DashboardsResponse } from 'src/dashboards/responses';
import { LayoutsResponse } from 'src/layouts/responses';
import { User } from 'src/users/user.schema';
import { FimsMsg, FIMS_SERVICE, IFimsService } from '../fims/interfaces/fims.interface';
import { IDBIService, URIs } from './dbi.interface';

export class DBIService implements IDBIService {
    constructor(
        @Inject(FIMS_SERVICE)
        private readonly fimsService: IFimsService,
    ) { }

    getUIConfigAssets = async (): Promise<AssetsResponse> => {
        const response = await this.fimsService.get(`/dbi${URIs.UI_Config_Assets}`);
        return response.body;
    }

    getUIConfigDashboards = async (): Promise<DashboardsResponse> => {
        const response = await this.fimsService.get(`/dbi${URIs.UI_Config_Dashboard}`);
        return response.body;
    }

    getUIConfigLayouts = async (): Promise<LayoutsResponse> => {
        const response = await this.fimsService.get(`/dbi${URIs.UI_Config_Layout}`);
        return response.body;
    }

    private sendToFims = async (
        uri: string,
        data: { data: object[] },
        user?: User,
    ) => {
        let body: any = { ...data };

        if (!body.data) {
            if (user) {
                body.username = user.username;
                body.userrole = user.role;
            }

            body.created = Date.now();
        }

        return this.fimsService.send({
            method: 'post',
            uri,
            replyto: '',
            body,
            username: body.username ?? ''
        });
    }

    postUIConfigAssets = async (
        body: { data: object[] },
        user: User,
    ): Promise<AssetsResponse> => {
        const response = await this.sendToFims(`/dbi${URIs.UI_Config_Assets}`, body, user);
        return response.body['assets'];
    }

    postUIConfigDashboards = async (
        body: { data: object[] },
        user: User,
    ): Promise<DashboardsResponse> => {
        const response = await this.sendToFims(`/dbi${URIs.UI_Config_Dashboard}`, body, user);
        return response.body['dashboard'];
    }

    postUIConfigLayouts = async (
        body: { data: object[] },
        user: User,
    ): Promise<LayoutsResponse> => {
        const response = await this.sendToFims(`/dbi${URIs.UI_Config_Layout}`, body, user);
        return response.body['layout'];
    }

    postUIConfigAuditLog = async (
        data,
        user: User
    ): Promise<FimsMsg> => {
        return this.sendToFims(`/dbi${URIs.UI_Config_Audit_Log}${Date.now()}`, data, user);
    }
}
