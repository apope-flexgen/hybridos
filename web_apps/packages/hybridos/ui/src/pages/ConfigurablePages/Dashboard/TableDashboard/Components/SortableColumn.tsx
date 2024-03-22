import {
  Box, IconButton, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import {
  AlarmFaultDataIndexable, ColumnData, ColumnSortDirection, RowData,
} from 'shared/types/dtos/dataTables.dto';
import { sortedTableHeaderSx } from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/tableDashboard.styles';

interface Props {
  label: string,
  columnID: keyof RowData,
  columnData: ColumnData[],
  rowData: RowData[],
  tableName: string,
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
  faultStatus?: AlarmFaultDataIndexable
}

export const SortableColumn = (
  {
    label, columnID, columnData, rowData, tableName, generateRowsData, alarmStatus, faultStatus,
  }: Props,
) => {
  const storedReverseOrder: ColumnSortDirection = JSON.parse(localStorage.getItem('dashboardColumnSortDirection') || '{}');
  const [reverseOrder, setReverseOrder] = useState<ColumnSortDirection>(storedReverseOrder);

  const handleSort = () => {
    generateRowsData(
      tableName,
      columnData,
      rowData,
      columnID,
      !reverseOrder?.[tableName]?.[columnID] ?? false,
      alarmStatus,
      faultStatus,
    );
    setReverseOrder((prevReverseOrder) => {
      storedReverseOrder[tableName] ??= {};
      storedReverseOrder[tableName][columnID] = !prevReverseOrder?.[tableName]?.[columnID] ?? false;
      localStorage.setItem('dashboardColumnSortDirection', JSON.stringify(storedReverseOrder));
      return storedReverseOrder;
    });
  };

  return (
    <Box sx={sortedTableHeaderSx}>
      <Typography text={label} variant="tableHeader" />
      {label && (
      <IconButton
        icon={reverseOrder?.[tableName]?.[columnID] ? 'ArrowDown' : 'ArrowUp'}
        onClick={handleSort}
        size="small"
      />
      )}
    </Box>
  );
};

export default SortableColumn;
