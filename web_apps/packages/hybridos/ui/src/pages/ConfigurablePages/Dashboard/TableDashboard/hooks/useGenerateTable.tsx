import { useState } from 'react';
import {
  AlarmFaultDataIndexable,
  ColumnData,
  RowData,
  TableSortBehavior,
} from 'shared/types/dtos/dataTables.dto';
import {
  generateAlarmFaultStatusComponent,
  sortByAlarmFaultStatus,
} from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/TableDashboard.helpers';

const useGenerateDashboardTable = () => {
  const storedSortBehavior: TableSortBehavior = JSON.parse(
    localStorage.getItem('dashboardSortBehavior') || '{}',
  );
  const [results, setResults] = useState<{ [key: string]: RowData[] }>({});
  const [, setSortBehavior] = useState<TableSortBehavior>(storedSortBehavior);

  const generateRowsData = (
    tableName: string,
    columns: ColumnData[],
    newRows: RowData[],
    columnToSortBy?: keyof RowData,
    reverseOrder?: boolean,
    alarmStatus?: AlarmFaultDataIndexable,
    faultStatus?: AlarmFaultDataIndexable,
  ) => {
    setSortBehavior((prevSortBehavior) => {
      const currentSortBehavior = prevSortBehavior[tableName];
      const newSortBehavior = {
        columnToSortBy: columnToSortBy ?? currentSortBehavior?.columnToSortBy ?? columns[0].id,
        reverseOrder: reverseOrder ?? currentSortBehavior?.reverseOrder ?? false,
      };
      localStorage.setItem(
        'dashboardSortBehavior',
        JSON.stringify({ ...prevSortBehavior, [tableName]: newSortBehavior }),
      );

      setResults((prevResults) => {
        const updatedRows = (() => {
          let oldRows = prevResults[tableName] ?? [];
          newRows?.forEach((newRow) => {
            const tempNewRow = newRow;
            columns.forEach((column) => {
              if (column.id === 'alarm_status' || column.id === 'fault_status') {
                tempNewRow[column.id] = generateAlarmFaultStatusComponent(
                  column.id,
                  alarmStatus?.[newRow.id] ?? {},
                  faultStatus?.[newRow.id] ?? {},
                );
              } else {
                tempNewRow[column.id] = String(tempNewRow?.[column.id] ?? '-');
              }
            });
            oldRows = oldRows.filter((oldRow) => oldRow.id !== tempNewRow.id);
            oldRows.push(tempNewRow);
          });
          return oldRows;
        })();
        updatedRows.sort((a, b) => a.id.localeCompare(b.id, undefined, { numeric: true }));

        const rowIndex = newSortBehavior.columnToSortBy;
        if (rowIndex === 'alarm_status' || rowIndex === 'fault_status') {
          // TODO: shorten line
          // eslint-disable-next-line max-len
          updatedRows.sort((objectA, objectB) => sortByAlarmFaultStatus(objectA, objectB, rowIndex, newSortBehavior.reverseOrder));
        } else {
          updatedRows.sort((objectA, objectB) => {
            if (!newSortBehavior.reverseOrder) {
              return `${objectA[rowIndex]}`.localeCompare(`${objectB[rowIndex]}`, 'en', {
                numeric: true,
              });
            }
            return `${objectB[rowIndex]}`.localeCompare(`${objectA[rowIndex]}`, 'en', {
              numeric: true,
            });
          });
        }
        return { ...prevResults, [tableName]: updatedRows };
      });

      return { ...prevSortBehavior, [tableName]: newSortBehavior };
    });
  };

  return { generateRowsData, results };
};

export default useGenerateDashboardTable;
