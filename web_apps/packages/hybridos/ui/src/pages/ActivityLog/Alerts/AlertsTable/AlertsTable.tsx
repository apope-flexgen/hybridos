import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, {
  useCallback, useEffect, useState,
} from 'react';
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
  const [filters] = useState<AlertFilters>(initialActiveAlertsFilters);
  const [activeAlertsData, setActiveAlertsData] = useState<ActiveAlertObject[]>([]);

  const { results, generateRowsData } = useGenerateActiveAlertRows();
  const axiosInstance = useAxiosWebUIInstance();

  const getInitialData = async () => {
    const filtersArray = Object.keys(filters)
      .filter((key) => key !== null && key !== undefined)
      .map((key: string) => [key, filters[key].toString()]);

    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const activeAlertsURL = `/alerts/active?${filtersWithAmpersand}`;

    axiosInstance.get(activeAlertsURL).then((res) => {
      setActiveAlertsData(res.data.data);
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
        columns={activeAlertsColumns()}
        dense
        headerColor="secondary"
        paperSx={{ boxShadow: 'none', maxHeight: '90%' }}
        expandable
        pagination
        rowsPerPage={[10, 20, 50]}
        showExpandableRowIcons
        emptyStateText="No Active Alerts"
      />
    </Box>
  );
};

export default AlertsTable;
