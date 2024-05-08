import {
  Box,
  CardRow,
  ThemeType,
  Typography,
  Divider,
  PageLoadingIndicator,
} from '@flexgen/storybook';
import { useState } from 'react';
import { headerBoxSx, tableBoxSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { mainContentBoxSx } from 'src/pages/ActivityLog/Events/Styles';
import ResolvedAlertsTable from 'src/pages/ActivityLog/ResolvedAlerts/ResolvedAlertsTable/ResolvedAlertsTable';
import { useTheme } from 'styled-components';

const ResolvedAlerts = () => {
  const [isLoading, setIsLoading] = useState<boolean>(true);

  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);

  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <CardRow justifyContent="space-between" styleOverrides={{ paddingTop: '16px' }}>
          <Typography text="Resolved Alerts" variant="bodyXLBold" />
        </CardRow>
        <Divider variant="fullWidth" orientation="horizontal" />
      </Box>
      <Box sx={tableBoxSx}>
        <ResolvedAlertsTable setIsLoading={setIsLoading} />
      </Box>
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};
export default ResolvedAlerts;
