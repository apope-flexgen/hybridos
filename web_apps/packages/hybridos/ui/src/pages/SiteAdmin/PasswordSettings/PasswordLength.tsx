import {
  Typography, Box, NumericInput, NumericStepper,
} from '@flexgen/storybook';
import { FunctionComponent } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import { siteAdminLabels, onArrow } from 'src/pages/SiteAdmin/SiteAdmin.constants';
import { numericInputSx } from './styles';

export interface PasswordLengthProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const PasswordLength: FunctionComponent<PasswordLengthProps> = ({
  passwordSettings,
  setPasswordSettings,
}: PasswordLengthProps) => {
  const MIN_VALUE = 8;
  const MAX_VALUE = 128;

  const handleChange = (e: React.ChangeEvent<HTMLTextAreaElement | HTMLInputElement>) => {
    const {
      target: { value, id },
    } = e;
    setPasswordSettings({
      ...passwordSettings,
      [id]: Number(value),
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
        <Box sx={numericInputSx}>
          <NumericInput
            color={
              passwordSettings.maximum_password_length < passwordSettings.minimum_password_length
                ? 'error'
                : 'primary'
            }
            helperText={
              passwordSettings.maximum_password_length < passwordSettings.minimum_password_length
                ? 'Min must be less than max'
                : ''
            }
            endComponentAdorment={(
              <NumericStepper
                downArrowOnClick={() => onArrow('down', passwordSettings, setPasswordSettings, 'minimum_password_length')}
                upArrowOnClick={() => onArrow('up', passwordSettings, setPasswordSettings, 'minimum_password_length')}
                disabledDownArrow={passwordSettings.minimum_password_length <= MIN_VALUE}
                disabledUpArrow={
                  passwordSettings.minimum_password_length
                  >= passwordSettings.maximum_password_length
                }
              />
            )}
            label="Min"
            onChange={handleChange}
            validationRegEx="positiveIntegers"
            value={passwordSettings.minimum_password_length.toString()}
            id="minimum_password_length"
          />
          to
          <NumericInput
            endComponentAdorment={(
              <NumericStepper
                downArrowOnClick={() => onArrow('down', passwordSettings, setPasswordSettings, 'maximum_password_length')}
                upArrowOnClick={() => onArrow('up', passwordSettings, setPasswordSettings, 'maximum_password_length')}
                disabledDownArrow={
                  passwordSettings.maximum_password_length
                  <= passwordSettings.minimum_password_length
                }
                disabledUpArrow={passwordSettings.maximum_password_length >= MAX_VALUE}
              />
            )}
            label="Max"
            color={
              passwordSettings.maximum_password_length < passwordSettings.minimum_password_length
                ? 'error'
                : 'primary'
            }
            helperText={
              passwordSettings.maximum_password_length < passwordSettings.minimum_password_length
                ? 'Max must be greater than min'
                : ''
            }
            onChange={handleChange}
            validationRegEx="positiveIntegers"
            value={passwordSettings.maximum_password_length.toString()}
            id="maximum_password_length"
          />
        </Box>
      </Box>
    </>
  );
};

export default PasswordLength;
