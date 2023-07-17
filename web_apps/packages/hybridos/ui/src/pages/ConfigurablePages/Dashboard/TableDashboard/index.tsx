// TODO: fix lint
/* eslint-disable */
import { Box, CardContainer, CardRow, DataTable, Progress, Typography } from '@flexgen/storybook';
import { useState, useEffect } from 'react';
import {
  ColumnData,
  TableDashboardDataTableDTO,
  RowData,
  BatteryViewData,
} from 'shared/types/dtos/dataTables.dto';

import QueryService from 'src/services/QueryService';
import { batteryViewBoxSx, tableBoxSx } from './tableDashboard.styles';

type DataTablesState = {
  [dataTableName: string]: {
    columns: ColumnData[];
    rows: RowData[];
    batteryViewData: BatteryViewData[];
  };
};

const TableDashboard = () => {
  const [dataTables, setDataTables] = useState<DataTablesState>({});

  useEffect(() => {
    QueryService.getTableDashboard(handleDataFromSocket);
    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  const handleDataFromSocket = (newDataFromSocket: TableDashboardDataTableDTO) => {
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

          const rows = (() => {
            let oldRows = prevDataTables[name]?.rows ?? [];
            dataTableInfo.rows?.forEach((newRow) => {
              columns.forEach((column) => {
                newRow[column.id] = String(newRow?.[column.id] ?? '-');
              });
              oldRows = oldRows.filter((oldRow) => oldRow.id !== newRow.id);
              oldRows.push(newRow);
            });
            return oldRows;
          })();
          rows.sort((a, b) => a.id.localeCompare(b.id, undefined, { numeric: true }));
          acc[name] = {
            columns: columns,
            rows,
            batteryViewData,
          };
          return acc;
        },
        { ...prevDataTables },
      );

      return newState;
    });
  };

  const renderedDataTables = Object.entries(dataTables).map(([dataTableName, dataTableInfo]) => (
    <Box key={dataTableName} sx={{ width: '100%' }}>
      <CardContainer flexDirection='column' styleOverrides={{ paddingTop: '12px', width: '100%' }}>
        <CardRow>
          <Typography text={dataTableName} variant='headingS' />
        </CardRow>
        <Box sx={batteryViewBoxSx}>
          {dataTableInfo.batteryViewData &&
            dataTableInfo.batteryViewData.map((battery) => {
              return (
                <Progress
                  orientation='vertical'
                  width={50}
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
            columns={dataTableInfo.columns}
            dense
            pagination
            rowsPerPage={[20, 10, 5]}
            rowsData={dataTableInfo.rows}
          />
        </Box>
      </CardContainer>
    </Box>
  ));

  return <Box sx={tableBoxSx}>{renderedDataTables}</Box>;
};

export default TableDashboard;
