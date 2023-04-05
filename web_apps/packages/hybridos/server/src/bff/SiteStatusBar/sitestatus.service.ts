import { Inject, Injectable } from '@nestjs/common'
import { Observable, map, merge } from 'rxjs'
import { computeClothedValue } from '../../utils/utils'

import { FimsService } from '../../fims/fims.service'
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface'
import { SiteStatusConfig, SiteStatusResponse } from './sitestatus.interface'

import tempConfig from './temp.config.json'
const BASE_URI = '/site/operation'

@Injectable()
export class SiteStatusService {
    constructor(@Inject(FIMS_SERVICE) private readonly fimsService: FimsService) {}
    config: SiteStatusConfig | undefined

    subscribeToSiteStatus = async (): Promise<Observable<SiteStatusResponse>> => {
        const config = await this.getConfig()

        return this.getObservable(config)
    }

    getConfig = async (): Promise<SiteStatusConfig> => {
        if (this.config) {
            return this.config
        }

        this.config = tempConfig
        return this.config
    }

    getObservable = async (config: SiteStatusConfig): Promise<Observable<SiteStatusResponse>> => {
        const fimsSubscribe = this.fimsService.subscribe(BASE_URI)

        const dataObservables: Observable<SiteStatusResponse>[] = config.dataSources.map(
            (dataSource): Observable<SiteStatusResponse> => {
                return this.fimsService.subscribe(dataSource.uri).pipe(
                    map((data): SiteStatusResponse => {
                        const body = data.body

                        if (!body[dataSource.field]) {
                            return {} as SiteStatusResponse
                        }

                        const fieldDataObject = body[dataSource.field]

                        let value: String = fieldDataObject['value']
                        let unit: String = fieldDataObject['unit']

                        if (dataSource.dataType === 'number') {
                            const { value: newValue, targetUnit } = computeClothedValue(
                                Number(fieldDataObject['value']),
                                Number(fieldDataObject['scaler']),
                                fieldDataObject['unit']
                            )

                            value = newValue
                            unit = targetUnit
                        }

                        return {
                            data: {
                                dataPoints: {
                                    [dataSource.uri]: { label: dataSource.label, value, unit },
                                },
                            },
                        } as SiteStatusResponse
                    })
                )
            }
        )

        // FIXME: validate the data from fims first
        const siteStateObs: Observable<SiteStatusResponse> = fimsSubscribe.pipe(
            map((fimsData) => {
                const newDTO: SiteStatusResponse = {
                    data: {
                        activeFaults: fimsData.body['active_faults']['value'],
                        activeAlarms: fimsData.body['active_alarms']['value'],
                        siteState: fimsData.body['site_state']['value'],
                    },
                }

                return newDTO
            })
        )

        const obs = merge(siteStateObs, ...dataObservables)

        return obs
    }
}
