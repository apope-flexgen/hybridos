import {
  Box,
  Chip, ColorType, Countdown, MuiButton, Typography,
} from '@flexgen/storybook';
import dayjs from 'dayjs';

import { useState } from 'react';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';

import { expandedRowBoxSx, expandedRowContentSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { ActiveAlertObject, ActiveAlertRow } from 'src/pages/ActivityLog/activityLog.types';

const useGenerateRows = () => {
  const [results, setResults] = useState<ActiveAlertRow[]>([]);

  const generateSeverityComponent = (severity: number) => <SeverityIndicator severity={severity} />;

  const generateStatusBadge = (
    status: string,
  ) => {
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
      <Chip
        label="Inactive"
        variant="filled"
        color="default"
        size="small"
        borderStyle="rounded"
      />
    );
  };

  const generateDeadline = (
    triggerTime: string,
    deadline: number,
  ) => {
    const endTime = dayjs(triggerTime).add(deadline, 'minutes');
    return (
      <Countdown
        endTime={endTime}
        showSeconds
        negativeCount
        onNegativeCountColor="#BA1A1A"
      />
    );
  };

  const generateResolveButton = (
    id: string,
  ) => <MuiButton label="Resolve" variant="text" onClick={() => { console.log(id); }} />;

  const generateExpandRowContent = (
    instances: { message: string, timestamp: string }[],
    alertTitle: string,
  ) => (
    <Box sx={expandedRowBoxSx}>
      {
          instances.map((instance) => (
            <Box sx={expandedRowContentSx}>
              <Typography text={instance.timestamp} variant="bodySBold" />
              <Box sx={{ display: 'flex', gap: '2px' }}>
                <Typography text={`${alertTitle} -`} variant="bodySBold" />
                <Typography text={instance.message} variant="bodyS" />
              </Box>
            </Box>
          ))
        }
    </Box>
  );

  const rowHoverColorMapping: { [key: number]: ColorType | 'inherit' | 'danger' } = {
    0: 'info',
    1: 'primary',
    2: 'warning',
    3: 'danger',
  };

  const generateRowsData = (
    alertsData: ActiveAlertObject[],
  ) => {
    const returnData: ActiveAlertRow[] = [];
    alertsData.forEach((alert) => {
      returnData.push({
        id: alert.id,
        status: generateStatusBadge(alert.status),
        severity: generateSeverityComponent(alert.severity),
        organization: alert.organization,
        site: alert.site,
        alert: alert.details[0].message,
        timestamp: alert.trigger_time,
        deadline: generateDeadline(alert.trigger_time, alert.deadline),
        resolve: generateResolveButton(alert.id),
        expandRowContent: generateExpandRowContent(alert.details, alert.title),
        rowHoverColor: rowHoverColorMapping[alert.severity],
      });
    });

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateRows;
