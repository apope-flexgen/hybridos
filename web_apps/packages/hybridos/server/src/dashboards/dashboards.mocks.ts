import { DashboardsResponse } from "./responses";
import { AddDashboardRequest } from '../../../../hybridos/shared/types/dtos/dashboards.dto';

export const UI_CONFIG_DASH: DashboardsResponse = {
    dashboards: "Example UI Config Dash"
};

export const UI_CONFIG_DASH_AFTER_POST: DashboardsResponse = {
    dashboards: "Example UI Config Dash After"
};

export const POST_DASH: AddDashboardRequest = {
    data: [{
        info: { name: 'Example Post' },
        status: [{
            name: 'Example Post Status',
            scalar: '1',
            units: 'Example Unit',
            uri: '/example/uri'
        }]
    }]
}