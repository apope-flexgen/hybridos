import {
  Box, Chip, IconButton, Switch, Tooltip,
} from '@flexgen/storybook';
import dayjs from 'dayjs';
import advancedFormat from 'dayjs/plugin/advancedFormat';
import timezone from 'dayjs/plugin/timezone';

import { useState } from 'react';
import { actionsBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';
import { AlertConfigurationObject, AlertManagementRow } from 'src/pages/ActivityLog/activityLog.types';

const useGenerateAlertManagementRows = (
  disableAlert: (value: boolean, alert: AlertConfigurationObject) => void,
  editAlert: (alert: AlertConfigurationObject) => void,
  duplicateAlert: (alert: AlertConfigurationObject) => void,
) => {
  const [results, setResults] = useState<AlertManagementRow[]>([]);
  dayjs.extend(timezone);
  dayjs.extend(advancedFormat);

  const generateSeverityComponent = (
    severity: number | string,
  ) => <SeverityIndicator severity={Number(severity)} />;

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
        <IconButton icon="EditOutlined" onClick={() => editAlert(alert)} />
      </Tooltip>
    </Box>
  );

  const generateRowsData = (
    alertConfigurationData: AlertConfigurationObject[],
  ) => {
    const returnData: AlertManagementRow[] = alertConfigurationData.map((alert) => ({
      id: alert.id || '',
      deliver: alert.enabled !== undefined ? generateDeliverToggle(alert.enabled, alert) : '-',
      name: alert.title ? alert.title : '-',
      organization: alert.organization ? <Chip size="small" borderStyle="rounded" variant="filled" color="primary" label={alert.organization} /> : '-',
      severity: alert.severity !== undefined ? generateSeverityComponent(alert.severity) : '-',
      deadline: alert.deadline ? `${alert.deadline.value} ${alert.deadline.unit}${Number(alert.deadline.value) > 1 ? 's' : ''}` : '-',
      last_triggered: alert.last_trigger_time ? dayjs(alert.last_trigger_time).format('YYYY-MM-DD HH:mm:ss z') : '',
      actions: generateActions(alert),
    }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateAlertManagementRows;
