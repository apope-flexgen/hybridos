import { Inject, Injectable } from '@nestjs/common';
import { Observable, map, merge } from 'rxjs';
import { computeClothedValue } from '../../utils/utils';
import { FimsService } from '../../fims/fims.service';
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface';
import {
  SiteStatusConfig,
  SiteStatusDataField,
  SiteStatusDataFieldConfig,
  SiteStatusResponse,
} from './sitestatus.interface';
import { DBI_URIs, IDBIService } from 'src/dbi/dbi.interface';
import { DBI_SERVICE } from 'src/dbi/dbi.interface';
import { AppEnvService } from 'src/environment/appEnv.service';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';

const SITE_SUMMARY_URI = '/site/summary';

@Injectable()
export class SiteStatusService {
  private siteConfiguration: SiteConfiguration;
  constructor(
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
    @Inject(FIMS_SERVICE)
    private readonly fimsService: FimsService,
    private readonly appEnvService: AppEnvService,
  ) {
    this.siteConfiguration = this.appEnvService.getSiteConfiguration();
  }
  config: SiteStatusConfig | undefined;

  subscribeToSiteStatus = async (): Promise<Observable<SiteStatusResponse>> => {
    const config = await this.getConfig();

    return this.getObservable(config);
  };

  getConfig = async (): Promise<SiteStatusConfig> => {
    if (this.config) {
      return this.config;
    }

    let unindexedConfig = await this.dbiService.getFromDBI(DBI_URIs.SITE_STATUS_BAR);
    let indexedDataSources = unindexedConfig.dataSources.map(
      (ds: SiteStatusDataFieldConfig, index: number): SiteStatusDataField => {
        return {
          ...ds,
          index,
        };
      },
    );
    unindexedConfig.dataSources = indexedDataSources;
    this.config = unindexedConfig;

    return this.config;
  };

  getObservable = async (config: SiteStatusConfig): Promise<Observable<SiteStatusResponse>> => {
    const fimsSubscribe = this.fimsService.subscribe(SITE_SUMMARY_URI);

    const dataObservables: Observable<SiteStatusResponse>[] = config.dataSources.map(
      (dataSource): Observable<SiteStatusResponse> => {
        return this.fimsService.subscribe(dataSource.uri).pipe(
          map((data): SiteStatusResponse => {
            const body = data.body;

            if (!body[dataSource.field]) {
              return {} as SiteStatusResponse;
            }

            const fieldData = body[dataSource.field];
            const fieldDataType = typeof fieldData;
            const validDataTypes = ['boolean', 'number', 'string'];

            if (validDataTypes.includes(fieldDataType)) {
              return {
                data: {
                  dataPoints: {
                    [`${dataSource.uri}/${dataSource.field}`]: {
                      label: dataSource.label,
                      value: fieldData,
                      unit: '',
                    },
                  },
                },
              } as SiteStatusResponse;
            }

            let value: String = fieldData['value'];
            let unit: String = fieldData['unit'];

            if (dataSource.dataType === 'number') {
              const { value: newValue, targetUnit } = computeClothedValue(
                Number(fieldData['value']),
                Number(fieldData['scaler']),
                fieldData['unit'],
                this.siteConfiguration,
              );

              value = newValue;
              unit = targetUnit;
            }

            return {
              data: {
                dataPoints: {
                  [`${dataSource.uri}/${dataSource.field}`]: {
                    label: dataSource.label,
                    value,
                    unit,
                    index: dataSource.index,
                  },
                },
              },
            } as SiteStatusResponse;
          }),
        );
      },
    );

    // FIXME: validate the data from fims first
    const siteStateObs: Observable<SiteStatusResponse> = fimsSubscribe.pipe(
      map((fimsData) => {
        const newDTO: SiteStatusResponse = {
          data: {
            activeFaults: fimsData.body['active_faults'],
            activeAlarms: fimsData.body['active_alarms'],
            siteState: fimsData.body['site_state'],
            siteStatusLabel: this.config.siteStatusLabel || '',
          },
        };

        return newDTO;
      }),
    );

    const obs = merge(siteStateObs, ...dataObservables);

    return obs;
  };
}
