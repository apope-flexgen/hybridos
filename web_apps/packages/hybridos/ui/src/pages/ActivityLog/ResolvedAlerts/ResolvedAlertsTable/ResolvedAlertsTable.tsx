import { DataTablePagination } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, {
  ChangeEvent, useCallback, useEffect, useState,
} from 'react';
import { useAppContext } from 'src/App/App';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import useGenerateRows from 'src/pages/ActivityLog/ResolvedAlerts/hooks/useGenerateRows';
import { initialResolvedAlertsFilters, resolvedAlertsColumns } from 'src/pages/ActivityLog/ResolvedAlerts/resolvedAlerts.helpers';
import { AlertFilters, ResolvedAlertObject } from 'src/pages/ActivityLog/activityLog.types';
import QueryService from 'src/services/QueryService';

export interface ResolvedAlertsTableProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>,
}

const ResolvedAlertsTable = ({
  setIsLoading,
}: ResolvedAlertsTableProps) => {
  const [filters, setFilters] = useState<AlertFilters>(initialResolvedAlertsFilters);
  const [resolvedAlertsData, setResolvedAlertsData] = useState<ResolvedAlertObject[]>([]);
  const [count, setCount] = useState<number>(1);
  const [currentPage, setCurrentPage] = useState<number>(0);
  const [rowsPerPage, setRowsPerPage] = useState<number>(10);

  const { product } = useAppContext();
  const { results, generateRowsData } = useGenerateRows();
  const axiosInstance = useAxiosWebUIInstance();

  const getInitialData = async () => {
    const filtersArray = Object.keys(filters)
      .filter((key) => key !== null && key !== undefined)
      .map((key: string) => [key, filters[key].toString()]);

    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const resolvedAlertsURL = `/alerts/resolved?${filtersWithAmpersand}`;

    axiosInstance.get(resolvedAlertsURL).then((res) => {
      setResolvedAlertsData(res.data.data);
      setCount(res.data.count);
      setIsLoading(false);
    });
  };

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

  const handleDataOnSocket = useCallback(() => { getInitialData(); }, []);

  // start listening to web sockets
  useEffect(() => {
    QueryService.getAlertsPage(handleDataOnSocket);
    return () => {
      QueryService.cleanupSocket();
    };
  }, [handleDataOnSocket]);

  useEffect(() => {
    getInitialData();
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [filters]);

  useEffect(() => {
    generateRowsData(resolvedAlertsData);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [resolvedAlertsData]);

  return (
    <Box sx={dataTableBox} id="resolved-alerts-table">
      <DataTable
        rowsData={results}
        columns={resolvedAlertsColumns(product || '')}
        dense
        headerColor="secondary"
        paperSx={{ boxShadow: 'none', maxHeight: '90%' }}
        expandable
        showExpandableRowIcons
        emptyStateText="No Resolved Alerts"
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

export default ResolvedAlertsTable;
