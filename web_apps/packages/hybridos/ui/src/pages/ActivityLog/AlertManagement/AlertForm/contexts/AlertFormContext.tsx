import { createContext, useContext } from 'react';
import { initialNewAlert } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';

export type AlertFormContextType = {
  alertValues: AlertConfigurationObject;
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void;
};

export const AlertFormContext = createContext<AlertFormContextType>({
  alertValues: initialNewAlert,
  handleFieldChange: () => null,
});

export function useAlertFormContext() {
  return useContext(AlertFormContext);
}

export default AlertFormContext;
