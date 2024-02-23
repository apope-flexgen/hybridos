import { ValueType } from './configurablePages.dto'

export type TableDashboardDataTableDTO = {
  [siteGroup: string]: DataTableDTO;
};

export type DataTableDTO = {
  displayName: string;
  columns?: ColumnData[];
  rows?: RowData[];
  batteryViewData?: BatteryViewData[];
};

export type DataTablesState = {
  [dataTableName: string]: {
    columns: ColumnData[];
    batteryViewData: BatteryViewData[];
  };
};

export type ColumnData = {
  id: string;
  label: string;
  minWidth?: number;
};

export type BatteryViewData = {
  label: string;
  value?: ValueType;
};

export type RowData = {
  id: string;
  [columnID: string]: ValueType;
};

export type TableDashboardDataTableIndexable = {
  [siteGroup: string]: DataTableIndexable;
};

export type DataTableIndexable = {
  displayName: string;
  columns?: ColumnDataIndexable;
  rows: RowDataIndexable;
  batteryViewData?: BatteryViewDataIndexable;
};

export type RowDataIndexable = {
  [id: string]: RowData;
};

export type ColumnDataIndexable = {
  [id: string]: ColumnData;
};

export type BatteryViewDataIndexable = {
  [id: string]: BatteryViewData;
};
