import {
  Box, IconButton, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { ColumnData, RowData } from 'shared/types/dtos/dataTables.dto';
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
    rows: RowData[],
    columnToSortBy?: keyof RowData,
    reverseOrder?: boolean
  ) => void
}

export const SortableColumn = (
  {
    label, columnID, columnData, rowData, tableName, generateRowsData,
  }: Props,
) => {
  const [reverseOrder, setReverseOrder] = useState<boolean>(false);

  const handleSort = () => {
    generateRowsData(tableName, columnData, rowData, columnID, !reverseOrder);
    setReverseOrder(!reverseOrder);
  };

  return (
    <Box sx={sortedTableHeaderSx}>
      <Typography text={label} variant="tableHeader" />
      {label && (
      <IconButton
        icon={reverseOrder ? 'ArrowDown' : 'ArrowUp'}
        onClick={handleSort}
        size="small"
      />
      )}
    </Box>
  );
};

export default SortableColumn;
