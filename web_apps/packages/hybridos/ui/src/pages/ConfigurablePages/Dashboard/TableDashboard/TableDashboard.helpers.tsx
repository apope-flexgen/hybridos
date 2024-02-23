import { ColumnData, RowData } from 'shared/types/dtos/dataTables.dto';
import SortableColumn from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/Components/SortableColumn';

export const tableColumns = (
  tableName: string,
  columns: ColumnData[],
  rows: RowData[],
  generateRowsData: (
    tableName: string,
    columns: ColumnData[],
    rows: RowData[],
    columnToSortBy?: keyof RowData,
    reverseOrder?: boolean
  ) => void,
): ColumnData[] => {
  const parsedColumns: ColumnData[] = columns.map((column) => ({
    id: column.id,
    label: column.label,
    minWidth: column.minWidth,
    content: <SortableColumn
      label={column.label}
      columnID={column.id}
      columnData={columns}
      rowData={rows}
      tableName={tableName}
      generateRowsData={generateRowsData}
    />,
  }));

  return parsedColumns;
};

export default tableColumns;
