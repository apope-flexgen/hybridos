/* eslint-disable max-len */
// TODO: fix lint
import { PasswordOptions } from 'shared/types/api/Users/Users.types';

export const generatePasswordRequirementsToDisplay = (
  passwordSettings: PasswordOptions,
  previousPasswords?: number,
  newUser?: boolean,
) => {
  const passwordDisplayRequirements = [];
  const initialRequiredChars = 'Must include one or more of each of the following characters: ';
  let requiredCharacters = initialRequiredChars;

  passwordDisplayRequirements.push(
    `${passwordSettings.passwordMinLength}-${passwordSettings.passwordMaxLength} characters`,
  );
  if (passwordSettings.lowercase) requiredCharacters += ' lowercase, ';
  if (passwordSettings.uppercase) requiredCharacters += ' uppercase, ';
  if (passwordSettings.digit) requiredCharacters += ' digit, ';
  if (passwordSettings.special) requiredCharacters += ' special character ';
  if (requiredCharacters !== initialRequiredChars) passwordDisplayRequirements.push(requiredCharacters);
  if (!newUser && previousPasswords !== 0) {
    passwordDisplayRequirements.push(
      `Cannot match current or previous ${previousPasswords} passwords`,
    );
  }

  return passwordDisplayRequirements;
};

export const userColumns = [
  {
    id: 'user',
    label: 'User',
    minWidth: 70,
  },
  {
    id: 'role',
    label: 'Role',
    minWidth: 100,
  },
  {
    id: 'expandRowButton',
    label: '',
    minWidth: 15,
    align: 'right' as const,
  },
];
