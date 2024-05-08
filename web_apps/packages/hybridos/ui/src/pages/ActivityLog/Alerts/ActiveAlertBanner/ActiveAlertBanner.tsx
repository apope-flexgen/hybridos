import {
  Box, Icon, ThemeType, Typography,
} from '@flexgen/storybook';
import {
  alertBannerIconSx,
  alertBannerMessageSx,
  resolveActiveAlertBannerSx,
} from 'src/pages/ActivityLog/Alerts/alerts.styles';
import { resolveActiveAlertBannerMessage } from 'src/pages/ActivityLog/activityLog.constants';
import { useTheme } from 'styled-components';

const ActiveAlertBanner: React.FC = () => {
  const theme = useTheme() as ThemeType;
  const bannerSx = resolveActiveAlertBannerSx(theme);

  return (
    <Box sx={bannerSx}>
      <Box sx={alertBannerIconSx}>
        <Icon src="FaultOutline" color="warning" />
      </Box>
      <Box sx={alertBannerMessageSx}>
        <Typography text="Active Alert" variant="headingS" color="warning" />
        <Typography text={resolveActiveAlertBannerMessage} variant="bodyS" color="warning" />
      </Box>
    </Box>
  );
};

export default ActiveAlertBanner;
