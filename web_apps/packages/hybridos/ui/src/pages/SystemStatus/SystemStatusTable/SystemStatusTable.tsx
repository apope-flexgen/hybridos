'use client';

import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import { useEffect } from 'react';
import { systemStatusColumns } from 'src/pages/SystemStatus/SystemStatus.helpers';
import { systemStatusTableSx } from 'src/pages/SystemStatus/SystemStatus.styles';
import { SystemStatusTableProps } from 'src/pages/SystemStatus/SystemStatus.types';
import useGenerateSystemStatusTable from 'src/pages/SystemStatus/hooks/useGenerateTable';

const SystemStatusTable = ({ systemStatusData }: SystemStatusTableProps) => {
  const { generateRowsData, results } = useGenerateSystemStatusTable();

  useEffect(() => {
    generateRowsData(systemStatusData);
  }, [systemStatusData]);

  return (
    <Box sx={systemStatusTableSx} id="system-status-table">
      <DataTable
        rowsData={results}
        columns={systemStatusColumns(systemStatusData, generateRowsData)}
        dense
        headerColor="secondary"
        pagination
        rowsPerPage={[10, 20, 50]}
      />
    </Box>
  );
};

export default SystemStatusTable;
