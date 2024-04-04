import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, { useCallback, useEffect, useState } from 'react';
import { useAppContext } from 'src/App/App';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { activeAlertsColumns, initialActiveAlertsFilters } from 'src/pages/ActivityLog/Alerts/alerts.helpers';
import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import useGenerateActiveAlertRows from 'src/pages/ActivityLog/Alerts/hooks/useGenerateRows';
import { ActiveAlertObject, AlertFilters } from 'src/pages/ActivityLog/activityLog.types';
import QueryService from 'src/services/QueryService';

export interface AlertsTableProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>,
}

const AlertsTable = ({
  setIsLoading,
}: AlertsTableProps) => {
  const [filters] = useState<AlertFilters>(initialActiveAlertsFilters);
  const [activeAlertsData, setActiveAlertsData] = useState<ActiveAlertObject[]>([]);
  const [count, setCount] = useState<number>(1);
  const { product } = useAppContext();
  const { results, generateRowsData } = useGenerateActiveAlertRows();
  const axiosInstance = useAxiosWebUIInstance();

  const getInitialData = async () => {
    const filtersArray: any[] = [];
    Object.keys(filters).forEach((key) => {
      if (key !== null && key !== undefined) filtersArray.push([key, filters[key].toString()]);
    });
    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const activeAlertsURL = `/alerts/active?${filtersWithAmpersand}`;

    axiosInstance.get(activeAlertsURL).then((res) => {
      setActiveAlertsData(res.data.data);
      setCount(res.data.count);
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
  }, []);

  useEffect(() => {
    if (activeAlertsData?.length > 0) {
      generateRowsData(activeAlertsData);
    }
  }, [activeAlertsData]);

  return (
    <Box sx={dataTableBox} id="active-alerts-table">
      <DataTable
        rowsData={results}
        columns={activeAlertsColumns(product || '')}
        dense
        headerColor="secondary"
        pagination
        rowsPerPage={[10, 20, 50]}
        paperSx={{ boxShadow: 'none', minHeight: '400px' }}
        expandable
        showExpandableRowIcons
        totalRows={count}
        onPageChange={() => { getInitialData(); }}
        emptyStateText="No Active Alerts"
      />
    </Box>
  );
};

export default AlertsTable;
