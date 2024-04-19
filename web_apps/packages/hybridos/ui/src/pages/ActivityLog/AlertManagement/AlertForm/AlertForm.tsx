import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import React, {
  useCallback, useEffect, useMemo, useState,
} from 'react';
import { useAppContext } from 'src/App/App';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import { checkRequiredAlertValues, initialNewAlert } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';

import { dataTableBox } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';
import AlertInfo from './AlertInfo/AlertInfo';
import Aliases from './Aliases/Aliases';
import RequiredResponseTime from './RequiredResponseTime/RequiredResponseTime';
import RuleLogic from './RuleLogic/RuleLogic';
import Scope from './Scope/Scope';
import Templates from './Templates/Templates';
import { AlertFormContext, AlertFormContextType } from './contexts/AlertFormContext';

export interface AlertFormProps {
  setAlertFormValues: React.Dispatch<React.SetStateAction<AlertConfigurationObject | null>>,
  alertValues: AlertConfigurationObject | null
  setSaveDisabled: React.Dispatch<React.SetStateAction<boolean>>
}
export interface AlertFormRowProps {
  alertValues: AlertConfigurationObject,
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void
}

// Input form for a single alert, container for all fields
const AlertForm = ({
  alertValues,
  setAlertFormValues,
  setSaveDisabled,
}: AlertFormProps) => {
  const [
    updatedAlertValues,
    setUpdatedAlertValues,
  ] = useState<AlertConfigurationObject>(alertValues || initialNewAlert);
  const { product } = useAppContext();

  useEffect(() => {
    if (updatedAlertValues) {
      setSaveDisabled(checkRequiredAlertValues(updatedAlertValues, product));
    }
  }, [updatedAlertValues]);

  const handleFieldChange = useCallback((
    field: string,
    updatedValue: any,
    commaSeparatedList?: boolean,
  ) => {
    let valueToPass = updatedValue;
    if (commaSeparatedList) {
      valueToPass = updatedValue.split(',');
    }

    setUpdatedAlertValues((prevState) => {
      setAlertFormValues({
        ...prevState, [field]: valueToPass,
      });
      return ({
        ...prevState, [field]: valueToPass,
      });
    });
  }, [setAlertFormValues]);

  const contextValue: AlertFormContextType = useMemo(() => ({
    handleFieldChange,
    alertValues: updatedAlertValues,
  }), [handleFieldChange, updatedAlertValues]);

  return (
    <AlertFormContext.Provider value={contextValue}>
      <Box sx={dataTableBox}>
        <AlertInfo />
        { product === FLEET_MANAGER
          && <Scope />}
        <RequiredResponseTime />
        <Templates />
        <Aliases />
        <RuleLogic />
      </Box>
    </AlertFormContext.Provider>
  );
};

export default AlertForm;
