import { Typography, Box, Checkbox } from '@flexgen/storybook';
import { FC } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import {
  PasswordContentRequirementsLabels,
  siteAdminLabels,
} from 'src/pages/SiteAdmin/SiteAdmin.constants';

export interface PasswordRequirementsProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const PasswordRequirements: FC<PasswordRequirementsProps> = ({
  passwordSettings,
  setPasswordSettings,
}: PasswordRequirementsProps) => (
  <>
    <Typography text={siteAdminLabels.passwordRequirements} variant="bodyL" />
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        width: '60%',
      }}
    >
      {Object.keys(PasswordContentRequirementsLabels).map((requirement: string) => (
        <Checkbox
          value={passwordSettings[requirement as keyof PasswordSettings] as boolean}
          label={PasswordContentRequirementsLabels[requirement]}
          onChange={(event, checked: boolean) => {
            setPasswordSettings({ ...passwordSettings, [requirement]: checked });
          }}
        />
      ))}
    </Box>
  </>
);

export default PasswordRequirements;
