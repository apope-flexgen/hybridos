/* ----- Clothed Body Types ----- */

export interface summaryDataFromFims {
  [assetID: string]: clothedBodyFromFims;
}

export type clothedBodyFromFims = {
  name: string;
  [fieldID: string]: individualClothedBody | string | any;
};

export type individualClothedBody = {
  name: string;
  unit: string;
  scaler: number;
  type: string;
  ui_type: string;
  enabled: boolean;
  value: any; // should this be generic instead of any?
  options?: [{ name: string; return_value: any }];
};

/* ----- Naked Body Types ----- */

export interface metadataFromDBI {
  alarms: {
    alarmFields: any[];
    faultFields: any[];
  };
  allControls: controlDataFromNakedBody[];
  controls: controlDataFromNakedBody[];
  info: {
    [key: string]: any;
    assetKey: string;
    hasSummary: boolean;
    hasAllControls?: boolean;
    baseURI: string;
    sourceURI: string;
    itemName?: string;
    alarmFields: string[];
    faultFields: string[];
    extension?: string;
    numberOfItems?: string | number;
  };
  statuses: statusDataFromNakedBody[];
  summary: {
    name: string;
    uri: string;
    scalar?: number;
    units?: string;
  }[];
  summaryControls: {
    inputType: string;
    name: string;
    uri: string;
    scalar?: number;
    units?: string;
  }[];
}

export interface nakedBodyFromFims {
  [key: string]: statusComponentStateInfo | controlObjectForNakedBody | individualClothedBody;
}

export interface statusDataFromNakedBody {
  name: string;
  scalar?: string;
  units?: string;
  uri: string;
}

export interface controlDataFromNakedBody {
  inputType: string;
  name: string;
  scalar?: string;
  units?: string;
  uri: string;
}

export interface controlObjectForNakedBody {
  value?: boolean | number;
  enabled: boolean;
  options: {
    name: string;
    return_value: any;
  }[];
}

export type statusComponentStateInfo = string | number | boolean;

export interface controlComponentStateInfo {
  value?: string | number | boolean;
  enabled: boolean;
}

export enum ControlTypes {
  MaintenaceMode = 'Maintenance Mode',
}
