export type summaryInfoFromStream = {
  [key: string]:
    | number
    | string
    | boolean
    | { value: number | string | boolean; enabled?: boolean };
  name?: string;
};

export type ConfigBody = {
  _doc: string;
  _id: string;
  _version: string;
  data: SingleCardData[];
};

export type SingleItemInfo = {
  name: string;
  uri: string;
};

export type SingleCardInfo = {
  alarmFields: string[];
  baseURI: string;
  batteryView: boolean;
  batteryViewSourceURI: string | null;
  batteryViewURI: string;
  faultFields: string[];
  isTemplate: boolean;
  items: SingleItemInfo[];
  name: string;
  sourceURIs: string[];
};

export type SingleStatus = {
  name: string;
  scalar: string | null;
  sourceURI: string;
  units: string;
  uri: string;
  precision?: number;
};

export type SingleCardData = {
  info: SingleCardInfo;
  status: SingleStatus[];
};
