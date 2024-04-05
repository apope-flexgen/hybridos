import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, { useCallback, useEffect, useState } from 'react';
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

  const { product } = useAppContext();
  const { results, generateRowsData } = useGenerateRows();
  const axiosInstance = useAxiosWebUIInstance();

  const getInitialData = async () => {
    const filtersArray: any[] = [];
    Object.keys(filters).forEach((key) => {
      if (key !== null && key !== undefined) filtersArray.push([key, filters[key].toString()]);
    });
    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const resolvedAlertsURL = `/alerts/resolved?${filtersWithAmpersand}`;

    axiosInstance.get(resolvedAlertsURL).then((res) => {
      setResolvedAlertsData(res.data.data);
      setCount(res.data.count);
      setIsLoading(false);
    });
  };

  // TODO: fix page change handling
  const handlePageChange = () => {
    setFilters((prevFilters) => ({
      ...prevFilters,
      page: prevFilters.page ? prevFilters.page + 1 : 2,
    }));
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
        pagination
        rowsPerPage={[10, 20, 50]}
        paperSx={{ boxShadow: 'none', minHeight: '400px' }}
        expandable
        showExpandableRowIcons
        totalRows={count}
        onPageChange={() => { handlePageChange(); }}
        emptyStateText="No Resolved Alerts"
      />
    </Box>
  );
};

export default ResolvedAlertsTable;
