import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import DataTable from '@flexgen/storybook/dist/components/DataDisplay/DataTable';
import React, { useContext, useEffect, useState } from 'react';

import { useAppContext } from 'src/App/App';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { alertManagementColumns } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import useGenerateAlertManagementRows from 'src/pages/ActivityLog/AlertManagement/hooks/useGenerateRows';

import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';

export interface AlertManagementTableProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>,
  setAlertManagementView: React.Dispatch<React.SetStateAction<'form' | 'table'>>,
  setCurrentAlert: React.Dispatch<React.SetStateAction<AlertConfigurationObject | null>>
}

const AlertManagementTable = ({
  setIsLoading,
  setAlertManagementView,
  setCurrentAlert,
}: AlertManagementTableProps) => {
  const [
    alertConfigurationData,
    setAlertConfigurationData,
  ] = useState<AlertConfigurationObject[]>([]);
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);
  const { product } = useAppContext();

  const duplicateAlert = (alert: AlertConfigurationObject) => {
    const newAlert = { ...alert, id: undefined, enabled: true };
    setAlertManagementView('form');
    setCurrentAlert(newAlert);
  };

  const editAlert = (alert: AlertConfigurationObject) => {
    setAlertManagementView('form');
    setCurrentAlert(alert);
  };

  const disableAlert = (value: boolean, alert: AlertConfigurationObject) => {
    const alertConfigurationURL = `/alerts/configuration/${alert.id}`;
    const disableAlertMessage = { alert, enabled: value };

    axiosInstance.post(alertConfigurationURL, disableAlertMessage).then((res) => {
      if (res.data.success) {
        notifCtx?.notif(
          'success',
          'Alert successfully updated',
        );
      } else {
        notifCtx?.notif(
          'error',
          'Error udpating alert',
        );
      }
    });
  };

  const {
    results,
    generateRowsData,
  } = useGenerateAlertManagementRows(disableAlert, editAlert, duplicateAlert);

  const getInitialData = async () => {
    setIsLoading(true);
    const alertConfigurationURL = '/alerts/configuration';

    axiosInstance.get(alertConfigurationURL).then((res) => {
      setAlertConfigurationData(res.data.data);
      setIsLoading(false);
    });
  };

  // initial GET request to populate data for page
  useEffect(() => {
    getInitialData();
  }, []);

  useEffect(() => {
    if (alertConfigurationData?.length > 0) {
      generateRowsData(alertConfigurationData);
    }
  }, [alertConfigurationData]);

  return (
    <Box sx={dataTableBox} id="alerts-management-table">
      <DataTable
        rowsData={results}
        columns={alertManagementColumns(product || '')}
        dense
        headerColor="secondary"
        pagination
        rowsPerPage={[10, 20, 50]}
        paperSx={{ boxShadow: 'none' }}
      />
    </Box>
  );
};

export default AlertManagementTable;
