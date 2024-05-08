import {
  Box,
  CardRow,
  ThemeType,
  Typography,
  Divider,
  PageLoadingIndicator,
} from '@flexgen/storybook';
import { useState } from 'react';
import AlertsTable from 'src/pages/ActivityLog/Alerts/AlertsTable/AlertsTable';
import { headerBoxSx, tableBoxSx } from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { mainContentBoxSx } from 'src/pages/ActivityLog/Events/Styles';
import { useTheme } from 'styled-components';

export interface AlertsPageProps {
  setTotalActiveAlertCount: React.Dispatch<React.SetStateAction<string>>;
}
const Alerts = ({ setTotalActiveAlertCount }: AlertsPageProps) => {
  const [isLoading, setIsLoading] = useState<boolean>(true);

  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);

  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <CardRow justifyContent="space-between" styleOverrides={{ paddingTop: '16px' }}>
          <Typography text="Alerts" variant="bodyXLBold" />
        </CardRow>
        <Divider variant="fullWidth" orientation="horizontal" />
      </Box>
      <Box sx={tableBoxSx}>
        <AlertsTable
          setIsLoading={setIsLoading}
          setTotalActiveAlertCount={setTotalActiveAlertCount}
        />
      </Box>
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};
export default Alerts;
