export const SiteAdminsDescriptions = {
  radiusSettings: 'Settings for radius server connection',
  radiusIsEnabled: 'Determines if radius authentication is used',
  radiusIp: 'Ip address of radius server',
  radiusPort: 'Port of radius server',
  radiusSecretPhrase: "Radius server's secret phrase",
  radiusWaitTime: 'Timeout length when connecting to radius server',
  radiusUsername: 'Username credential sent to test radius server',
  radiusPassword: 'Password credential sent to test radius server',
  radiusLocalAuthDisabled: 'Deterimes if local authentication is disabled (only use radius)',
  radiusTestResponse:
    'A response message describing whether or not the test of radius credentials passed or failed',

  passwordSettings: 'Settings for password requirements/expiration',
  passwordExpiration: 'Determines if password expiration is enabled',
  passwordMinLength: 'Minimum password length',
  passwordMaxLength: 'Maximum password length',
  passExpInt:
    'Time until password expires, in format length (numeric) interval (m - minutes, d -days); ex: 8d would mean 8 days',
  passOldPasswords: 'Number of previous passwords to disallow',
  passRegEx: 'Regular expression to create passwords',

  lowercase: 'Determines if a lowercase letter is required in the password',
  uppercase: 'Determines if a uppercase letter is required in the password',
  digit: 'Determines if a digit is required in the password',
  special: 'Determines if a special character is required in the password',

  mfaIsEnabled: 'Determines if multi-factor authentication is used',
};
