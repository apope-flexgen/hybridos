import { useState } from 'react';
import { AlarmFaultDataIndexable, ColumnData, RowData } from 'shared/types/dtos/dataTables.dto';
import { generateAlarmFaultStatusComponent, sortByAlarmFaultStatus } from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/TableDashboard.helpers';

const useGenerateDashboardTable = () => {
  const [results, setResults] = useState<{ [key: string]: RowData[] }>({});
  const [, setSortBehavior] = useState<{
    [key: string]: { columnToSortBy: keyof RowData, reverseOrder: boolean }
  }>({});

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

      setResults((prevResults) => {
        const updatedRows = (() => {
          let oldRows = prevResults[tableName] ?? [];
          newRows?.forEach((newRow) => {
            columns.forEach((column) => {
              if (column.id === 'alarm_status' || column.id === 'fault_status') {
                newRow[column.id] = generateAlarmFaultStatusComponent(
                  column.id,
                  alarmStatus?.[newRow.id] ?? {},
                  faultStatus?.[newRow.id] ?? {},
                );
              } else {
                newRow[column.id] = String(newRow?.[column.id] ?? '-');
              }
            });
            oldRows = oldRows.filter((oldRow) => oldRow.id !== newRow.id);
            oldRows.push(newRow);
          });
          return oldRows;
        })();
        updatedRows.sort((a, b) => a.id.localeCompare(b.id, undefined, { numeric: true }));

        const rowIndex = newSortBehavior.columnToSortBy;
        if (rowIndex === 'alarm_status' || rowIndex === 'fault_status') {
          updatedRows.sort((objectA, objectB) => sortByAlarmFaultStatus(
            objectA,
            objectB,
            rowIndex,
            newSortBehavior.reverseOrder,
          ));
        } else {
          updatedRows.sort(
            (objectA, objectB) => {
              if (!newSortBehavior.reverseOrder) return `${objectA[rowIndex]}`.localeCompare(`${objectB[rowIndex]}`, 'en', { numeric: true });
              return `${objectB[rowIndex]}`.localeCompare(`${objectA[rowIndex]}`, 'en', { numeric: true });
            },
          );
        }
        return ({ ...prevResults, [tableName]: updatedRows });
      });

      return ({ ...prevSortBehavior, [tableName]: newSortBehavior });
    });
  };

  return { generateRowsData, results };
};

export default useGenerateDashboardTable;
