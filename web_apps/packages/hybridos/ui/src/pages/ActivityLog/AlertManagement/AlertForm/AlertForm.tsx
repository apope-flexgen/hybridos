import { Box, ThemeType, customMUIScrollbar } from '@flexgen/storybook';

import React, {
  useCallback, useEffect, useMemo, useState,
} from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

import {
  checkRequiredAlertValues,
  initialNewAlert,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';

import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { AlertConfigurationObject, Organization } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';
import AlertInfo from './AlertInfo/AlertInfo';
import Aliases from './Aliases/Aliases';
import OrganizationsModal from './OrganizationsModal/OrganizationsModal';
import RequiredResponseTime from './RequiredResponseTime/RequiredResponseTime';
import RuleLogic from './RuleLogic/RuleLogic';
import Scope from './Scope/Scope';
import Templates from './Templates/Templates';

import { AlertFormContext, AlertFormContextType } from './contexts/AlertFormContext';

export interface AlertFormProps {
  setAlertFormValues: React.Dispatch<React.SetStateAction<AlertConfigurationObject | null>>;
  alertValues: AlertConfigurationObject | null;
  setSaveDisabled: React.Dispatch<React.SetStateAction<boolean>>;
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>;
  handleCancel: () => void;
}

// Input form for a single alert, container for all fields
const AlertForm = ({
  alertValues,
  setAlertFormValues,
  setSaveDisabled,
  setIsLoading,
  handleCancel,
}: AlertFormProps) => {
  const [updatedAlertValues, setUpdatedAlertValues] = useState<AlertConfigurationObject>(
    alertValues || initialNewAlert,
  );
  const [orgModalOpen, setOrgModalOpen] = useState<boolean>(false);
  const [organizations, setOrganizations] = useState<Organization[]>([]);

  const theme = useTheme() as ThemeType;

  const axiosInstance = useAxiosWebUIInstance();

  useEffect(() => {
    if (updatedAlertValues) {
      setSaveDisabled(checkRequiredAlertValues(updatedAlertValues));
    }
  }, [updatedAlertValues]);

  const handleFieldChange = useCallback(
    (field: string, updatedValue: any, commaSeparatedList?: boolean) => {
      let valueToPass = updatedValue;
      if (commaSeparatedList) {
        valueToPass = updatedValue.split(',');
      }

      setUpdatedAlertValues((prevState) => {
        setAlertFormValues({
          ...prevState,
          [field]: valueToPass,
        });
        return {
          ...prevState,
          [field]: valueToPass,
        };
      });
    },
    [setAlertFormValues],
  );

  useEffect(() => {
    setIsLoading(true);
    axiosInstance.get('/alerts/organizations').then((orgResponse) => {
      setOrganizations(orgResponse.data.data);
      axiosInstance.get('/alerts/configuration').then((res) => {
        handleFieldChange('templates', res.data.templates);
        setIsLoading(false);
      });
    });
  }, []);

  const contextValue: AlertFormContextType = useMemo(
    () => ({
      handleFieldChange,
      alertValues: updatedAlertValues,
    }),
    [handleFieldChange, updatedAlertValues],
  );

  return (
    <AlertFormContext.Provider value={contextValue}>
      <OrganizationsModal
        organizations={organizations}
        open={orgModalOpen}
        onClose={() => setOrgModalOpen(false)}
        setOrganizations={setOrganizations}
        handleCancel={handleCancel}
      />
      <Box sx={{ dataTableBox, overflowY: 'auto', ...customMUIScrollbar(theme) }}>
        <AlertInfo />
        <Scope setOrgModalOpen={setOrgModalOpen} organizations={organizations} />
        <RequiredResponseTime />
        <Templates />
        <Aliases />
        <RuleLogic />
      </Box>
    </AlertFormContext.Provider>
  );
};

export default AlertForm;
