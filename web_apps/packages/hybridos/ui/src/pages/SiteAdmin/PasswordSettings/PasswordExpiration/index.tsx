/* eslint-disable max-lines */
import { Typography, Box, Switch } from '@flexgen/storybook';
import { FC } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import { siteAdminLabels } from 'src/pages/SiteAdmin/SiteAdmin.constants';
import PasswordExpirationExpandedContent from './ExpandedContent';

export interface PasswordExpirationProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const PasswordExpiration: FC<PasswordExpirationProps> = ({
  passwordSettings,
  setPasswordSettings,
}: PasswordExpirationProps) => {
  const expirationSwitchOnChange = (value: boolean | undefined) => {
    if (value !== undefined) {
      setPasswordSettings({
        ...passwordSettings,
        password_expiration: value,
      });
    }
  };

  return (
    <>
      <Typography text={siteAdminLabels.passwordExpirationField} variant="bodyL" />
      <Box sx={{ display: 'flex', width: '60%' }}>
        <Box sx={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <Box sx={{ display: 'flex' }}>
            <Switch
              color="primary"
              id="enable_password_expiration"
              label={siteAdminLabels.passwordExpirationSwitch}
              labelPlacement="right"
              onChange={expirationSwitchOnChange}
              value={passwordSettings.password_expiration}
            />
          </Box>
          {passwordSettings.password_expiration && (
            <Box sx={{ display: 'flex', gap: '16px' }}>
              <PasswordExpirationExpandedContent
                passwordSettings={passwordSettings}
                setPasswordSettings={setPasswordSettings}
              />
            </Box>
          )}
        </Box>
      </Box>
    </>
  );
};

export default PasswordExpiration;
