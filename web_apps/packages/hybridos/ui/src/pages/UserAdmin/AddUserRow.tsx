// TODO: fix lint
/* eslint-disable no-prototype-builtins */
import {
  Box, MuiButton, ThemeType, Typography,
} from '@flexgen/storybook';
import { FunctionComponent, useContext, useState } from 'react';
import { PasswordOptions } from 'shared/types/api/Users/Users.types';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import {
  addUserRowButtonsSx,
  addUserRowSx,
  addUserAndTypographySx,
} from 'src/pages/UserAdmin/UserAdmin.styles';
import UserRow from 'src/pages/UserAdmin/UserRow';
import { useTheme } from 'styled-components';

export interface AddUserRowProps {
  updateUserData: () => void;
  setIsLoading: (state: boolean) => void;
  showDeveloper?: boolean;
  passwordOptions: PasswordOptions;
  oldPasswords: number;
}

const AddUserRow: FunctionComponent<AddUserRowProps> = ({
  updateUserData,
  setIsLoading,
  showDeveloper,
  passwordOptions,
  oldPasswords,
}: AddUserRowProps) => {
  const axiosInstance = useAxiosWebUIInstance(true);
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const [userData, setUserData] = useState<any>({});

  const [validPassword, setValidPassword] = useState<boolean>(false);

  const checkPassword = (): boolean => validPassword;

  const validInput = (
    data: any,
  ): { valid: boolean; field: 'username' | 'password' | 'role' | null } => {
    if (!checkPassword()) return { valid: false, field: 'password' };
    if (!data.hasOwnProperty('username')) return { valid: false, field: 'username' };
    if (!data.hasOwnProperty('password')) return { valid: false, field: 'password' };
    if (!data.hasOwnProperty('role')) return { valid: false, field: 'role' };
    return { valid: true, field: null };
  };

  const addUser = async () => {
    if (!validInput(userData).valid) {
      notifCtx?.notif(
        'error',
        `Error creating user: invalid input for ${validInput(userData).field} field`,
      );
      return;
    }
    try {
      setIsLoading(true);
      await axiosInstance.post('/users', userData);
      notifCtx?.notif('success', 'Successfully created user');
      updateUserData();
      setUserData({});
    } catch (axiosError: any) {
      notifCtx?.notif('error', `Error creating user: ${axiosError.response.data.message}`);
    } finally {
      setIsLoading(false);
    }
  };

  const cancel = () => {
    setUserData({});
  };

  const theme = useTheme() as ThemeType;

  return (
    <Box sx={addUserAndTypographySx}>
      <Typography text="Add User" variant="bodyLBold" sx={{ width: '20%' }} />
      <Box sx={addUserRowSx(theme)}>
        <UserRow
          setUserData={setUserData}
          setValidPassword={setValidPassword}
          showDeveloper={showDeveloper}
          userData={userData}
          passwordOptions={passwordOptions}
          oldPasswords={oldPasswords}
        />
        <Box sx={addUserRowButtonsSx}>
          <MuiButton label="Cancel" onClick={cancel} variant="text" />
          <MuiButton label="Add User" onClick={addUser} variant="contained" />
        </Box>
      </Box>
    </Box>
  );
};

export default AddUserRow;
