import { Inject, Injectable } from '@nestjs/common';
import { map, merge, Observable } from 'rxjs';
import { FIMS_SERVICE, IFimsService } from '../../../fims/interfaces/fims.interface';
import { ConfigurablePageDTO, DisplayGroupDTO } from 'shared/types/dtos/configurablePages.dto';
import {
  clothedBodyFromFims,
  individualClothedBody,
  metadataFromDBI,
  nakedBodyFromFims,
  summaryDataFromFims,
} from '../configurablePages.types';
import { parseClothedData, parseNakedData, parseSummaryData } from '../Parser';
import { DBI_SERVICE, DBI_URIs } from 'src/dbi/dbi.interface';
import { IDBIService } from 'src/dbi/dbi.interface';
import { AppEnvService } from 'src/environment/appEnv.service';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import { LockModeService } from './lockMode/lockMode.service';
import { DEFAULT_URIS, NAKED_URIS } from './assetsPage.constants';

@Injectable()
export class AssetsPageService {
  private siteConfiguration: SiteConfiguration;

  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: IFimsService,
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
    private readonly appEnvService: AppEnvService,
    private readonly lockModeService: LockModeService,
  ) {
    this.siteConfiguration = this.appEnvService.getSiteConfiguration();
  }

  private setLockMode = (uri: string) => (state: boolean) =>
    this.lockModeService.setStateForURI(uri, state);

  async subscribeToCategory(
    assetKey: string,
    enableAssetPageControls: boolean,
  ): Promise<Observable<ConfigurablePageDTO>> {
    const individualMetadataArray = await this.getIndividualMetadata(assetKey);
    const individualMetadata = individualMetadataArray ? individualMetadataArray[0] : null;

    const defaultUri = DEFAULT_URIS[assetKey];

    if (individualMetadata === null && defaultUri === undefined) {
      console.error(
        `No custom or default configuration found for assetKey ${assetKey}, no response will be returned`,
      );
      return;
    }

    let queryUri = individualMetadata
      ? `${individualMetadata.info.sourceURI}${individualMetadata.info.baseURI}`
      : defaultUri;

    if (
      individualMetadata !== null &&
      'extension' in individualMetadata.info &&
      individualMetadata.info.extension.lastIndexOf('/') !== 0
    ) {
      console.warn('config includes partial baseURI in extension, please move this to baseURI');
      queryUri += individualMetadata.info.extension.slice(
        0,
        individualMetadata.info.extension.lastIndexOf('/'),
      );
    }

    let isClothed = true;
    for (const nakedURI of NAKED_URIS) {
      if (queryUri.startsWith(nakedURI)) {
        isClothed = false;
        break;
      }
    }

    // const initialDataFromFims = await this.fimsService.get(queryUri);
    // let summaryData = initialDataFromFims.body as summaryDataFromFims;

    let initialRawData = await this.getInitialRawData(queryUri, individualMetadataArray);

    let keys = Object.keys(initialRawData);
    if (keys.length === 1 && keys[0] === '/assets') {
      initialRawData = initialRawData['/assets'];
      keys = Object.keys(initialRawData);
    }

    const uriCandidates = keys.filter((key) => key !== 'summary');

    // for clothed bodies, if no value on the URI has a ui_type that
    // should be displayed, there is no reason to send that data to the UI
    const URIs = isClothed
      ? uriCandidates.filter((uriCandidate) => {
          for (const value of Object.values(initialRawData[uriCandidate])) {
            if (value !== null && typeof value === 'object' && 'ui_type' in value) {
              if (value.ui_type !== 'none') return true;
            }
          }

          return false;
        })
      : uriCandidates;

    const initialSendData = await this.getInitialSendData(
      initialRawData,
      URIs,
      isClothed,
      queryUri,
      enableAssetPageControls,
      individualMetadataArray,
    );

    return this.getMergedStream(
      initialSendData,
      URIs,
      queryUri,
      initialRawData,
      enableAssetPageControls,
      individualMetadataArray,
      isClothed,
    );
  }

  private getInitialRawData = async (
    queryUri: string,
    individualMetadata?: metadataFromDBI[],
  ): Promise<summaryDataFromFims> => {
    if (!individualMetadata) {
      return (await this.fimsService.get(queryUri)).body as summaryDataFromFims;
    }

    const initialRawData: summaryDataFromFims = {};

    individualMetadata.forEach((individualMetadata) => {
      if (!individualMetadata.info) {
        console.warn(
          `found initial metadata for ${queryUri} but no info found in the metadata, getting initial data will fail`,
        );
      }

      // if the metadata includes the hasMaintenanceActions field
      // add a new control to the metadata for the maintenanceActions control
      if (individualMetadata?.info.hasMaintenanceActions) {
        const maintenanceActionControl = {
          inputType: 'maint-action',
          name: 'Start Maintenance Action',
          uri: '/actions/control',
        };

        individualMetadata.controls.push(maintenanceActionControl)
      }
  
      let extension = individualMetadata.info.extension ?? '';
      const maybeNumber = Number(individualMetadata.info.numberOfItems);
      const numItems = isNaN(maybeNumber) ? -1 : maybeNumber;
      const range = individualMetadata.info.range
      const baseName = individualMetadata.info.name ?? '';
      const itemName = individualMetadata.info.itemName ?? '';

      (numItems < 0 && (range === undefined || range.length < 1))&&
        console.warn(
          `numberOfItems for ${queryUri} is not configured, getting initial data will fail`,
        );

      const leadingZero = (() => {
        if (extension.slice(-1) === '0') {
          extension = extension.slice(0, -1);
          return '0';
        }

        return '';
      })();

      if (range) {
        let numericExtensions = []
        range.forEach((rangeItem) => {
          if (typeof rangeItem === 'string' && rangeItem.includes('..')) {
              const arrayOfRange = rangeItem.split('..')
              const startNumber = Number(arrayOfRange[0]);
              const endNumber = Number(arrayOfRange[1]);
              if (startNumber > endNumber || Number.isNaN(startNumber) || Number.isNaN(endNumber)) {
                console.warn(
                  `received an invalid string within range array for ${queryUri}, getting initial data will fail`,
                );
              }
              for (var i = startNumber; i < endNumber + 1; i++) {
                numericExtensions.push(i)
              }
          } else {
            numericExtensions.push(Number(rangeItem))
          }
        })
        numericExtensions.forEach((rangeItem) => {
          const assetID = `${extension}${rangeItem < 10 ? leadingZero : ''}${rangeItem}`.substring(1);
    
          initialRawData[assetID] = {
            name: `${baseName} ${itemName} ${rangeItem}`,
            ...this.getClothedBodyFieldsForConfiguredAsset(individualMetadata),
          };
        })
      } else {
        for (let i = 1; i <= numItems; i++) {
          const templatedItem: boolean = extension.slice(-1) === '_' || extension.slice(-2) === '_0';
          const assetID = 
            templatedItem ?
            `${extension}${i < 10 ? leadingZero : ''}${i}`.substring(1)
            : `${extension}`.substring(1);
    
          const name = templatedItem ?  `${baseName} ${itemName} ${i}` : (itemName || `${baseName}`);
    
          initialRawData[assetID] = {
            name: name,
            ...this.getClothedBodyFieldsForConfiguredAsset(individualMetadata),
          };
        }
      }
    })
   
    return initialRawData;
  };

  // Configured Assets use naked body sends and configuration
  //
  // ESS has multiple processes running on some sites, meaning
  // 'get /assets/{ess|bms|pcs}' is a race condition and it is
  // unpredictable which process will reply.
  //
  // This replicates getting clothed body FIMS messages for a naked body asset
  // so we can continue using that 'get' for default assets while changing as little
  // as possible this close to release. Very ineffecient long term solution, but best
  // solution for now to introduce as few as possible points of failure for this fix
  //
  private getClothedBodyFieldsForConfiguredAsset = (
    individualMetadata: metadataFromDBI,
  ): { [fieldID: string]: individualClothedBody } => {
    const clothedBodyFields: { [fieldID: string]: individualClothedBody } = {};
    individualMetadata.statuses?.forEach((status) => {
      clothedBodyFields[status.uri.substring(1)] = {
        name: status.name ?? status.uri,
        unit: '',
        scaler: -1,
        type: '',
        ui_type: '',
        enabled: false,
        value: '-',
      };
    });
    individualMetadata.controls?.forEach((control) => {
      clothedBodyFields[control.uri.substring(1)] = {
        name: control.name ?? control.uri,
        unit: '',
        scaler: -1,
        type: '',
        ui_type: '',
        enabled: false,
        value: '-',
      };
    });
    return clothedBodyFields;
  };

  private getIndividualMetadata = async (assetKey: string): Promise<metadataFromDBI[] | null> => {
    const allMetadata = await this.getAssetConfig();
    const filteredMetadata = allMetadata.filter(
      (metadata) => metadata.info.assetKey.toLowerCase() === assetKey.toLowerCase(),
    );
    if (filteredMetadata.length === 0) {
      // TODO: return an error to the UI here?
      console.warn(
        `No metadata found for assetKey ${assetKey}, will use a default configuration if available, but this should be added to assets.json`,
      );
      return null;
    } else if (filteredMetadata.length > 1) {
      const arrayOfExtensions = filteredMetadata.map(function(item){ return item.info.extension });
      
      var containsDuplicateExtensions = arrayOfExtensions.some(function(item, idx){ 
          return arrayOfExtensions.indexOf(item) != idx 
      });

      if (containsDuplicateExtensions) {
        console.error(
          `Multiple metadata found for assetKey ${assetKey} with the same extension, the last found instance of this extension will be used, but duplicates should be removed from assets.json`,
        );
      }
    }

    return filteredMetadata;
  };

  private getMergedStream = (
    initialSendData: ConfigurablePageDTO,
    URIs: string[],
    baseUri: string,
    summaryData: summaryDataFromFims,
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
    isClothed: boolean,
  ): Observable<ConfigurablePageDTO> => {
    const base = new Observable<ConfigurablePageDTO>((observer) => {
      observer.next(initialSendData);
    });

    const hasAllControls = dbiMetadata?.[0].info.hasAllControls;
    const hasMaintenanceActions = dbiMetadata?.[0].info.hasMaintenanceActions;

    let uriSpecificObservables = URIs.map((assetID) => {
      return this.getUriSpecificObservable(
        `${baseUri}/${assetID}`,
        summaryData[assetID]['name'],
        enableAssetPageControls,
        dbiMetadata,
        isClothed,
        false,
        hasAllControls,
        hasMaintenanceActions,
      );
    });

    if (dbiMetadata && dbiMetadata[0].info.hasSummary) {
      uriSpecificObservables.push(
        this.getUriSpecificObservable(
          `${baseUri}/summary`,
          'Summary',
          enableAssetPageControls,
          dbiMetadata,
          false,
          true,
          hasAllControls,
          hasMaintenanceActions,
        ),
      );
    }

    const merged = merge(base, ...uriSpecificObservables);

    return merged;
  };

  private getAssetConfig = async (): Promise<metadataFromDBI[]> => {
    // TODO: fix any
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    const res: any = await this.dbiService.getFromDBI(DBI_URIs.UI_Config_Assets);
    if (res.data) {
      return res.data;
    }
    return {} as metadataFromDBI[];
  };

  private getIndividualAssetMetadata = (
    uri: string,
    dbiMetadata: metadataFromDBI[],
  ) => {
    let indexOfMetadata = -1;
      dbiMetadata.every((metadata, index) => {
        // check if the extension listed in this metadata is the full uri
        // if so, this is the correct metadata for this uri
        if (metadata.info.extension === `/${uri}`) {
          indexOfMetadata = index;
          return false;
        }
        // check if there is a range object in the metadata 
        // if so and if the asset number from this uri is listed in this range,
        // this is the correct metadata for this uri
        else if ('range' in metadata.info) {
          const lastUnderscore = uri.lastIndexOf("_")
          const assetNumber = uri.slice(lastUnderscore + 1)
          let numericExtensions = []
          metadata.info.range.forEach((rangeItem) => {
            if (typeof rangeItem === 'string' && rangeItem.includes('..')) {
                const arrayOfRange = rangeItem.split('..')
                const startNumber = Number(arrayOfRange[0]);
                const endNumber = Number(arrayOfRange[1]);
                for (var i = startNumber; i < endNumber + 1; i++) {
                  numericExtensions.push(i)
                }
            } else {
              numericExtensions.push(Number(rangeItem))
            }
          })
          if (numericExtensions.includes(Number(assetNumber))) {
            indexOfMetadata = index;
            return false;
          }
        }
        // check if this is a templated extension
        // i.e. the extension ends in _ or _0 (numberOfItems field has be used to generate uri)
        // if so, this is the correct metadata for this uri
        else if (
          metadata.info.extension.slice(-1) === '_' ||
          metadata.info.extension.slice(-2) === '_0'  
        ) {
          indexOfMetadata = index;
          return false;
        };
        return true;
      })

      if (indexOfMetadata === -1) {
        console.warn(`error getting metadata for uri ${uri}`);
      }

      return dbiMetadata[indexOfMetadata]
  }
  private getInitialSendData = async (
    summaryData: summaryDataFromFims,
    URIs: string[],
    isClothed: boolean,
    baseUri: string,
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
  ): Promise<ConfigurablePageDTO> => {
    const returnWithMetadata: ConfigurablePageDTO = {
      hasStatic: true,
      hasAllControls: dbiMetadata?.[0].info.hasAllControls ? true : false,
      hasMaintenanceActions: dbiMetadata?.[0].info.hasMaintenanceActions ? true : false,
      displayGroups: {},
    };

    URIs.forEach((uri) => {
      const parsedData: DisplayGroupDTO = isClothed
        ? parseClothedData(
            summaryData[uri],
            this.setLockMode(`${baseUri}/${uri}`),
            true,
            enableAssetPageControls,
            this.siteConfiguration,
          )
        : parseNakedData(
            summaryData[uri],
            this.setLockMode(`${baseUri}/${uri}`),
            this.getIndividualAssetMetadata(uri, dbiMetadata),
            true,
            enableAssetPageControls,
            this.siteConfiguration,
          );

      const displayName = summaryData[uri]['name'] ?? `${baseUri}/${uri}`;
      // const displayName =
      //   dbiMetadata &&
      //   dbiMetadata?.info?.itemName !== 'undefined' &&
      //   typeof dbiMetadata.info.itemName === 'string' &&
      //   dbiMetadata.info.itemName.length > 0
      //     ? `${dbiMetadata.info.itemName} ${uri.substring(uri.lastIndexOf('_') + 1)}`
      //     : summaryData[uri]['name'] ?? `${baseUri}/${uri}`;
      // FIXME: hardcoded
      returnWithMetadata.displayGroups[`${baseUri}/${uri}`] = {
        ...parsedData,
        displayName,
      };
    });


    if (dbiMetadata !== null && dbiMetadata[0].info.hasSummary) {
      const initialSummaryData = await this.fimsService.get(`${baseUri}/summary`);
      if (initialSummaryData.method === 'error')
        console.warn(`error while getting ${baseUri}/summary`);
      returnWithMetadata.displayGroups[`${baseUri}/summary`] = {
        ...parseSummaryData(
          dbiMetadata[0],
          initialSummaryData.body['summary'] || initialSummaryData.body,
          true,
          enableAssetPageControls,
          this.siteConfiguration,
        ),
        displayName: 'Summary',
      };
    }

    // initial send with metadata
    return returnWithMetadata;
  };

  private getUriSpecificObservable = (
    uri: string,
    name: string,
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
    isClothed: boolean,
    isSummary: boolean = false,
    hasAllControls: boolean = false,
    hasMaintenanceActions: boolean = false,
  ): Observable<ConfigurablePageDTO> => {
    const fimsSubscribe = this.fimsService.subscribe(uri);

    const observableForMeta: Observable<ConfigurablePageDTO> = fimsSubscribe.pipe(
      map((event) => {
        const assetURI = uri.substring(uri.lastIndexOf("/") + 1, uri.length);

        // if this event body contains information about maintenance actions
        // and this page is configured to show maintenance actions
        // send action data to frontend
        // otherwise, send empty object
        const maintenanceActions = hasMaintenanceActions ? (event.body.actions || {}) : {}

        const parsedData = isSummary
          ? parseSummaryData(
              this.getIndividualAssetMetadata(assetURI, dbiMetadata),
              event.body,
              false,
              enableAssetPageControls,
              this.siteConfiguration,
            )
          : !isClothed
          ? parseNakedData(
              event.body as nakedBodyFromFims,
              this.setLockMode(uri),
              this.getIndividualAssetMetadata(assetURI, dbiMetadata),
              false,
              enableAssetPageControls,
              this.siteConfiguration,
            )
          : parseClothedData(
              event.body as clothedBodyFromFims,
              this.setLockMode(uri),
              false,
              enableAssetPageControls,
              this.siteConfiguration,
            );
        return {
          hasStatic: false,
          hasAllControls: hasAllControls,
          hasMaintenanceActions: hasMaintenanceActions,
          displayGroups: {
            [uri]: {
              ...parsedData,
              displayName: name || uri,
              maintenanceActions: maintenanceActions,
            },
          },
        };
      }),
    );

    return observableForMeta;
  };
}
