// TODO: fix lint
/* eslint-disable */
import { Box, CardContainer, CardRow, DataTable, Progress, Typography } from '@flexgen/storybook';
import { useState, useEffect, useCallback } from 'react';
import {
  TableDashboardDataTableDTO,
  DataTablesState,
} from 'shared/types/dtos/dataTables.dto';
import QueryService from 'src/services/QueryService';
import { batteryViewBoxSx, tableBoxSx } from './tableDashboard.styles';
import useGenerateDashboardTable from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/hooks/useGenerateTable';
import { tableColumns } from 'src/pages/ConfigurablePages/Dashboard/TableDashboard/TableDashboard.helpers';

const TableDashboard = () => {
  const [dataTables, setDataTables] = useState<DataTablesState>({});

  const { generateRowsData, results } = useGenerateDashboardTable();

  const handleDataFromSocket = useCallback((newDataFromSocket: TableDashboardDataTableDTO) => {
    setDataTables((prevDataTables) => {
      const newState = Object.values(newDataFromSocket).reduce(
        (acc, dataTableInfo) => {
          const name = dataTableInfo.displayName;
          const columns = dataTableInfo.columns ?? prevDataTables[name]?.columns ?? [];

          const batteryViewData =
            dataTableInfo.batteryViewData === undefined
              ? prevDataTables[name]?.batteryViewData
              : (() => {
                let oldBatteryViewData = prevDataTables[name]?.batteryViewData ?? [];
                dataTableInfo.batteryViewData?.forEach((newBatteryDataPoint) => {
                  oldBatteryViewData = oldBatteryViewData.filter(
                    (oldBatteryDataPoint) =>
                      oldBatteryDataPoint.label !== newBatteryDataPoint.label,
                  );
                  oldBatteryViewData.push(newBatteryDataPoint);
                });
                return oldBatteryViewData;
              })();
          batteryViewData.sort((a, b) =>
            a.label.localeCompare(b.label, undefined, { numeric: true }),
          );

          generateRowsData(name, columns, dataTableInfo.rows ?? []);

          acc[name] = {
            columns: columns,
            batteryViewData
          };
          return acc;
        },
        { ...prevDataTables },
      );

      return newState;
    });
  }, []);

  useEffect(() => {
    QueryService.getTableDashboard(handleDataFromSocket);

    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  const renderedDataTables = Object.entries(dataTables).map(([dataTableName, dataTableInfo]) => {
    const renderedColumns = tableColumns(dataTableName, dataTableInfo.columns, results[dataTableName], generateRowsData);

    return (<Box key={dataTableName} sx={{ width: '100%' }}>
      <CardContainer direction='column' styleOverrides={{ paddingTop: '12px', width: '100%' }}>
        <CardRow>
          <Typography text={dataTableName} variant='headingS' />
        </CardRow>
        <Box sx={batteryViewBoxSx}>
          {dataTableInfo.batteryViewData &&
            dataTableInfo.batteryViewData.map((battery) => {
              return (
                <Progress
                  orientation='vertical'
                  width={65}
                  fullWidth={false}
                  height={80}
                  label={battery.label}
                  value={Number(battery.value)}
                />
              );
            })}
        </Box>
        <Box sx={{ width: '100%' }}>
          <DataTable
            columns={renderedColumns}
            dense
            pagination
            rowsPerPage={[20, 10, 5]}
            rowsData={results[dataTableName] || []}
          />
        </Box>
      </CardContainer>
    </Box>)
  });

  return (
    <Box sx={tableBoxSx}>
      {renderedDataTables}
    </Box>
  );
};

export default TableDashboard;
