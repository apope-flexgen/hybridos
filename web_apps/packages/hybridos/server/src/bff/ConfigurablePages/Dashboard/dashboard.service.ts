import { Inject, Injectable } from '@nestjs/common'
import { map, merge, Observable } from 'rxjs'
import { FimsService } from '../../../fims/fims.service'
import { FIMS_SERVICE } from '../../../fims/interfaces/fims.interface'
import { computeNakedValue } from '../../../utils/utils'
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto'
import {
    summaryInfoFromStream,
    ConfigBody,
    SingleCardData,
    SingleStatus,
} from './dashboard.interface'
import { IDBIService } from 'src/dbi/dbi.interface'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'

@Injectable()
export class DashboardService {
    configData: SingleCardData[] = []
    scalarStore: { [fullURI: string]: number } = {}
    constructor(
        @Inject(FIMS_SERVICE)
        private readonly fimsService: FimsService,
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) {
        this.getConfigData()
    }

    subscribeToDashboard = async (): Promise<
        Observable<ConfigurablePageDTO>
    > => {
        const staticData = await this.getStaticData()

        return this.getMergedStream(staticData)
    }

    private getConfigData = async (): Promise<SingleCardData[]> => {
        if (this.configData.length > 0) {
            return this.configData
        }
        const configData = await this.dbiService.getUIConfigDashboards();

        // FIXME: validate this instead
        this.configData = (configData as unknown as ConfigBody).data
        return this.configData
    }

    private getStaticData = async (): Promise<ConfigurablePageDTO> => {
        const configData = await this.getConfigData()

        const displayGroups = configData.reduce((acc, card) => {
            // store all scalars for use later. since this is coming from a static
            // config, scalars will not be different between users, so it is safe
            // to store them here.
            const { info, status } = card
            const baseURI = info.sourceURIs[0] + info.baseURI
            status.forEach((status) => {
                const { uri, scalar } = status
                if (scalar === null || scalar === undefined || scalar === '') {
                    this.scalarStore[baseURI + uri] = 1
                    return
                }
                this.scalarStore[baseURI + uri] = Number(scalar)
            })

            acc[baseURI] = {
                displayName: info.name,
                status: this.getStatusDTOs(status),
            }

            return acc
        }, {} as ConfigurablePageDTO['displayGroups'])

        return {
            hasStatic: true,
            displayGroups,
        }
    }

    private getStatusDTOs = (
        statuses: SingleStatus[]
    ): DisplayGroupDTO['status'] => {
        return statuses.reduce((acc, status) => {
            const { name, units, uri } = status

            const uriWithOpeningSlashRemoved = uri.replace(/^\//, '')

            acc[uriWithOpeningSlashRemoved] = {
                static: {
                    label: name,
                    unit: units,
                }
            }

            return acc
        }, {} as DisplayGroupDTO['status'])
    }

    private getMergedStream = (
        data: ConfigurablePageDTO
    ): Observable<ConfigurablePageDTO> => {
        const uris = Object.keys(data.displayGroups)

        const base: Observable<ConfigurablePageDTO> =
            new Observable((subscriber) => {
                subscriber.next(data)
            })

        const observables = uris.map((uri) => this.subscribeToUri(uri))

        const merged: Observable<ConfigurablePageDTO> = merge(
            base,
            ...observables
        )

        return merged
    }

    private subscribeToUri = (uri: string): Observable<ConfigurablePageDTO> => {
        const fimsSubscribe = this.fimsService.subscribe(uri)

        return fimsSubscribe.pipe(
            map((event) => {
                const transformed: ConfigurablePageDTO = {
                    hasStatic: false,
                    displayGroups: {
                        [uri]: {
                            displayName: '',
                            status: {},
                        },
                    },

                }

                const rawData = event.body as summaryInfoFromStream

                Object.entries(rawData).forEach(([key, value]) => {
                    const trueValue = typeof value === 'number' ? (() => {
                        const scalar = this.scalarStore[uri + '/' + key] || 1
                        return computeNakedValue(value, scalar)
                    })(): value


                    transformed.displayGroups[uri].status[key] = {
                        state: {
                            value: trueValue,
                        }
                    }
                })
                return transformed
            })
        )
    }
}
