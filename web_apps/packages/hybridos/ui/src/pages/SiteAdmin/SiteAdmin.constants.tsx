/* eslint-disable max-lines */
// TODO: fix lint
import { numberValidationEnum } from '@flexgen/storybook';
import { RadiusSettings, PasswordSettings, SiteAdmins } from 'shared/types/api/SiteAdmin.types';

export const initialRadiusSettings: RadiusSettings = {
  _id: '',
  port: '',
  is_enabled: false,
  ip_address: '',
  secret_phrase: '',
  wait_time: 0,
  is_local_auth_disabled: false,
};

export const initialPasswordSettings: PasswordSettings = {
  _id: '',
  password_expiration: false,
  minimum_password_length: 0,
  maximum_password_length: 0,
  password_expiration_interval: '',
  old_passwords: 0,
  multi_factor_authentication: false,
  lowercase: false,
  uppercase: false,
  digit: false,
  special: false,
};

export const initialsiteAdmins: SiteAdmins = {
  _id: '',
  password: initialPasswordSettings,
  radius: initialRadiusSettings,
  __v: 0,
};

export const AbsoluteMinPasswordLength = 8;
export const AbsoluteMaxPasswordLength = 128;

export const PasswordContentRequirementsLabels: { [option: string]: string } = {
  lowercase: 'Lowercase (abc)',
  uppercase: 'Uppercase (ABC)',
  digit: 'Number (123)',
  special: 'Special Character (!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~)',
};

export type PasswordExpirationIntervalUnits = 'd' | 'm';

export type ConnectionMethods = 'onlyLocal' | 'onlyRadius' | 'localAndRadius';

export const passwordExpirationIntervalRadios: {
  label: string;
  value: PasswordExpirationIntervalUnits;
}[] = [
  {
    label: 'Minutes',
    value: 'm',
  },
  {
    label: 'Days',
    value: 'd',
  },
];

export const APP_SETTINGS_URL = '/site-admins';
export const RADIUS_TEST_URL = '/site-admins/radius-test';
export const RADIUS_FAILED_MESSAGE = 'Radius test failed';

export const onArrow = (
  direction: 'up' | 'down',
  passwordSettings: PasswordSettings,
  setPasswordSettings: React.Dispatch<React.SetStateAction<PasswordSettings>>,
  field: keyof PasswordSettings,
) => {
  const newValue = direction === 'up' ? Number(passwordSettings[field]) + 1 : Number(passwordSettings[field]) - 1;
  const regEx = new RegExp(numberValidationEnum.positiveIntegers);
  if (regEx.test(`${newValue}`)) {
    setPasswordSettings({
      ...passwordSettings,
      [field]: newValue,
    });
  }
};

export const connectionMethodRadios: { label: string; value: ConnectionMethods }[] = [
  {
    label: 'Local Authentication Only',
    value: 'onlyLocal',
  },
  {
    label: 'Local and Radius Authentication',
    value: 'localAndRadius',
  },
  {
    label: 'Radius Authentication Only',
    value: 'onlyRadius',
  },
];

export const siteAdminLabels = {
  siteAdminPageTitle: 'Site Admin',
  needSuccessfulTestErrorMessage: 'Radius Authentication requires a successful test request',
  authenticationMethodTitle: 'Authentication Method',
  localPageTitle: 'Password Settings',
  passwordLengthField: 'Password Length',
  maxLessThanMinErrorMessage: 'Max length must be greater than min',
  passwordRequirements: 'Password Content Requirements',
  oldPasswordsField: 'Old Passwords to Store',
  oldPasswordsFieldHelper: 'Stored passwords are not repeatable',
  disableOldPasswordsFieldHelper: 'To disable password expiration, set this field to 0',
  oldPasswordsTextFieldHelper: 'Passwords',
  mfaField: 'Multifactor Authentication',
  mfaSwitch: 'Enable Multifactor Authentication',
  passwordExpirationField: 'Password Expiration',
  passwordExpirationSwitch: 'Enable Password Expiration',
  passwordExpirationInterval: 'Require reset every',
  radiusPageTitle: 'Radius Authentication Settings',
  radiusPageSubtitle: 'Radius Credentials',
  radiusPageCardTitle: 'Radius Connection',
  radiusTestButton: 'Send Test Request',
  radiusRequirementsTitle: 'Radius Credentials',
  radiusTestConnectionTitle: 'Personal Credentials',
  radiusTestConnectionHelper:
    'Required to process test connection. Personal credentials will not be saved.',
  radiusTestBadgeError: 'Test Required',
  radiusTestBadgeSuccess: 'Test Successful',
  ipAddressField: 'IP Address',
  portField: 'Port',
  secretKeyField: 'Secret Key',
  waitTimeField: 'Wait Time',
  usernameField: 'Username',
  passwordField: 'Password',
};
