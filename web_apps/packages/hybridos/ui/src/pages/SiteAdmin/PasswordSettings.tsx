// TODO: fix lint
/* eslint-disable max-lines */
import {
  NumericInput,
  NumericStepper,
  Divider,
  CardRow,
  TextField,
  Switch,
  Typography,
  Box,
  numberValidationEnum,
  RadioButton,
} from '@flexgen/storybook';
import { useState, FunctionComponent, useEffect } from 'react';
import { PasswordSettings } from 'shared/types/api/SiteAdmin.types';
import {
  passwordExpirationIntervalRadios,
  PasswordExpirationIntervalUnits,
  onArrow,
  siteAdminLabels,
} from './SiteAdmin.constants';

export interface PasswordSettingsProps {
  passwordSettings: PasswordSettings
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>
}
const PasswordSettingsFields: FunctionComponent<PasswordSettingsProps> = ({
  passwordSettings,
  setPasswordSettings,
}) => {
  const [passwordExpirationIntervalNum, setPasswordExpirationIntervalNum] = useState<number>(0);
  const [passwordExpirationIntervalUnit, setPasswordExpirationIntervalUnit] = useState<PasswordExpirationIntervalUnits>('d');

  useEffect(() => {
    setPasswordExpirationIntervalUnit(
      passwordSettings.password_expiration_interval.slice(
        -1,
      ) as PasswordExpirationIntervalUnits,
    );
    setPasswordExpirationIntervalNum(
      parseInt(passwordSettings.password_expiration_interval.slice(0, -1), 10),
    );
  }, [passwordSettings]);

  // TODO: fix lint
  // eslint-disable-next-line max-len
  const maxLessThanMin = passwordSettings.maximum_password_length < passwordSettings.minimum_password_length;
  return (
    <>
      <CardRow alignItems="center" justifyContent="flex-start">
        <Typography text={siteAdminLabels.localPageTitle} variant="headingS" />
      </CardRow>
      <CardRow alignItems="center" justifyContent="space-between" width="100%">
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
          <Box sx={{ width: '100px' }}>
            <NumericInput
              color={maxLessThanMin ? 'error' : 'primary'}
              endComponentAdorment={(
                <NumericStepper
                  downArrowOnClick={() => onArrow(
                    'down',
                    passwordSettings,
                    setPasswordSettings,
                    'minimum_password_length',
                  )}
                  upArrowOnClick={() => onArrow(
                    'up',
                    passwordSettings,
                    setPasswordSettings,
                    'minimum_password_length',
                  )}
                />
                              )}
              helperText={
                                maxLessThanMin
                                  ? siteAdminLabels.maxLessThanMinErrorMessage
                                  : undefined
                            }
              label="Min"
              onChange={(e) => setPasswordSettings({
                ...passwordSettings,
                minimum_password_length: parseInt(e.target.value, 10),
              })}
              size="small"
              validationRegEx="positiveIntegers"
              value={passwordSettings.minimum_password_length.toString()}
            />
          </Box>
          <Typography text="to" variant="bodyL" />
          <Box sx={{ width: '100px' }}>
            <NumericInput
              color={maxLessThanMin ? 'error' : 'primary'}
              endComponentAdorment={(
                <NumericStepper
                  downArrowOnClick={() => onArrow(
                    'down',
                    passwordSettings,
                    setPasswordSettings,
                    'maximum_password_length',
                  )}
                  upArrowOnClick={() => onArrow(
                    'up',
                    passwordSettings,
                    setPasswordSettings,
                    'maximum_password_length',
                  )}
                />
                              )}
              helperText={
                                maxLessThanMin
                                  ? siteAdminLabels.maxLessThanMinErrorMessage
                                  : undefined
                            }
              label="Max"
              onChange={(e) => setPasswordSettings({
                ...passwordSettings,
                maximum_password_length: parseInt(e.target.value, 10),
              })}
              validationRegEx="positiveIntegers"
              value={passwordSettings.maximum_password_length.toString()}
            />
          </Box>
        </Box>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Box sx={{ paddingBottom: '16px' }} />
      <CardRow alignItems="center" justifyContent="space-between" width="100%">
        <Typography text={siteAdminLabels.customRegexField} variant="bodyL" />
        <Box sx={{ display: 'flex', gap: '16px', width: '60%' }}>
          <TextField
            label={siteAdminLabels.customRegexField}
            onChange={(e) => setPasswordSettings({
              ...passwordSettings,
              password_regular_expression: e.target.value,
            })}
            value={passwordSettings.password_regular_expression}
          />
        </Box>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Box sx={{ paddingBottom: '16px' }} />
      <CardRow alignItems="center" justifyContent="space-between" width="100%">
        <Typography text={siteAdminLabels.oldPasswordsField} variant="bodyL" />
        <Box sx={{ display: 'flex', gap: '16px', width: '60%' }}>
          <Box sx={{ width: '100px' }}>
            <NumericInput
              endComponentAdorment={(
                <NumericStepper
                  downArrowOnClick={() => onArrow(
                    'down',
                    passwordSettings,
                    setPasswordSettings,
                    'old_passwords',
                  )}
                  upArrowOnClick={() => onArrow(
                    'up',
                    passwordSettings,
                    setPasswordSettings,
                    'old_passwords',
                  )}
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
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Box sx={{ paddingBottom: '16px  ' }} />
      <CardRow alignItems="center" justifyContent="space-between" width="100%">
        <Typography text={siteAdminLabels.mfaField} variant="bodyL" />
        <Box sx={{ display: 'flex', gap: '16px', width: '60%' }}>
          <Box sx={{ display: 'flex' }}>
            <Switch
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
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Box sx={{ paddingBottom: '16px' }} />
      <CardRow alignItems="center" justifyContent="space-between" width="100%">
        <Typography text={siteAdminLabels.passwordExpirationField} variant="bodyL" />
        <Box sx={{ display: 'flex', width: '60%' }}>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
            <Box sx={{ display: 'flex' }}>
              <Switch
                color="primary"
                id="enable_password_expiration"
                label={siteAdminLabels.passwordExpirationSwitch}
                labelPlacement="right"
                onChange={(value) => {
                  if (value !== undefined) {
                    setPasswordSettings({
                      ...passwordSettings,
                      password_expiration: value,
                    });
                  }
                }}
                value={passwordSettings.password_expiration}
              />
            </Box>
            <Box sx={{ display: 'flex', gap: '16px' }}>
              {passwordSettings.password_expiration && (
                <Box sx={{ display: 'flex', gap: '16px', alignItems: 'center' }}>
                  <Typography
                    text={siteAdminLabels.passwordExpirationInterval}
                    variant="bodyM"
                  />
                  <Box sx={{ width: '100px' }}>
                    <NumericInput
                      endComponentAdorment={(
                        <NumericStepper
                          downArrowOnClick={() => {
                            const regEx = new RegExp(
                              numberValidationEnum.positiveIntegers,
                            );
                            if (
                              regEx.test(
                                `${
                                  passwordExpirationIntervalNum
                                                                    - 1
                                }`,
                              )
                            ) {
                              setPasswordExpirationIntervalNum(
                                passwordExpirationIntervalNum - 1,
                              );
                              setPasswordSettings({
                                ...passwordSettings,
                                password_expiration_interval: `${
                                  passwordExpirationIntervalNum
                                                                    - 1
                                }
                                                ${passwordExpirationIntervalUnit}`,
                              });
                            }
                          }}
                          upArrowOnClick={() => {
                            const regEx = new RegExp(
                              numberValidationEnum.positiveIntegers,
                            );
                            if (
                              regEx.test(
                                `${
                                  passwordExpirationIntervalNum
                                                                    + 1
                                }`,
                              )
                            ) {
                              setPasswordExpirationIntervalNum(
                                passwordExpirationIntervalNum + 1,
                              );
                              setPasswordSettings({
                                ...passwordSettings,
                                password_expiration_interval: `${
                                  passwordExpirationIntervalNum
                                                                    + 1
                                }
                                                ${passwordExpirationIntervalUnit}`,
                              });
                            }
                          }}
                        />
                                              )}
                      id="enable_password_interval"
                      label=""
                      onChange={(e) => {
                        setPasswordExpirationIntervalNum(
                          parseInt(e.target.value, 10),
                        );
                        setPasswordSettings({
                          ...passwordSettings,
                          password_expiration_interval: `${e.target.value}${passwordExpirationIntervalUnit}`,
                        });
                      }}
                      validationRegEx="positiveIntegers"
                      value={passwordExpirationIntervalNum.toString()}
                    />
                  </Box>
                </Box>
              )}
              {passwordSettings.password_expiration
                                && passwordExpirationIntervalRadios.map((passwordUnit) => (
                                  <RadioButton
                                    label={passwordUnit.label}
                                    labelPlacement="end"
                                    onChange={() => {
                                      setPasswordExpirationIntervalUnit(passwordUnit.value);
                                      setPasswordSettings({
                                        ...passwordSettings,
                                        password_expiration_interval: `${passwordExpirationIntervalNum}${passwordUnit.value}`,
                                      });
                                    }}
                                    size="small"
                                    value={
                                            passwordExpirationIntervalUnit === passwordUnit.value
                                        }
                                  />
                                ))}
            </Box>
          </Box>
        </Box>
      </CardRow>
      <Divider orientation="horizontal" variant="fullWidth" />
      <Box sx={{ paddingBottom: '24px' }} />
    </>
  );
};

export default PasswordSettingsFields;
