import { Inject, Injectable } from '@nestjs/common'
import { FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface'
import { ActiveAlertsResponse } from './responses/alerts.response'
import { mockedActiveAlertData } from './alerts.constants'
import { ActiveAlertsRequest } from './dtos/alerts.dto'

@Injectable()
export class AlertsService {
    constructor(@Inject(FIMS_SERVICE) private readonly fimsService: IFimsService) {}
    async activeAlerts(query: ActiveAlertsRequest): Promise<ActiveAlertsResponse> {
        // TODO: eventually this will use fims data coming from /events/alerts
        // for now, use mocks
        // const alertsFromFims = await this.fimsService.get('/events/alerts', query);
        
        const data = mockedActiveAlertData
        const count = mockedActiveAlertData.length

        return { data, count }
    }
}
