import {
  AlarmFaultData,
  AlarmFaultDataIndexable,
  ColumnData,
  RowData,
} from 'shared/types/dtos/dataTables.dto';
import AlarmFaultContainer from 'src/pages/ConfigurablePages/ComponentFactory/components/AlarmFaultContainer';
import SortableColumn from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/Components/SortableColumn';

export const tableColumns = (
  tableName: string,
  columns: ColumnData[],
  rows: RowData[],
  generateRowsData: (
    tableName: string,
    columns: ColumnData[],
    newRows: RowData[],
    columnToSortBy?: keyof RowData,
    reverseOrder?: boolean,
    alarmStatus?: AlarmFaultDataIndexable,
    faultStatus?: AlarmFaultDataIndexable,
  ) => void,
  alarmStatus?: AlarmFaultDataIndexable,
  faultStatus?: AlarmFaultDataIndexable,
): ColumnData[] => {
  const parsedColumns: ColumnData[] = columns.map((column) => ({
    id: column.id,
    label: column.label,
    minWidth: column.minWidth,
    content: (
      <SortableColumn
        label={column.label}
        columnID={column.id}
        columnData={columns}
        rowData={rows}
        tableName={tableName}
        generateRowsData={generateRowsData}
        alarmStatus={alarmStatus}
        faultStatus={faultStatus}
      />
    ),
  }));

  return parsedColumns;
};

export const generateAlarmFaultStatusComponent = (
  component: string,
  alarmStatus?: AlarmFaultData,
  faultStatus?: AlarmFaultData,
): JSX.Element => {
  const alarmPresent = component === 'alarm_status' && alarmStatus
    ? Object.values(alarmStatus).some((value) => value)
    : false;
  const faultPresent = component === 'fault_status' && faultStatus
    ? Object.values(faultStatus).some((value) => value)
    : false;
  return <AlarmFaultContainer showAlarm={alarmPresent} showFault={faultPresent} tableView />;
};

export const sortByAlarmFaultStatus = (
  objectA: RowData,
  objectB: RowData,
  rowIndex: keyof RowData,
  reverseOrder: boolean,
): number => {
  const objAProps = (objectA[rowIndex] as JSX.Element).props;
  const objBProps = (objectB[rowIndex] as JSX.Element).props;
  const objAHasComponent = Object.values(objAProps).some(
    (prop) => prop && (prop === objAProps.showAlarm || prop === objAProps.showFault),
  );
  const objBHasComponent = Object.values(objBProps).some(
    (prop) => prop && (prop === objBProps.showAlarm || prop === objBProps.showFault),
  );

  if (reverseOrder) {
    if (objAHasComponent && !objBHasComponent) return -1;
    if (!objAHasComponent && objBHasComponent) return 1;
  } else {
    if (objAHasComponent && !objBHasComponent) return 1;
    if (!objAHasComponent && objBHasComponent) return -1;
  }

  return `${objectA.id}`.localeCompare(`${objectB.id}`, 'en', { numeric: true });
};
