import { Inject, Injectable } from '@nestjs/common'
import { map, merge, Observable } from 'rxjs'
import { FIMS_SERVICE, IFimsService } from '../../../fims/interfaces/fims.interface'
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto'
import {
    clothedBodyFromFims,
    metadataFromDBI,
    nakedBodyFromFims,
    summaryDataFromFims,
} from '../configurablePages.types'
import { parseClothedData, parseNakedData } from '../Parser'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { IDBIService } from 'src/dbi/dbi.interface'

@Injectable()
export class AssetsPageService {
    private assetConfig: metadataFromDBI

    constructor(
        @Inject(FIMS_SERVICE)
        private readonly fimsService: IFimsService,
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) {
        this.getAssetConfig()
    }

    async subscribeToCategory(
        category: string,
        isClothed: boolean,
        enableAssetPageControls: boolean
    ): Promise<Observable<ConfigurablePageDTO>> {
        // FIXME: baseUri should come from an enum/hash/something that is not
        // just this. this works for now to get the demo going, but it is really bad.
        // we need a single source of truth for clothed/naked and site/assets
        const baseUri = category === 'site' ? '/site' : `/assets/${category}`

        const initialDataFromFims = await this.fimsService.get(
            baseUri
        )

        const summaryData = initialDataFromFims.body as summaryDataFromFims
        const keys = Object.keys(summaryData)
        const URIs = keys.filter((key) => key !== 'summary')

        const [initialSendData, dbiMetadata] = await this.getInitialSendData(
            summaryData,
            URIs,
            isClothed,
            baseUri,
            enableAssetPageControls,
        )

        return this.getMergedStream(initialSendData, URIs, baseUri, summaryData, enableAssetPageControls, dbiMetadata)
    }

    private getMergedStream = (
        initialSendData: ConfigurablePageDTO,
        URIs: string[],
        baseUri: string,
        summaryData: summaryDataFromFims,
        enableAssetPageControls: boolean,
        dbiMetadata?: metadataFromDBI
    ): Observable<ConfigurablePageDTO> => {
        const base = new Observable<ConfigurablePageDTO>((observer) => {
            observer.next(initialSendData)
        })

        const uriSpecificObservables = URIs.map((uri) => {
            return this.getUriSpecificObservable(
                `${baseUri}/${uri}`,
                summaryData[uri]['name'],
                enableAssetPageControls,
                dbiMetadata
            )
        })

        const merged = merge(base, ...uriSpecificObservables)

        return merged
    }

    private getAssetConfig = async (): Promise<metadataFromDBI> => {
        if (this.assetConfig) {
            return this.assetConfig
        }

        // TODO: fix any
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        const res: any = await this.dbiService.getUIConfigAssets();
        this.assetConfig = res.data[0]
        return this.assetConfig
    }

    private getInitialSendData = async (
        summaryData: summaryDataFromFims,
        URIs: string[],
        isClothed: boolean,
        baseUri: string,
        enableAssetPageControls: boolean,
    ): Promise<[ConfigurablePageDTO, metadataFromDBI]> => {
        let dbiMetadata: metadataFromDBI = null
        if (!isClothed) {
            dbiMetadata = await this.getAssetConfig()
        }

        const returnWithMetadata: ConfigurablePageDTO = {
            hasStatic: true,
            displayGroups: {},
        }

        URIs.forEach((uri) => {
            const parsedData: DisplayGroupDTO = isClothed
                ? parseClothedData(summaryData[uri], true, enableAssetPageControls)
                : parseNakedData(summaryData[uri], dbiMetadata, true, enableAssetPageControls)

            // FIXME: hardcoded
            returnWithMetadata.displayGroups[`${baseUri}/${uri}`] = {
                ...parsedData,
                displayName: summaryData[uri]['name'] || `${baseUri}/${uri}`,
            }
        })

        // initial send with metadata
        return [returnWithMetadata, dbiMetadata]
    }

    private getUriSpecificObservable = (
        uri: string,
        name: string,
        enableAssetPageControls: boolean,
        dbiMetadata?: metadataFromDBI,
    ): Observable<ConfigurablePageDTO> => {
        const fimsSubscribe = this.fimsService.subscribe(uri)

        const observableForMeta: Observable<ConfigurablePageDTO> = fimsSubscribe.pipe(
            map((event) => {
                const parsedData = dbiMetadata
                    ? parseNakedData(
                          event.body as nakedBodyFromFims,
                          dbiMetadata,
                          false,
                          enableAssetPageControls,
                      )
                    : parseClothedData(event.body as clothedBodyFromFims, false, enableAssetPageControls)
                return {
                    hasStatic: false,
                    displayGroups: {
                        [uri]: {
                            ...parsedData,
                            displayName: name || uri,
                        },
                    },
                }
            })
        )

        return observableForMeta
    }
}
