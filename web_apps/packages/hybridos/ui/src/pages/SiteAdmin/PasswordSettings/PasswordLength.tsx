import { Typography, Box, Slider } from '@flexgen/storybook';
import { FunctionComponent } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import {
  AbsoluteMaxPasswordLength,
  AbsoluteMinPasswordLength,
  siteAdminLabels,
} from 'src/pages/SiteAdmin/SiteAdmin.constants';

export interface PasswordLengthProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const PasswordLength: FunctionComponent<PasswordLengthProps> = ({
  passwordSettings,
  setPasswordSettings,
}: PasswordLengthProps) => {
  const handleChange = (event: Event, newValue: number | number[]) => {
    setPasswordSettings({
      ...passwordSettings,
      minimum_password_length: (newValue as number[])[0],
      maximum_password_length: (newValue as number[])[1],
    });
  };

  return (
    <>
      <Typography text={siteAdminLabels.passwordLengthField} variant="bodyL" />
      <Box
        sx={{
          display: 'flex',
          gap: '16px',
          alignItems: 'center',
          justifyContent: 'flex-start',
          width: '60%',
        }}
      >
        <Box sx={{ width: '60%' }}>
          <Slider
            min={8}
            max={128}
            valueLabelDisplay="on"
            value={[
              passwordSettings.minimum_password_length,
              passwordSettings.maximum_password_length,
            ]}
            onChange={handleChange}
            marks={[
              { value: AbsoluteMinPasswordLength, label: AbsoluteMinPasswordLength.toString() },
              { value: AbsoluteMaxPasswordLength, label: AbsoluteMaxPasswordLength.toString() },
            ]}
          />
        </Box>
      </Box>
    </>
  );
};

export default PasswordLength;
