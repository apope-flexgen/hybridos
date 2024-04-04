import {
  Box, Chip, IconButton, Switch, Tooltip,
} from '@flexgen/storybook';

import { useState } from 'react';
import { actionsBoxSx, sitesBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';
import { AlertConfigurationObject, AlertManagementRow } from 'src/pages/ActivityLog/activityLog.types';

const useGenerateAlertManagementRows = (
  disableAlert: (value: boolean, alert: AlertConfigurationObject) => void,
  editAlert: (alert: AlertConfigurationObject) => void,
  duplicateAlert: (alert: AlertConfigurationObject) => void,
) => {
  const [results, setResults] = useState<AlertManagementRow[]>([]);
  const generateSeverityComponent = (severity: number) => <SeverityIndicator severity={severity} />;

  const generateSitesComponent = (sites: string[]) => (
    <Box sx={sitesBoxSx}>
      {
        sites.map((site) => <Chip size="small" borderStyle="rounded" variant="filled" color="primary" label={site} />)
      }
    </Box>
  );

  const generateDeliverToggle = (deliver: boolean, alert: AlertConfigurationObject) => (
    <Switch
      value={deliver}
      onChange={(value?: boolean) => disableAlert(value || false, alert)}
    />
  );

  const generateActions = (alert: AlertConfigurationObject) => (
    <Box sx={actionsBoxSx}>
      <Tooltip placement="bottom-end" arrow title="Duplicate Alert">
        <IconButton icon="ContentCopy" onClick={() => duplicateAlert(alert)} />
      </Tooltip>
      <Tooltip placement="bottom-end" arrow title="Edit Alert">
        <IconButton icon="Edit" onClick={() => editAlert(alert)} />
      </Tooltip>
    </Box>
  );

  const generateRowsData = (
    alertConfigurationData: AlertConfigurationObject[],
  ) => {
    const returnData: AlertManagementRow[] = alertConfigurationData.map((alert) => ({
      id: alert.id || '',
      deliver: generateDeliverToggle(alert.enabled, alert),
      name: alert.title,
      organization: <Chip size="small" borderStyle="rounded" variant="filled" color="primary" label={alert.organization} />,
      sites: generateSitesComponent(alert.sites),
      severity: generateSeverityComponent(alert.severity),
      deadline: `${alert.deadline} minutes`,
      last_triggered: alert.last_trigger_time || '',
      actions: generateActions(alert),
    }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateAlertManagementRows;
