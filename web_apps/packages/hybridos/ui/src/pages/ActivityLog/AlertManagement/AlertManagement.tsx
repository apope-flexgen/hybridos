/* eslint-disable max-lines */
import {
  Box,
  PageLoadingIndicator,
  CardRow,
  Typography,
  Divider,
  ThemeType,
  MuiButton,
  customMUIScrollbar,
} from '@flexgen/storybook';
import { useContext, useState } from 'react';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import AlertForm from 'src/pages/ActivityLog/AlertManagement/AlertForm/AlertForm';
import AlertManagementTable from 'src/pages/ActivityLog/AlertManagement/AlertManagementTable/AlertManagementTable';
import {
  AlertManagementPageLayout,
  AlertManagementPageLayouts,
  determineAlertManagementHeader,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  actionsBoxSx,
  extraPadding,
  saveDisabledErrorBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { headerBoxSx, tableBoxSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { mainContentBoxSx } from 'src/pages/ActivityLog/Events/Styles';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

const AlertManagement = () => {
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [saveDisabled, setSaveDisabled] = useState<boolean>(false);
  // TODO: fix max line length
  // eslint-disable-next-line max-len
  const [alertManagementView, setAlertManagementView] = useState<AlertManagementPageLayout>(
    AlertManagementPageLayouts.TABLE,
  );
  const [alertFormValues, setAlertFormValues] = useState<AlertConfigurationObject | null>(null);
  const theme = useTheme() as ThemeType;

  const mainBoxSx = mainContentBoxSx(theme);
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const handleCancel = () => {
    setAlertManagementView(AlertManagementPageLayouts.TABLE);
    setAlertFormValues(null);
  };

  const handleResponse = (response: any, successMessage: string, errorMessage: string) => {
    if (response.data.success) {
      notifCtx?.notif('success', successMessage);
      setAlertFormValues(null);
      setAlertManagementView(AlertManagementPageLayouts.TABLE);
    } else {
      notifCtx?.notif('error', errorMessage);
    }
  };

  const handleDelete = () => {
    const deleteBody = { ...alertFormValues, deleted: true };
    axiosInstance.post(`/alerts/configuration/${alertFormValues?.id}`, deleteBody).then((res) => {
      handleResponse(
        res,
        'Alert configuration successfully deleted',
        'Error deleting alert configuration',
      );
    });
  };

  const handleSave = () => {
    const existingConfig = alertFormValues?.id;
    const urlToUse = existingConfig
      ? `/alerts/configuration/${alertFormValues?.id}`
      : '/alerts/configuration';
    axiosInstance.post(urlToUse, alertFormValues).then((res) => {
      handleResponse(
        res,
        'Alert configuration successfully updated',
        'Error updating alert configuration',
      );
    });
  };
  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <CardRow justifyContent="space-between" styleOverrides={extraPadding}>
          <Typography
            text={determineAlertManagementHeader(alertManagementView, alertFormValues)}
            variant="bodyXLBold"
          />
          {alertManagementView === AlertManagementPageLayouts.TABLE ? (
            <MuiButton
              label="Create Alert"
              startIcon="Add"
              onClick={() => {
                setAlertManagementView(AlertManagementPageLayouts.FORM);
              }}
            />
          ) : (
            <Box sx={saveDisabledErrorBoxSx}>
              <Box sx={actionsBoxSx}>
                {alertFormValues?.id && (
                  <MuiButton
                    label="Delete"
                    onClick={handleDelete}
                    variant="outlined"
                    color="error"
                    startIcon="Trash"
                  />
                )}
                <MuiButton label="Cancel" variant="text" onClick={handleCancel} />
                <MuiButton label="Save" onClick={handleSave} disabled={saveDisabled} />
              </Box>
              {saveDisabled && (
                <Typography
                  text="Please fill out all required fields"
                  variant="bodySBold"
                  color="error"
                />
              )}
            </Box>
          )}
        </CardRow>
        <Divider variant="fullWidth" orientation="horizontal" />
      </Box>
      {alertManagementView === AlertManagementPageLayouts.TABLE ? (
        <Box sx={{ ...tableBoxSx, overflowY: 'auto' }}>
          <AlertManagementTable
            setIsLoading={setIsLoading}
            setAlertManagementView={setAlertManagementView}
            setCurrentAlert={setAlertFormValues}
          />
        </Box>
      ) : (
        <Box sx={{ ...tableBoxSx, overflowY: 'auto', ...customMUIScrollbar(theme) }}>
          <AlertForm
            setIsLoading={setIsLoading}
            alertValues={alertFormValues}
            setAlertFormValues={setAlertFormValues}
            setSaveDisabled={setSaveDisabled}
            handleCancel={handleCancel}
          />
        </Box>
      )}
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};
export default AlertManagement;
