import {
  Box, Icon, ThemeType, Typography,
} from '@flexgen/storybook';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  alertBannerIconSx,
  alertBannerMessageSx,
  resolveActiveAlertBannerSx,
} from 'src/pages/ActivityLog/Alerts/alerts.styles';

import { useTheme } from 'styled-components';

const DeleteOrganizationsBanner: React.FC = () => {
  const theme = useTheme() as ThemeType;
  const bannerSx = resolveActiveAlertBannerSx(theme);

  return (
    <Box sx={bannerSx}>
      <Box sx={alertBannerIconSx}>
        <Icon src="FaultOutline" color="warning" />
      </Box>
      <Box sx={alertBannerMessageSx}>
        <Typography text="Note:" variant="headingS" color="warning" />
        <Typography
          text={alertManagementHelperText.deletingOrganizations}
          variant="bodyS"
          color="warning"
        />
      </Box>
    </Box>
  );
};

export default DeleteOrganizationsBanner;
