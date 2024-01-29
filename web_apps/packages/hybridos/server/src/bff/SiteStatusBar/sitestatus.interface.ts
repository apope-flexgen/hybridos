import { Observable } from 'rxjs';

export type SiteStatusConfig = {
  siteStatusLabel?: string;
  dataSources: SiteStatusDataField[];
};

export type SiteStatusDataFieldConfig = {
  uri: string;
  field: string;
  dataType: string;
  label: string;
};

export type SiteStatusDataField = SiteStatusDataFieldConfig & {
  index: number;
};

export type SiteStatusDataPointInfo = {
  label: string;
  value: string;
  unit: string;
  index: number;
};

export type SiteStatusDataPointsDTO = {
  [uri: string]: SiteStatusDataPointInfo;
};

export type SiteStatusResponse = {
  data: {
    siteStatusLabel?: string;
    activeFaults?: number;
    activeAlarms?: number;
    siteState?: string;
    dataPoints?: SiteStatusDataPointsDTO;
  };
};

export const SITE_STATUS_SERVICE = 'SiteStatusService';

export interface ISiteStatusService {
  subscribeToSiteStatus(): Promise<Observable<SiteStatusResponse>>;

  getConfig(): Promise<SiteStatusConfig>;

  getObservable(config: SiteStatusConfig): Promise<Observable<SiteStatusResponse>>;

  buildSiteStateObservable(): Observable<SiteStatusResponse>;

  buildDataSourceObservable(dataSource: SiteStatusDataField): Observable<SiteStatusResponse>;

  getFieldData(data: any, dataSource: SiteStatusDataField): any | undefined;

  fieldDataExists(data: any, dataSource: SiteStatusDataField): boolean;

  processData(fieldData: any, dataSource: SiteStatusDataField): SiteStatusResponse;

  processNakedData(
    fieldData: number | string | boolean,
    dataSource: SiteStatusDataField,
  ): SiteStatusResponse;

  processClothedData(fieldData: object, dataSource: SiteStatusDataField): SiteStatusResponse;

  buildResponse(dataSource: SiteStatusDataField, value: any, unit?: string): SiteStatusResponse;
}
