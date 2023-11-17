import { Inject, Injectable } from '@nestjs/common'
import { FimsService } from '../../fims/fims.service'
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface'
import { Observable, map } from 'rxjs';
import { FimsMsg } from 'src/fims/responses/fimsMsg.response';
import { ServiceStatusReponse } from './dto/serviceStatusResponse.dto';

@Injectable()
export class SystemStatusService {
    constructor(
        @Inject(FIMS_SERVICE) 
        private readonly fimsService: FimsService
    ) {}

    subscribeToSystemStatus = (): Observable<ServiceStatusReponse> => {
        const fimsSubscribe = this.fimsService.subscribe('/cops/stats')

        const newObservable: Observable<ServiceStatusReponse> =
            fimsSubscribe.pipe(
                map((event) => {
                    const parsedFimsData = this.parseSystemStatusDataFromFims(event)
                    return parsedFimsData
                })
            )

        return newObservable
    }

    private parseSystemStatusDataFromFims = (data: FimsMsg): ServiceStatusReponse => {
        const rawSystemStatusData = data.body;

        // parse the URI received to get the service name
        // TODO: if service name is included as field in future, replace this
        const indexOfLastSlash = data.uri.lastIndexOf('/')
        const serviceName = data.uri.substring(indexOfLastSlash + 1)

        const parsedData: ServiceStatusReponse = {
            serviceName: serviceName || '',
            serviceStatus: rawSystemStatusData.service_status || '',
            memoryUsage: rawSystemStatusData.last_mem_usage_pct || -1,
            uptime: rawSystemStatusData.elapsed_time || -1,
            lastRestart: rawSystemStatusData.last_restart || '',
        }

        return parsedData
    }

    // NOTE: This method does not currently work, backend support will need to be added for a get to /cops/stats
    async getSystemStatus() {
        const systemStatusResponse = await this.fimsService.get('/cops/stats')
        
        const parsedSystemStatusResponse = this.parseSystemStatusDataFromFims(systemStatusResponse);

        return parsedSystemStatusResponse
    }
}
