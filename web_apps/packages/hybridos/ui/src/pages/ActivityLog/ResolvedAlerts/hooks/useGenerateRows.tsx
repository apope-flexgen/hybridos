import { Box, ColorType, Typography } from '@flexgen/storybook';
import dayjs from 'dayjs';
import advancedFormat from 'dayjs/plugin/advancedFormat';
import timezone from 'dayjs/plugin/timezone';
import { useState } from 'react';
import SeverityIndicator from 'src/pages/ActivityLog/Alerts/SeverityIndicator/SeverityIndicator';
import { expandedRowBoxSx, expandedRowContentSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import AlertNotesButton from 'src/pages/ActivityLog/ResolvedAlerts/AlertNotesButton/AlertNotesButton';
import { expandedRowSx } from 'src/pages/ActivityLog/activityLog.styles';
import { ResolvedAlertObject, ResolvedAlertRow } from 'src/pages/ActivityLog/activityLog.types';

const useGenerateRows = () => {
  const [results, setResults] = useState<ResolvedAlertRow[]>([]);
  dayjs.extend(timezone);
  dayjs.extend(advancedFormat);

  const generateSeverityComponent = (severity: number) => <SeverityIndicator severity={severity} />;

  const generateNotesButton = (alert: ResolvedAlertObject) => (
    <AlertNotesButton alertInfo={alert} />
  );

  const generateExpandRowContent = (
    instances: { message: string; timestamp: string }[],
    alertTitle: string,
  ) => (
    <Box sx={expandedRowBoxSx}>
      {instances.map((instance) => (
        <Box sx={expandedRowContentSx}>
          <Typography text={instance.timestamp} variant="bodySBold" />
          <Box sx={expandedRowSx}>
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

  const generateRowsData = (alertsData: ResolvedAlertObject[]) => {
    const returnData: ResolvedAlertRow[] = alertsData.map((alert) => ({
      id: alert.id,
      severity: generateSeverityComponent(alert.severity),
      organization: alert.organization || '-',
      alert: alert.details[0]?.message || '-',
      triggerTime: alert.trigger_time
        ? dayjs(alert.trigger_time).format('YYYY-MM-DD HH:mm:ss z')
        : '-',
      resolutionTime: alert.resolution_time
        ? dayjs(alert.resolution_time).format('YYYY-MM-DD HH:mm:ss z')
        : '-',
      notes: generateNotesButton(alert),
      expandRowContent: generateExpandRowContent(alert.details, alert.title),
      rowHoverColor: rowHoverColorMapping[alert.severity],
    }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateRows;
