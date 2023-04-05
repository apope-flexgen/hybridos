// TODO: fix lint
/* eslint-disable no-prototype-builtins */
import { Box, MuiButton } from '@flexgen/storybook';
import { FunctionComponent, useContext, useState } from 'react';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { initialPasswordRequirements } from './UserAdmin.constants';
import UserRow from './UserRow';

export interface AddUserRowProps {
  updateUserData: () => void
  setIsLoading: (state: boolean) => void
  setShowAddUser: (state: any) => void
}

const AddUserRow: FunctionComponent<AddUserRowProps> = ({
  updateUserData,
  setIsLoading,
  setShowAddUser,
}: AddUserRowProps) => {
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const [userData, setUserData] = useState<any>({});

  const [validPassword, setValidPassword] = useState<boolean>(false);

  const checkPassword = (): boolean => validPassword;

  const validInput = (data: any): boolean => {
    if (!checkPassword()) return false;

    return (
      data.hasOwnProperty('username')
            && data.hasOwnProperty('password')
            && data.hasOwnProperty('role')
    );
  };

  const addUser = async () => {
    if (!validInput(userData)) {
      notifCtx?.notif('error', 'Error creating user: invalid input');
      return;
    }
    try {
      setIsLoading(true);
      await axiosInstance.post('/users', userData);
      notifCtx?.notif('success', 'Successfully created User');
      updateUserData();
      setShowAddUser(false);
    } catch (axiosError: any) {
      notifCtx?.notif(
        'error',
        `Error creating user: ${axiosError.response.data.message}`,
      );
    } finally {
      setIsLoading(false);
    }
  };

  const cancel = () => {
    setUserData({});
    setShowAddUser(false);
  };

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        gap: '12px',
        paddingY: '12px',
        width: '100%',
        flexGrow: 1,
      }}
    >
      <UserRow
        passwordRequirements={initialPasswordRequirements}
        setUserData={setUserData}
        setValidPassword={setValidPassword}
        userData={userData}
      />
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'row',
          alignItems: 'left',
          gap: '12px',
          width: '100%',
          flexGrow: 1,
        }}
      >
        <MuiButton label="add user" onClick={addUser} variant="contained" />
        <MuiButton label="cancel" onClick={cancel} variant="text" />
      </Box>
    </Box>
  );
};

export default AddUserRow;
