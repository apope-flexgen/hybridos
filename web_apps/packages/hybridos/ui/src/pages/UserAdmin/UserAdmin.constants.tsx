const pwdRequirements: string[] = [
  '8-1028 characters',
  'Include lowercase, uppercase, number, and special character',
];

export const initialPasswordRequirements: string[] = pwdRequirements;
export const changingPasswordRequirements: string[] = pwdRequirements.concat(
  'Cannot match current or previous 4 passwords',
);
