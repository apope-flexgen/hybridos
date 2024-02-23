import {
  Box, IconButton, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { sortedTableHeaderSx } from 'src/pages/SystemStatus/SystemStatus.styles';
import { SystemStatusObject, SystemStatusRow } from 'src/pages/SystemStatus/SystemStatus.types';

export interface SortRowProps {
  label: string,
  column: keyof SystemStatusObject,
  systemStatusData: SystemStatusObject[],
  generateRowsData: (
    systemStatusData: SystemStatusObject[],
    sortByRow?: keyof SystemStatusObject,
    reverse?: boolean,
  ) => SystemStatusRow[],

}
const SortRow = ({
  label,
  column,
  systemStatusData,
  generateRowsData,
}: SortRowProps) => {
  const [reverseOrder, setReverseOrder] = useState<boolean>(true);
  const handleSort = () => {
    generateRowsData(systemStatusData, column, reverseOrder);
    setReverseOrder(!reverseOrder);
  };
  return (
    <Box sx={sortedTableHeaderSx}>
      <Typography text={label} variant="tableHeader" />
      <IconButton
        icon={reverseOrder ? 'ArrowDown' : 'ArrowUp'}
        onClick={handleSort}
        size="small"
      />
    </Box>
  );
};

export default SortRow;
