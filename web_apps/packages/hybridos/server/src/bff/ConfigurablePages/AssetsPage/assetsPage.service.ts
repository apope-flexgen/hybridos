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
import { parseDefaultData, parseConfiguredData, parseSummaryData } from '../Parser';
import { DBI_SERVICE, DBI_URIs } from 'src/dbi/dbi.interface';
import { IDBIService } from 'src/dbi/dbi.interface';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import { LockModeService } from './lockMode/lockMode.service';
import { DEFAULT_URIS } from './assetsPage.constants';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AssetsPageService {
  private siteConfiguration: SiteConfiguration;

  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: IFimsService,
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
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
    const defaultUri = DEFAULT_URIS[assetKey];

    if (individualMetadataArray === null && defaultUri === undefined) {
      console.error(
        `No custom or default configuration found for assetKey ${assetKey}, no response will be returned`,
      );
      return;
    }

    let queryURIsArray = individualMetadataArray?.map((individualMetadata) => {
      let uri = individualMetadata
        ? `${individualMetadata.info.sourceURI}${individualMetadata.info.baseURI}`
        : defaultUri;

      if (
        individualMetadata !== null &&
        'extension' in individualMetadata.info &&
        individualMetadata.info.extension.lastIndexOf('/') !== 0
      ) {
        console.warn('config includes partial baseURI in extension, please move this to baseURI');
        uri += individualMetadata.info.extension.slice(
          0,
          individualMetadata.info.extension.lastIndexOf('/'),
        );
      }

      return uri;
    });

    if (!queryURIsArray || queryURIsArray.length === 0) {
      queryURIsArray = [defaultUri];
    }

    // const initialDataFromFims = await this.fimsService.get(queryUri);
    // let summaryData = initialDataFromFims.body as summaryDataFromFims;

    let initialRawData = await this.getInitialRawData(queryURIsArray, individualMetadataArray);

    let keys = Object.keys(initialRawData);
    if (keys.length === 1 && keys[0] === '/assets') {
      initialRawData = initialRawData['/assets'];
      keys = Object.keys(initialRawData); 
    }

    const uriCandidates = keys.filter((key) => !key.includes('summary'));

    // for clothed bodies, if no value on the URI has a ui_type that
    // should be displayed, there is no reason to send that data to the UI
    const URIs = uriCandidates.filter((uriCandidate) => {
      for (const value of Object.values(initialRawData[uriCandidate])) {
        if (value !== null && typeof value === 'object' && 'ui_type' in value) {
          if (value.ui_type !== 'none') return true;
        }
      }
      return false;
    })

    let isDefaultValues = {};
    URIs.forEach((uri) => {
      const isDefault = assetKey in DEFAULT_URIS;
      isDefaultValues = {...isDefaultValues, [uri]: isDefault};
    });
    
    const initialSendData = await this.getInitialSendData(
      initialRawData,
      URIs,
      isDefaultValues,
      queryURIsArray,
      enableAssetPageControls,
      individualMetadataArray,
      assetKey
    );

    return this.getMergedStream(
      initialSendData,
      URIs,
      queryURIsArray,
      initialRawData,
      enableAssetPageControls,
      individualMetadataArray,
      isDefaultValues,
      assetKey
    );
  }

  private getInitialRawData = async (
    queryURIsArray: string[],
    individualMetadata?: metadataFromDBI[],
  ): Promise<summaryDataFromFims> => {
    if (!individualMetadata) {
      const fimsData = (await this.fimsService.get(queryURIsArray[0])).body as summaryDataFromFims;
      
      const initialRawData: summaryDataFromFims = {};
      Object.entries(fimsData).forEach(([key, value]) => {
        const { name, ...fields } = value;
        initialRawData[`${queryURIsArray[0]}/${key}`] = { name, ...fields };
      });
      
      return initialRawData;
    }

    const initialRawData: summaryDataFromFims = {};

    individualMetadata.forEach((metadata, index) => {
      if (!metadata.info) {
        console.warn(
          `found initial metadata for ${queryURIsArray[index]} but no info found in the metadata, getting initial data will fail`,
        );
      }

      // if the metadata includes the hasMaintenanceActions field
      // add a new control to the metadata for the maintenanceActions control
      if (metadata.info.hasMaintenanceActions) {
        const maintenanceActionControl = {
          inputType: 'maint_action_control',
          name: 'Start Maintenance Action',
          uri: '/maint_actions_ctl',
        };


        // if this asset is configured to include a control for maintenance mode
        // put the maintenance actions right after the maintenance mode control so they appear together
        const indexOfMaintMode = metadata.controls.findIndex((control) => control.name.toLowerCase() === "maintenance mode");
        if (indexOfMaintMode !== -1) {
          metadata.controls.splice(indexOfMaintMode + 1, 0, maintenanceActionControl)
        } else { 
          metadata.controls.push(maintenanceActionControl)
        }

        // if user has included a batch controls array in the config and has matintenance actions
        // include maintenance action control on batch control array
        if (metadata.info.hasBatchControls && metadata.batchControls) {
          const indexOfMaintMode = metadata.batchControls.findIndex((control) => control.name.toLowerCase() === "maintenance mode");
          if (indexOfMaintMode !== -1) {
            metadata.batchControls.splice(indexOfMaintMode + 1, 0, maintenanceActionControl)
          } else { 
            metadata.batchControls.push(maintenanceActionControl)
          }
        }
      }
  
      let extension = metadata.info.extension ?? '';
      const maybeNumber = Number(metadata.info.numberOfItems);
      const numItems = isNaN(maybeNumber) ? -1 : maybeNumber;
      const range = metadata.info.range
      const baseName = metadata.info.name ?? '';
      const itemName = metadata.info.itemName ?? '';

      numItems < 0 &&
        (range === undefined || range.length < 1) &&
        console.warn(
          `numberOfItems for ${queryURIsArray[index]} is not configured, getting initial data will fail`,
        );

      const leadingZero = (() => {
        if (extension.slice(-1) === '0') {
          extension = extension.slice(0, -1);
          return '0';
        }

        return '';
      })();

      if (range) {
        let numericExtensions = [];
        range.forEach((rangeItem) => {
          if (typeof rangeItem === 'string' && rangeItem.includes('..')) {
              const arrayOfRange = rangeItem.split('..')
              const startNumber = Number(arrayOfRange[0]);
              const endNumber = Number(arrayOfRange[1]);
              if (startNumber > endNumber || Number.isNaN(startNumber) || Number.isNaN(endNumber)) {
                console.warn(
                  `received an invalid string within range array for ${queryURIsArray[index]}, getting initial data will fail`,
                );
              }
              for (var i = startNumber; i < endNumber + 1; i++) {
                numericExtensions.push(i)
              }
          } else {
            numericExtensions.push(Number(rangeItem));
          }
        });
        numericExtensions.forEach((rangeItem) => {
          const assetID = `${extension}${rangeItem < 10 ? leadingZero : ''}${rangeItem}`.substring(1);
    
          initialRawData[`${queryURIsArray[index]}/${assetID}`] = {
            name: `${baseName} ${itemName} ${rangeItem}`,
            ...this.getClothedBodyFieldsForConfiguredAsset(metadata),
          };
        });
      } else {
        for (let j = 1; j <= numItems; j++) {
          const templatedItem: boolean = extension.slice(-1) === '_' || extension.slice(-2) === '_0';
          const assetID = 
            templatedItem ?
            `${extension}${j < 10 ? leadingZero : ''}${j}`.substring(1)
            : `${extension}`.substring(1);
    
          const name = templatedItem ?  `${baseName} ${itemName} ${j}` : (itemName || `${baseName}`);
    
          initialRawData[`${queryURIsArray[index]}/${assetID}`] = {
            name: name,
            ...this.getClothedBodyFieldsForConfiguredAsset(metadata),
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
        options: [
          {
            name: '',
            return_value: '',
          }
        ]
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
    } 

    return filteredMetadata;
  };

  private getMergedStream = (
    initialSendData: ConfigurablePageDTO,
    URIs: string[],
    baseURIsArray: string[],
    summaryData: summaryDataFromFims,
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
    isDefaultValues: { [uri: string]: boolean },
    assetKey: string
  ): Observable<ConfigurablePageDTO> => {
    const base = new Observable<ConfigurablePageDTO>((observer) => {
      observer.next(initialSendData);
    });

    const hasBatchControls = dbiMetadata?.[0].info.hasBatchControls;
    const hasMaintenanceActions = dbiMetadata?.[0].info.hasMaintenanceActions;

    let uriSpecificObservables = URIs.map((uri) => {
      return this.getUriSpecificObservable(
        uri,
        summaryData[uri]['name'],
        enableAssetPageControls,
        dbiMetadata,
        isDefaultValues[uri],
        false,
        hasBatchControls,
        hasMaintenanceActions,
        assetKey
      );
    });

    dbiMetadata?.forEach((metadata, index) => {
      const hasBatchControls = metadata.info.hasBatchControls;
      if (metadata.info.hasSummary) {
        uriSpecificObservables.push(
          this.getUriSpecificObservable(
            `${baseURIsArray[index]}/summary`,
            'Summary',
            enableAssetPageControls,
            dbiMetadata,
            false,
            true,
            hasBatchControls,
            hasMaintenanceActions,
            assetKey
          )
        )
      }
    });

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

  private getIndividualAssetMetadata = (uri: string, dbiMetadata: metadataFromDBI[]) => {
    let indexOfMetadata = -1;
    dbiMetadata.every((metadata, index) => {
      // check if the uri listed in this metadata matches the uri passed in 
      // if so, this is the correct metadata for this uri
      const metadataUri = `${metadata.info.sourceURI}${metadata.info.baseURI}${metadata.info.extension}`;

      if (metadataUri === uri) {
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

    return dbiMetadata[indexOfMetadata];
  };
  private getInitialMaintenanceActionsData = async (
    URIs: string[],
    baseUri: string,
  ): Promise<any> => {
    // if this asset is configured to have maintenance actions 
    // get initial data to send to frontend
    const entries = await Promise.all(
      URIs.map(async (uri, index) => {
        const indiviudalMetadata = await this.fimsService.get(`${uri}/actions`)
        return [uri, indiviudalMetadata.body];
      })
    );
    return Object.fromEntries(entries);
  }

  private getInitialSendData = async (
    summaryData: summaryDataFromFims,
    URIs: string[],
    isDefaultValues: { [uri: string]: boolean },
    baseURIsArray: string[],
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
    assetKey: string,
  ): Promise<ConfigurablePageDTO> => {
    const hasMaintenanceActions = dbiMetadata?.[0].info.hasMaintenanceActions ? true : false;
    const hasBatchControls = dbiMetadata?.[0].info.hasBatchControls ? true : false

    const returnWithMetadata: ConfigurablePageDTO = {
      hasStatic: true,
      hasBatchControls: hasBatchControls,
      assetKey: assetKey,
      hasMaintenanceActions: hasMaintenanceActions,
      displayGroups: {},
    };

    const maintenanceActionsMetadata = hasMaintenanceActions ? await this.getInitialMaintenanceActionsData(URIs, baseURIsArray[0]) : undefined;

    URIs.forEach((uri, index) => {
      const parsedData: DisplayGroupDTO = isDefaultValues[uri]
        ? parseDefaultData(
            summaryData[uri],
            this.setLockMode(uri),
            true,
            enableAssetPageControls,
            this.siteConfiguration,
          )
        : parseConfiguredData(
            summaryData[uri],
            this.setLockMode(uri),
            this.getIndividualAssetMetadata(uri, dbiMetadata),
            true,
            enableAssetPageControls,
            this.siteConfiguration,
            maintenanceActionsMetadata?.[uri],
            hasBatchControls,
          );

      const displayName = summaryData[uri]['name'] ?? uri;
      // const displayName =
      //   dbiMetadata &&
      //   dbiMetadata?.info?.itemName !== 'undefined' &&
      //   typeof dbiMetadata.info.itemName === 'string' &&
      //   dbiMetadata.info.itemName.length > 0
      //     ? `${dbiMetadata.info.itemName} ${uri.substring(uri.lastIndexOf('_') + 1)}`
      //     : summaryData[uri]['name'] ?? `${baseUri}/${uri}`;
      // FIXME: hardcoded
      returnWithMetadata.displayGroups[uri] = {
        ...parsedData,
        displayName,
        tabKey: dbiMetadata?.[index]?.info.tabKey
      };
    });

    if (dbiMetadata) {
      for (let i = 0; i < dbiMetadata.length; i++) {
        if (dbiMetadata[i].info.hasSummary) {
          const initialSummaryData = await this.fimsService.get(`${baseURIsArray[i]}/summary`);
          if (initialSummaryData.method === 'error')
            console.warn(`error while getting ${baseURIsArray[i]}/summary`);
          returnWithMetadata.displayGroups[`${baseURIsArray[i]}/summary`] = {
            ...parseSummaryData(
              dbiMetadata[i],
              initialSummaryData.body['summary'] || initialSummaryData.body,
              true,
              enableAssetPageControls,
              this.siteConfiguration,
            ),
            displayName: 'Summary',
          };
        }
      }
    }

    // initial send with metadata
    return returnWithMetadata;
  };

  private getUriSpecificObservable = (
    uri: string,
    name: string,
    enableAssetPageControls: boolean,
    dbiMetadata: metadataFromDBI[],
    isDefault: boolean,
    isSummary: boolean = false,
    hasBatchControls: boolean = false,
    hasMaintenanceActions: boolean = false,
    assetKey: string,
  ): Observable<ConfigurablePageDTO> => {
    const fimsSubscribe = this.fimsService.subscribe(uri);

    const observableForMeta: Observable<ConfigurablePageDTO> = fimsSubscribe.pipe(
      map((event) => {
        // if this event body contains information about maintenance actions
        // and this page is configured to show maintenance actions
        // send action data to frontend
        // otherwise, send empty object
        const maintenanceActions = hasMaintenanceActions ? event.body.actions || {} : {};

        const parsedData = isSummary
          ? parseSummaryData(
              this.getIndividualAssetMetadata(uri, dbiMetadata),
              event.body,
              false,
              enableAssetPageControls,
              this.siteConfiguration,
            )
          : !isDefault
            ? parseConfiguredData(
              event.body as nakedBodyFromFims,
              this.setLockMode(uri),
              this.getIndividualAssetMetadata(uri, dbiMetadata),
              false,
              enableAssetPageControls,
              this.siteConfiguration,
              maintenanceActions,
            )
            : parseDefaultData(
              event.body as clothedBodyFromFims,
              this.setLockMode(uri),
              false,
              enableAssetPageControls,
              this.siteConfiguration,
            );
        return {
          hasStatic: false,
          hasBatchControls: hasBatchControls,
          assetKey: assetKey,
          hasMaintenanceActions: hasMaintenanceActions,
          displayGroups: {
            [uri]: {
              ...parsedData,
              displayName: name || uri,
              tabKey: !isDefault ? this.getIndividualAssetMetadata(uri, dbiMetadata).info.tabKey : undefined
            },
          },
        };
      }),
    );

    return observableForMeta;
  };
}
