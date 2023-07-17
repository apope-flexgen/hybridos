import { Typography, Box, Switch } from '@flexgen/storybook';
import { FC } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import { siteAdminLabels } from 'src/pages/SiteAdmin/SiteAdmin.constants';

export interface MultifactorProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const Multifactor: FC<MultifactorProps> = ({
  passwordSettings,
  setPasswordSettings,
}: MultifactorProps) => (
  <>
    <Typography text={siteAdminLabels.mfaField} variant="bodyL" />
    <Box sx={{ display: 'flex', gap: '16px', width: '60%' }}>
      <Box sx={{ display: 'flex' }}>
        <Switch
          autoLayout
          color="primary"
          id="enable_mfa"
          label={siteAdminLabels.mfaSwitch}
          labelPlacement="right"
          onChange={(value) => {
            if (value !== undefined) {
              setPasswordSettings({
                ...passwordSettings,
                multi_factor_authentication: value,
              });
            }
          }}
          value={passwordSettings.multi_factor_authentication}
        />
      </Box>
    </Box>
  </>
);

export default Multifactor;
