import {
  Box, PageLoadingIndicator, CardRow, Typography, Divider, ThemeType, MuiButton,
} from '@flexgen/storybook';
import { useState } from 'react';
import AlertManagementTable from 'src/pages/ActivityLog/AlertManagement/AlertManagementTable/AlertManagementTable';
import { AlertManagementPageLayout, AlertManagementPageLayouts, determineAlertManagementTable } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import { actionsBoxSx, extraPadding } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { headerBoxSx, tableBoxSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { mainContentBoxSx } from 'src/pages/ActivityLog/Events/Styles';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

const AlertManagement = () => {
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [
    alertManagementView,
    setAlertManagementView,
  ] = useState<AlertManagementPageLayout>(AlertManagementPageLayouts.TABLE);
  const [alertFormValues, setAlertFormValues] = useState<AlertConfigurationObject | null>(null);
  const theme = useTheme() as ThemeType;

  const mainBoxSx = mainContentBoxSx(theme);

  const handleCancel = () => {
    setAlertManagementView(AlertManagementPageLayouts.TABLE); setAlertFormValues(null);
  };

  const handleSave = () => {
    // TODO: add post request to save alertFormValues
    setAlertManagementView(AlertManagementPageLayouts.FORM);
  };

  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <CardRow justifyContent="space-between" styleOverrides={extraPadding}>
          <Typography text={determineAlertManagementTable(alertManagementView, alertFormValues)} variant="bodyXLBold" />
          {
            alertManagementView === AlertManagementPageLayouts.TABLE
              ? <MuiButton label="Create Alert" startIcon="Add" onClick={() => { setAlertManagementView(AlertManagementPageLayouts.FORM); }} />
              : (
                <Box sx={actionsBoxSx}>
                  <MuiButton label="Cancel" variant="text" onClick={handleCancel} />
                  <MuiButton label="Save" onClick={handleSave} />
                </Box>
              )
          }
        </CardRow>
        <Divider variant="fullWidth" orientation="horizontal" />
      </Box>
      {
        alertManagementView === AlertManagementPageLayouts.TABLE
          ? (
            <Box sx={tableBoxSx}>
              <AlertManagementTable
                setIsLoading={setIsLoading}
                setAlertManagementView={setAlertManagementView}
                setCurrentAlert={setAlertFormValues}
              />
            </Box>
          )
          : <Box sx={tableBoxSx} />
      }
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};

export default AlertManagement;
