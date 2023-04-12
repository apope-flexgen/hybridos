import {
  Typography, Box, NumericInput, NumericStepper, RadioButton,
} from '@flexgen/storybook';
import { FC, useEffect, useState } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import {
  passwordExpirationIntervalRadios,
  PasswordExpirationIntervalUnits,
  siteAdminLabels,
} from 'src/pages/SiteAdmin/SiteAdmin.constants';

export interface PasswordExpirationExpandedContentProps {
  passwordSettings: PasswordSettings;
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>;
}

const PasswordExpirationExpandedContent: FC<PasswordExpirationExpandedContentProps> = ({
  passwordSettings,
  setPasswordSettings,
}: PasswordExpirationExpandedContentProps) => {
  const [passwordExpirationIntervalNum, setPasswordExpirationIntervalNum] = useState<number>(1);
  const [passwordExpirationIntervalUnit, setPasswordExpirationIntervalUnit] = useState<PasswordExpirationIntervalUnits>('d');

  useEffect(() => {
    setPasswordExpirationIntervalUnit(
      passwordSettings.password_expiration_interval.slice(-1) as PasswordExpirationIntervalUnits,
    );
    setPasswordExpirationIntervalNum(
      parseInt(passwordSettings.password_expiration_interval.slice(0, -1), 10),
    );
  }, [passwordSettings]);

  const onClickArrow = (delta: number) => () => {
    const naturalNumbersRegex = /^[1-9]+$/; // TODO: Tech Debt, add this to numberValidationEnum in storybook
    const regEx = new RegExp(naturalNumbersRegex);
    if (regEx.test(`${passwordExpirationIntervalNum + delta}`)) {
      setPasswordExpirationIntervalNum(passwordExpirationIntervalNum + delta);
      setPasswordSettings({
        ...passwordSettings,
        password_expiration_interval: `${
          passwordExpirationIntervalNum + delta
        }${passwordExpirationIntervalUnit}`,
      });
    }
  };
  const onClickDownArrow = onClickArrow(-1);
  const onClickUpArrow = onClickArrow(1);

  const intervalOnChange = (e: React.ChangeEvent<HTMLTextAreaElement | HTMLInputElement>) => {
    setPasswordExpirationIntervalNum(parseInt(e.target.value, 10));
    setPasswordSettings({
      ...passwordSettings,
      password_expiration_interval: `${e.target.value}${passwordExpirationIntervalUnit}`,
    });
  };

  const unitOnChange = (passwordUnit: any) => () => {
    setPasswordExpirationIntervalUnit(passwordUnit.value);
    setPasswordSettings({
      ...passwordSettings,
      password_expiration_interval: `${passwordExpirationIntervalNum}${passwordUnit.value}`,
    });
  };

  return (
    <>
      <Box sx={{ display: 'flex', gap: '16px', alignItems: 'center' }}>
        <Typography text={siteAdminLabels.passwordExpirationInterval} variant="bodyM" />
        <Box sx={{ width: '100px' }}>
          <NumericInput
            endComponentAdorment={
              <NumericStepper downArrowOnClick={onClickDownArrow} upArrowOnClick={onClickUpArrow} />
            }
            id="enable_password_interval"
            label=""
            onChange={intervalOnChange}
            validationRegEx="positiveIntegers"
            value={passwordExpirationIntervalNum.toString()}
          />
        </Box>
      </Box>

      {passwordExpirationIntervalRadios.map((passwordUnit) => (
        <RadioButton
          label={passwordUnit.label}
          labelPlacement="end"
          onChange={unitOnChange(passwordUnit)}
          size="small"
          value={passwordExpirationIntervalUnit === passwordUnit.value}
        />
      ))}
    </>
  );
};

export default PasswordExpirationExpandedContent;
