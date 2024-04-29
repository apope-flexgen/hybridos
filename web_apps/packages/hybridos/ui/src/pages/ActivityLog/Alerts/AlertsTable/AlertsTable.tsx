import { DataTablePagination } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, {
  ChangeEvent, useCallback, useEffect, useState,
} from 'react';
import { useAppContext } from 'src/App/App';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { activeAlertsColumns, initialActiveAlertsFilters } from 'src/pages/ActivityLog/Alerts/alerts.helpers';
import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import useGenerateActiveAlertRows from 'src/pages/ActivityLog/Alerts/hooks/useGenerateRows';
import { ActiveAlertObject, AlertFilters } from 'src/pages/ActivityLog/activityLog.types';
import QueryService from 'src/services/QueryService';

export interface AlertsTableProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>,
  setTotalActiveAlertCount: React.Dispatch<React.SetStateAction<string>>
}

const AlertsTable = ({
  setIsLoading,
  setTotalActiveAlertCount,
}: AlertsTableProps) => {
  const [filters, setFilters] = useState<AlertFilters>(initialActiveAlertsFilters);
  const [activeAlertsData, setActiveAlertsData] = useState<ActiveAlertObject[]>([]);
  const [count, setCount] = useState<number>(1);
  const [currentPage, setCurrentPage] = useState<number>(0);
  const [rowsPerPage, setRowsPerPage] = useState<number>(10);

  const { product } = useAppContext();
  const { results, generateRowsData } = useGenerateActiveAlertRows();
  const axiosInstance = useAxiosWebUIInstance();

  const handleChangeRowsPerPage = (event: ChangeEvent<HTMLInputElement | HTMLTextAreaElement>) => {
    setRowsPerPage(+event.target.value);
    setCurrentPage(0);
    setFilters({
      ...filters,
      page: 0,
      limit: +event.target.value,
    });
  };

  const handleChangePage = (event: unknown, newPage: number) => {
    setCurrentPage(newPage);
    setFilters({
      ...filters,
      page: newPage,
    });
  };

  const getInitialData = async () => {
    const filtersArray = Object.keys(filters)
      .filter((key) => key !== null && key !== undefined)
      .map((key: string) => [key, filters[key].toString()]);

    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const activeAlertsURL = `/alerts/active?${filtersWithAmpersand}`;

    axiosInstance.get(activeAlertsURL).then((res) => {
      setActiveAlertsData(res.data.data);
      setCount(res.data.count);
      setTotalActiveAlertCount(res.data.count.toString());
      setIsLoading(false);
    });
  };

  const handleDataOnSocket = useCallback(() => { getInitialData(); }, []);

  // start listening to web sockets
  useEffect(() => {
    QueryService.getAlertsPage(handleDataOnSocket);
    return () => {
      QueryService.cleanupSocket();
    };
  }, [handleDataOnSocket]);

  // initial GET request to populate data for page
  useEffect(() => {
    getInitialData();
  }, [filters]);

  useEffect(() => {
    generateRowsData(activeAlertsData);
  }, [activeAlertsData]);

  return (
    <Box sx={dataTableBox} id="active-alerts-table">
      <DataTable
        rowsData={results}
        columns={activeAlertsColumns(product || '')}
        dense
        headerColor="secondary"
        paperSx={{ boxShadow: 'none', maxHeight: '90%' }}
        expandable
        showExpandableRowIcons
        emptyStateText="No Active Alerts"
      />
      <DataTablePagination
        dataLength={count}
        onPageChange={handleChangePage}
        rowsPerPageOptions={[10, 25, 50]}
        onRowsPerPageChange={handleChangeRowsPerPage}
        page={currentPage}
        rowsPerPage={rowsPerPage}
      />
    </Box>
  );
};

export default AlertsTable;
