import { useState } from 'react';
import { ColumnData, RowData } from 'shared/types/dtos/dataTables.dto';

const useGenerateDashboardTable = () => {
  const [results, setResults] = useState<{ [key: string]: RowData[] }>({});
  const [, setSortBehavior] = useState<{
    [key: string]: { columnToSortBy: keyof RowData, reverseOrder: boolean }
  }>({});

  const generateRowsData = (
    tableName: string,
    columns: ColumnData[],
    rowsFromServer: RowData[],
    columnToSortBy?: keyof RowData,
    reverseOrder?: boolean,
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
          rowsFromServer?.forEach((newRow) => {
            columns.forEach((column) => {
              newRow[column.id] = String(newRow?.[column.id] ?? '-');
            });
            oldRows = oldRows.filter((oldRow) => oldRow.id !== newRow.id);
            oldRows.push(newRow);
          });
          return oldRows;
        })();
        updatedRows.sort((a, b) => a.id.localeCompare(b.id, undefined, { numeric: true }));

        const rowIndex = newSortBehavior.columnToSortBy;
        updatedRows.sort(
          (objectA, objectB) => {
            if (!newSortBehavior.reverseOrder) return `${objectA[rowIndex]}`.localeCompare(`${objectB[rowIndex]}`, 'en', { numeric: true });
            return `${objectB[rowIndex]}`.localeCompare(`${objectA[rowIndex]}`, 'en', { numeric: true });
          },
        );

        return ({ ...prevResults, [tableName]: updatedRows });
      });

      return ({ ...prevSortBehavior, [tableName]: newSortBehavior });
    });
  };

  return { generateRowsData, results };
};

export default useGenerateDashboardTable;
