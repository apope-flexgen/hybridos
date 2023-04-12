import {
  Typography, Box, NumericInput, NumericStepper,
} from '@flexgen/storybook';
import { FC } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import { onArrow, siteAdminLabels } from 'src/pages/SiteAdmin/SiteAdmin.constants';

export interface OldPasswordsProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const OldPasswords: FC<OldPasswordsProps> = ({
  passwordSettings,
  setPasswordSettings,
}: OldPasswordsProps) => (
  <>
    <Box sx={{ display: 'flex', flexDirection: 'column' }}>
      <Typography text={siteAdminLabels.oldPasswordsField} variant="bodyL" />
      <Typography
        text={siteAdminLabels.oldPasswordsFieldHelper}
        variant="bodyS"
        color="secondary"
      />
    </Box>
    <Box sx={{ display: 'flex', gap: '16px', width: '60%' }}>
      <Box sx={{ width: '100px' }}>
        <NumericInput
          endComponentAdorment={(
            <NumericStepper
              downArrowOnClick={() => onArrow('down', passwordSettings, setPasswordSettings, 'old_passwords')}
              upArrowOnClick={() => onArrow('up', passwordSettings, setPasswordSettings, 'old_passwords')}
            />
          )}
          label={siteAdminLabels.oldPasswordsTextFieldHelper}
          onChange={(e) => setPasswordSettings({
            ...passwordSettings,
            old_passwords: Number(e.target.value),
          })}
          validationRegEx="positiveIntegers"
          value={passwordSettings.old_passwords.toString()}
        />
      </Box>
    </Box>
  </>
);

export default OldPasswords;
