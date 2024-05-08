import * as React from 'react'
import { ValueType } from './configurablePages.dto'

export type TableDashboardDataTableDTO = {
  [siteGroup: string]: DataTableDTO;
};

export type DataTableDTO = {
  displayName: string;
  columns?: ColumnData[];
  rows?: RowData[];
  batteryViewData?: BatteryViewData[];
  alarmStatus?: AlarmFaultDataIndexable;
  faultStatus?: AlarmFaultDataIndexable;
};

export type DataTablesState = {
  [dataTableName: string]: {
    columns: ColumnData[];
    batteryViewData: BatteryViewData[];
    alarmStatus?: AlarmFaultDataIndexable;
    faultStatus?: AlarmFaultDataIndexable;
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
  [columnID: string]: ValueType | React.ReactElement;
};

export type AlarmFaultData = {
  [field: string]: ValueType;
};

export type TableDashboardDataTableIndexable = {
  [siteGroup: string]: DataTableIndexable;
};

export type DataTableIndexable = {
  displayName: string;
  columns?: ColumnDataIndexable;
  rows: RowDataIndexable;
  batteryViewData?: BatteryViewDataIndexable;
  alarmStatus?: AlarmFaultDataIndexable;
  faultStatus?: AlarmFaultDataIndexable;
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

export type AlarmFaultDataIndexable = {
  [id: string]: AlarmFaultData;
};

export type TableSortBehavior = {
  [key: string]: { columnToSortBy: keyof RowData; reverseOrder: boolean };
};

export type ColumnSortDirection = {
  [tableName: string]: {
    [columnID: string]: boolean;
  };
};
