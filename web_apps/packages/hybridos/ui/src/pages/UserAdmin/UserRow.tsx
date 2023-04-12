import { Box, Select, TextField, Typography } from '@flexgen/storybook';
import { FunctionComponent, useCallback, useEffect, useState } from 'react';
import { validatePassword } from 'shared/functions/passwordValidation';
import { Roles } from 'shared/types/api/Users/Users.types';
import { PasswordOptions } from 'shared/types/api/Users/Users.types'
import { generatePasswordRequirementsToDisplay } from 'src/pages/UserAdmin/UserAdmin.constants'
import { passwordReqsSx, rowSx } from 'src/pages/UserAdmin/UserAdmin.styles'

export interface UserRowProps {
  userData?: any
  setUserData: (state: any) => void
  existingUser?: any
  setValidPassword: (state: boolean) => void
  showDeveloper?: boolean
  passwordOptions: PasswordOptions
  oldPasswords: number
}

const UserRow: FunctionComponent<UserRowProps> = ({
  userData = {},
  setUserData,
  existingUser = {},
  setValidPassword,
  showDeveloper,
  passwordOptions,
  oldPasswords
}: UserRowProps) => {
  const rolesWithoutDeveloper = Object.values(Roles).filter((role) => {return role !== 'developer'})  

  const [ passwordRequirements, setPasswordRequirements ]  = useState<string[]>([''])

  const generatePasswordRequirements = useCallback(async () => {
    try {
      const newUser: boolean = Object.keys(existingUser).length === 0
      
      const updatedPasswordRequirements = generatePasswordRequirementsToDisplay(passwordOptions, oldPasswords, newUser)

      setPasswordRequirements(updatedPasswordRequirements)
    } finally {
    }
  }, [existingUser, passwordOptions, oldPasswords]);

  useEffect(() => {
    generatePasswordRequirements()
  }, []);

  return (
    <Box sx={rowSx}>
      <Box sx={{display: 'flex', width: '60%', gap: '8px' }}>
      <TextField
        fullWidth
        disableLabelAnimation
        label="Username"
        onChange={(e) => {
          setUserData({ ...userData, username: e.target.value });
        }}
        placeholder={existingUser?.username}
        value={userData?.username ?? ''}
      />
      <Select
        minWidth={175}
        label="Role"
        menuItems={showDeveloper ? Object.values(Roles) : rolesWithoutDeveloper}
        onChange={(e) => {
          setUserData({ ...userData, role: e.target.value });
        }}
        placeholder={existingUser?.role}
        value={userData?.role ?? ''}
      />
      <TextField
        fullWidth
        disableLabelAnimation
        label="Password"
        onChange={(e) => {
          setUserData({ ...userData, password: e.target.value });
        }}
        onValid={setValidPassword}
        type="password"
        validator={userData?.password && (()=>validatePassword(userData?.password, passwordOptions))}
        value={userData?.password ?? ''}
      />
      </Box>
      <Box sx={passwordReqsSx}>
        <Typography text="Password Requirments" variant="bodySBold"/>
        {passwordRequirements.map((requirement) => (
          <Typography 
            text={`- ${requirement}`} 
            variant="helperText"
          />
        ))}
      </Box>
    </Box>
  );
}

export default UserRow;
