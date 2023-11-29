import { Inject, Injectable } from '@nestjs/common'
import { FimsService } from '../../fims/fims.service'
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface'
import { Observable, map } from 'rxjs';
import { FimsMsg } from 'src/fims/responses/fimsMsg.response';
import { ServiceAction, ServiceStatusResponse } from './dto/serviceStatusResponse.dto';

@Injectable()
export class SystemStatusService {
    constructor(
        @Inject(FIMS_SERVICE) 
        private readonly fimsService: FimsService
    ) {}

    subscribeToSystemStatus = (): Observable<ServiceStatusResponse> => {
        const fimsSubscribe = this.fimsService.subscribe('/cops/stats')

        const newObservable: Observable<ServiceStatusResponse> =
            fimsSubscribe.pipe(
                map((event) => {
                    const serviceName = event.uri.split("/").pop()
                    const parsedFimsData = this.parseSystemStatusData(event.body, serviceName)
                    return parsedFimsData
                })
            )

        return newObservable
    }

    private parseSystemStatusData = (rawSystemStatusData: any, serviceName: string): ServiceStatusResponse => {
        const actions: ServiceAction[] = Object.keys(rawSystemStatusData.controls).map((controlKey) => ({
            enabled: rawSystemStatusData.controls[controlKey].enabled as boolean,
            action: controlKey.toLowerCase() as 'start' | 'stop' | 'restart',
        }))

        const parsedData: ServiceStatusResponse = {
            serviceName: serviceName || '',
            serviceStatus: rawSystemStatusData.service_status || '',
            memoryUsage: rawSystemStatusData.last_mem_usage_pct || -1,
            uptime: rawSystemStatusData.elapsed_time || -1,
            lastRestart: rawSystemStatusData.last_restart || '',
            actions: actions
        }

        return parsedData
    }

    
    private parseSystemStatusGetData = (data: FimsMsg): ServiceStatusResponse[] => {
        const rawSystemStatusData = data.body.procStats;
        const parsedData: ServiceStatusResponse[] = [];
        rawSystemStatusData.forEach(
            (serviceObject) => {
                Object.keys(serviceObject).forEach(
                    (serviceName) => {
                        parsedData.push(this.parseSystemStatusData(serviceObject[serviceName], serviceName))
                    }
                )
        })
        return parsedData
    }

    async doServiceAction(
        serviceName: string, 
        action: 'start' | 'stop' | 'restart'
    ): Promise<FimsMsg> {
        const actionURI = `/cops/stats/${serviceName}/controls/${action}`;
        const body = true

        const message: FimsMsg = {
            method: 'set',
            uri: actionURI,
            replyto: '/response',
            body: body,
            username: 'web_ui',
        };

        return await this.fimsService.send(message)
    }

    async getSystemStatus() {
        const systemStatusResponse = await this.fimsService.get('/cops/stats')
        
        return this.parseSystemStatusGetData(systemStatusResponse);
    }
}
