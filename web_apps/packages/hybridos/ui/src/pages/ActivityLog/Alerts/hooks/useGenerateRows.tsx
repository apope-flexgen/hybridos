import {
  Box, Chip, ColorType, Countdown, Typography,
} from '@flexgen/storybook';
import dayjs from 'dayjs';
import advancedFormat from 'dayjs/plugin/advancedFormat';
import timezone from 'dayjs/plugin/timezone';

import { useState } from 'react';
import ResolveAlertButton from 'src/pages/ActivityLog/Alerts/ResolveAlertButton/ResolveAlertButton';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';
import { expandedRowBoxSx, expandedRowContentSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { ActiveAlertObject, ActiveAlertRow } from 'src/pages/ActivityLog/activityLog.types';

const useGenerateActiveAlertRows = () => {
  const [results, setResults] = useState<ActiveAlertRow[]>([]);
  dayjs.extend(timezone);
  dayjs.extend(advancedFormat);

  const generateSeverityComponent = (severity: number) => <SeverityIndicator severity={severity} />;

  const generateStatusBadge = (status: string) => {
    if (status.toLowerCase() === 'active') {
      return (
        <Chip
          label="Active"
          variant="filled"
          color="secondary"
          size="small"
          borderStyle="rounded"
        />
      );
    }
    return (
      <Chip label="Inactive" variant="filled" color="default" size="small" borderStyle="rounded" />
    );
  };

  const generateDeadline = (triggerTime: string, deadline: number) => {
    const endTime = dayjs(triggerTime).add(deadline, 'minutes');
    return <Countdown endTime={endTime} showSeconds negativeCount onNegativeCountColor="#BA1A1A" />;
  };

  const generateResolveButton = (alert: ActiveAlertObject) => (
    <ResolveAlertButton alertInfo={alert} />
  );

  const generateExpandRowContent = (
    instances: { message: string; timestamp: string }[],
    alertTitle: string,
  ) => (
    <Box sx={expandedRowBoxSx}>
      {instances
        .sort((a, b) => dayjs(b.timestamp).diff(a.timestamp))
        .map((instance) => (
          <Box sx={expandedRowContentSx}>
            <Typography
              text={dayjs(instance.timestamp).format('YYYY-MM-DD HH:mm:ss z')}
              variant="bodySBold"
            />
            <Box sx={{ display: 'flex', gap: '2px' }}>
              <Typography text={alertTitle ? `${alertTitle} -` : ''} variant="bodySBold" />
              <Typography text={instance.message} variant="bodyS" />
            </Box>
          </Box>
        ))}
    </Box>
  );

  const rowHoverColorMapping: { [key: number]: ColorType | 'inherit' | 'danger' } = {
    0: 'info',
    1: 'primary',
    2: 'warning',
    3: 'danger',
  };

  const generateRowsData = (alertsData: ActiveAlertObject[]) => {
    const returnData: ActiveAlertRow[] = alertsData
      .filter((alert) => !alert.resolved)
      .map((alert) => ({
        id: alert.id,
        status: generateStatusBadge(alert.status),
        severity: generateSeverityComponent(alert.severity),
        organization: alert.organization,
        alert: alert.details[alert.details.length - 1].message,
        timestamp: dayjs(alert.trigger_time).format('YYYY-MM-DD HH:mm:ss z'),
        deadline: generateDeadline(alert.trigger_time, alert.deadline),
        resolve: generateResolveButton(alert),
        expandRowContent: generateExpandRowContent(alert.details, alert.title),
        rowHoverColor: rowHoverColorMapping[alert.severity],
      }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateActiveAlertRows;
