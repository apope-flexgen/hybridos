// TODO: fix lint
/* eslint-disable max-lines */
/* eslint-disable react/no-children-prop */
import {
  CardRow,
  TextField,
  Typography,
  MuiButton,
  Box,
  Badge,
  CardContainer,
  ThemeType,
} from '@flexgen/storybook';
import { FunctionComponent } from 'react';
import { RadiusSettings } from 'shared/types/api/SiteAdmin.types';
import { useTheme } from 'styled-components';
import { siteAdminLabels } from './SiteAdmin.constants';
import {
  radiusCardsContainerSx,
  radiusSettingsContainerSx,
  radiusTestRequiredBadgeSx,
  radiusTtitleSx,
} from './styles';

export interface RadiusSettingsProps {
  radiusSettings: RadiusSettings;
  setRadiusSettings: React.Dispatch<React.SetStateAction<RadiusSettings>>;
  username: string;
  setUsername: React.Dispatch<React.SetStateAction<string>>;
  password: string;
  setPassword: React.Dispatch<React.SetStateAction<string>>;
  sendRadiusTest: () => void;
  isRadiusTestLoading: boolean;
  radiusTestSuccessful: boolean;
}

const RadiusSettingsFields: FunctionComponent<RadiusSettingsProps> = ({
  radiusSettings,
  setRadiusSettings,
  username,
  setUsername,
  password,
  setPassword,
  sendRadiusTest,
  isRadiusTestLoading,
  radiusTestSuccessful,
}) => {
  const theme = useTheme() as ThemeType;

  return (
    <>
      <CardRow alignItems="center" justifyContent="flex-start">
        <Typography text={siteAdminLabels.radiusPageTitle} variant="headingS" />
      </CardRow>
      <CardRow alignItems="flex-start" justifyContent="space-between" width="100%">
        <Typography text={siteAdminLabels.radiusPageSubtitle} variant="bodyL" />
        <Box sx={radiusSettingsContainerSx(theme)}>
          <Box sx={radiusTtitleSx}>
            <Typography text={siteAdminLabels.radiusPageCardTitle} variant="bodyLBold" />
            <Box sx={radiusTestRequiredBadgeSx}>
              <Badge
                anchorOrigin={{ horizontal: 'right', vertical: 'top' }}
                badgeColor={!radiusTestSuccessful ? 'error' : 'success'}
                badgeContent={
                  !radiusTestSuccessful
                    ? siteAdminLabels.radiusTestBadgeError
                    : siteAdminLabels.radiusTestBadgeSuccess
                }
                children={<Box />}
              />
            </Box>
          </Box>
          <Box sx={radiusCardsContainerSx}>
            <CardContainer
              direction="column"
              styleOverrides={{ padding: '16px', gap: '16px', boxSizing: 'border-box' }}
            >
              <Typography text={siteAdminLabels.radiusRequirementsTitle} variant="bodyMBold" />
              <Box sx={{ display: 'flex', gap: '16px' }}>
                <TextField
                  fullWidth
                  label={siteAdminLabels.ipAddressField}
                  onChange={(e) => setRadiusSettings({
                    ...radiusSettings,
                    ip_address: e.target.value,
                  })}
                  value={radiusSettings.ip_address}
                />
                <TextField
                  fullWidth
                  label={siteAdminLabels.portField}
                  onChange={(e) => setRadiusSettings({ ...radiusSettings, port: e.target.value })}
                  value={radiusSettings.port}
                />
                <TextField
                  TextAdornment="ms"
                  adornment="end"
                  fullWidth
                  label={siteAdminLabels.waitTimeField}
                  onChange={(e) => setRadiusSettings({
                    ...radiusSettings,
                    wait_time: Number(e.target.value),
                  })}
                  value={radiusSettings.wait_time.toString()}
                />
              </Box>
              <Box sx={{ display: 'flex', gap: '16px', width: '100%' }}>
                <TextField
                  adornment="end"
                  fullWidth
                  label={siteAdminLabels.secretKeyField}
                  onChange={(e) => setRadiusSettings({
                    ...radiusSettings,
                    secret_phrase: e.target.value,
                  })}
                  type="password"
                  value={radiusSettings.secret_phrase}
                />
              </Box>
            </CardContainer>
            <CardContainer
              direction="column"
              styleOverrides={{ padding: '16px', boxSizing: 'border-box', gap: '16px' }}
            >
              <Box sx={{ display: 'flex', flexDirection: 'column' }}>
                <Typography text={siteAdminLabels.radiusTestConnectionTitle} variant="bodyMBold" />
                <Typography text={siteAdminLabels.radiusTestConnectionHelper} variant="bodyS" />
              </Box>
              <Box sx={{ display: 'flex', gap: '16px' }}>
                <TextField
                  fullWidth
                  label={siteAdminLabels.usernameField}
                  onChange={(e) => {
                    setUsername(e.target.value);
                  }}
                  value={username}
                />
                <TextField
                  adornment="end"
                  fullWidth
                  label={siteAdminLabels.passwordField}
                  onChange={(e) => setPassword(e.target.value)}
                  type="password"
                  value={password}
                />
              </Box>
            </CardContainer>
            <Box
              sx={{
                display: 'flex',
                gap: '16px',
                justifyContent: 'flex-start',
              }}
            >
              <MuiButton
                label={siteAdminLabels.radiusTestButton}
                loading={isRadiusTestLoading}
                onClick={sendRadiusTest}
              />
            </Box>
          </Box>
        </Box>
      </CardRow>
    </>
  );
};

export default RadiusSettingsFields;
