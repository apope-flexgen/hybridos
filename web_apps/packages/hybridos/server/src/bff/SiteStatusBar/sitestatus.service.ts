import { Inject, Injectable } from '@nestjs/common';
import { Observable, filter, map, merge, mergeMap } from 'rxjs';
import { computeClothedValue, computeNakedValue } from '../../utils/utils';
import { FimsService } from '../../fims/fims.service';
import { FIMS_SERVICE } from '../../fims/interfaces/fims.interface';
import {
  ISiteStatusService,
  SiteStatusConfig,
  SiteStatusDataField,
  SiteStatusDataFieldConfig,
  SiteStatusResponse,
} from './sitestatus.interface';
import { DBI_URIs, IDBIService } from 'src/dbi/dbi.interface';
import { DBI_SERVICE } from 'src/dbi/dbi.interface';
import { SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

const SITE_SUMMARY_URI = '/site/summary';
const ACTIVE_ALERTS_URI = '/events/alerts';
@Injectable()
export class SiteStatusService implements ISiteStatusService {
  private siteConfiguration: SiteConfiguration;
  constructor(
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
    @Inject(FIMS_SERVICE)
    private readonly fimsService: FimsService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
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
    const dataObservables: Observable<SiteStatusResponse>[] = config.dataSources.map(
      (dataSource): Observable<SiteStatusResponse> => this.buildDataSourceObservable(dataSource),
    );

    const siteStateObs: Observable<SiteStatusResponse> = this.buildSiteStateObservable();
    const alertStateObs: Observable<SiteStatusResponse> = await this.buildAlertStateObservable();

    return merge(siteStateObs, alertStateObs, ...dataObservables);
  };

  buildSiteStateObservable = (): Observable<SiteStatusResponse> => {
    const siteSummaryFimsSubscribe = this.fimsService.subscribe(SITE_SUMMARY_URI);

    // FIXME: validate the data from fims first
    const siteStateObs: Observable<SiteStatusResponse> = siteSummaryFimsSubscribe.pipe(
      map((fimsData) => {
        const newDTO: SiteStatusResponse = {
          data: {
            baseData: {
              activeFaults: fimsData.body['active_faults'],
              activeAlarms: fimsData.body['active_alarms'],
              siteState: fimsData.body['site_state'],
              siteStatusLabel: this.config?.siteStatusLabel || '',
            },
          },
        };

        return newDTO;
      }),
    );

    return siteStateObs;
  };

  getActiveAlertsData = async (): Promise<number> => {
    const activeAlertsFimsMsg = (await this.fimsService.get(ACTIVE_ALERTS_URI)).body['count'];
    return activeAlertsFimsMsg;
  };

  buildAlertStateObservable = async (): Promise<Observable<SiteStatusResponse>> => {
    const initialActiveAlertsFimsMsg = await this.getActiveAlertsData();
    const activeAlertsFimsSubscribe = this.fimsService.subscribe(ACTIVE_ALERTS_URI);
    const base = new Observable<SiteStatusResponse>((observer) => {
      observer.next({
        data: {
          baseData: {
            activeAlerts: initialActiveAlertsFimsMsg,
          },
        },
      });
    });

    // FIXME: validate the data from fims first
    const activeAlertsData: Observable<SiteStatusResponse> = activeAlertsFimsSubscribe.pipe(
      mergeMap(async (fimsMsg) => {
        const newData = await this.getActiveAlertsData();
        const newDTO: SiteStatusResponse = {
          data: {
            baseData: {
              activeAlerts: newData,
            },
          },
        };

        return newDTO;
      }),
    );

    return merge(base, activeAlertsData);
  };

  buildDataSourceObservable = (dataSource: SiteStatusDataField): Observable<SiteStatusResponse> => {
    return this.fimsService.subscribe(dataSource.uri).pipe(
      filter((data) => this.fieldDataExists(data, dataSource)),
      map((data): SiteStatusResponse => {
        const fieldData = this.getFieldData(data, dataSource);

        return this.processData(fieldData, dataSource);
      }),
    );
  };

  getFieldData = (data: any, dataSource: SiteStatusDataField): any | undefined => {
    return data?.body[dataSource.field];
  };

  fieldDataExists = (data: any, dataSource: SiteStatusDataField): boolean => {
    const fieldData = this.getFieldData(data, dataSource);
    return fieldData !== undefined;
  };

  processData = (fieldData: any, dataSource: SiteStatusDataField): SiteStatusResponse => {
    switch (typeof fieldData) {
      case 'boolean':
      case 'number':
      case 'string':
        return this.processNakedData(fieldData, dataSource);

      case 'object':
        if (fieldData['value'] !== undefined) {
          return this.processClothedData(fieldData, dataSource);
        }
        break;
    }

    return {} as SiteStatusResponse;
  };

  processNakedData = (
    fieldData: number | string | boolean,
    dataSource: SiteStatusDataField,
  ): SiteStatusResponse => {
    const fieldDataType = typeof fieldData;
    if (fieldDataType === 'boolean' || fieldDataType === 'string') {
      return this.buildResponse(dataSource, fieldData);
    }

    const { value } = computeNakedValue(
      fieldData as number,
      dataSource.scalar,
      dataSource.unit,
      dataSource.precision,
    );
    return this.buildResponse(dataSource, value, dataSource.unit);
  };

  processClothedData = (fieldData: object, dataSource: SiteStatusDataField): SiteStatusResponse => {
    let value: string = fieldData['value'];
    let unit: string = fieldData['unit'];

    if (dataSource.dataType === 'number') {
      const { value: newValue, targetUnit } = computeClothedValue(
        Number(fieldData['value']),
        Number(fieldData['scaler']),
        fieldData['unit'],
        this.siteConfiguration,
        dataSource.precision,
      );

      value = newValue;
      unit = targetUnit;
    }

    return this.buildResponse(dataSource, value, unit);
  };

  buildResponse = (dataSource: SiteStatusDataField, value: any, unit = ''): SiteStatusResponse => {
    return {
      data: {
        dataPoints: {
          [`${dataSource.uri}/${dataSource.field}`]: {
            label: dataSource.label,
            value: value.toString(),
            unit,
            index: dataSource.index,
          },
        },
      },
    };
  };
}
