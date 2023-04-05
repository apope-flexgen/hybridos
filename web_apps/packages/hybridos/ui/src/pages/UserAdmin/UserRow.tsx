import {
  Box, Select, TextField, Typography,
} from '@flexgen/storybook';
import { FunctionComponent } from 'react';
import { validatePassword } from 'shared/functions/passwordValidation';
import { Roles } from 'shared/types/api/Users/Users.types';

export interface UserRowProps {
  userData?: any
  setUserData: (state: any) => void
  existingUser?: any
  passwordRequirements?: string[]
  setValidPassword: (state: boolean) => void
}

const UserRow: FunctionComponent<UserRowProps> = ({
  userData = {},
  setUserData,
  existingUser = {},
  passwordRequirements = [],
  setValidPassword,
}: UserRowProps) => (
  <Box
    sx={{
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      gap: '12px',
      width: '100%',
      flexGrow: 1,
    }}
  >
    <TextField
      disableLabelAnimation
      label="Username"
      onChange={(e) => {
        setUserData({ ...userData, username: e.target.value });
      }}
      placeholder={existingUser?.username}
      value={userData?.username ?? ''}
    />
    <Select
      label="Role"
      menuItems={Object.values(Roles)}
      onChange={(e) => {
        setUserData({ ...userData, role: e.target.value });
      }}
      placeholder={existingUser?.role}
      value={userData?.role ?? ''}
    />
    <TextField
      disableLabelAnimation
      label="Password"
      onChange={(e) => {
        setUserData({ ...userData, password: e.target.value });
      }}
      onValid={setValidPassword}
      type="password"
      validator={validatePassword}
      value={userData?.password ?? ''}
    />
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'left',
      }}
    >
      {passwordRequirements.map((requirement) => (
        <Typography text={`- ${requirement}`} />
      ))}
    </Box>
  </Box>
);

export default UserRow;
