import { Box, MuiButton } from '@flexgen/storybook';
import { FunctionComponent, useContext, useState } from 'react';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { changingPasswordRequirements } from './UserAdmin.constants';
import UserRow from './UserRow';

export interface RowContentsProps {
  user: any
  updateUserData: () => void
  setIsLoading: (state: boolean) => void
}

const EditUserRow: FunctionComponent<RowContentsProps> = ({
  user,
  updateUserData,
  setIsLoading,
}: RowContentsProps) => {
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const [userData, setUserData] = useState<any>({});

  const [validPassword, setValidPassword] = useState<boolean>(false);

  const checkValidity = () => {
    if (!validPassword) {
      notifCtx?.notif('error', 'Error: Password is invalid');
      return false;
    }

    return true;
  };

  const deleteUser = async () => {
    if (!checkValidity) return;

    try {
      setIsLoading(true);
      await axiosInstance.delete(`/users/${user.id}`);
      notifCtx?.notif('success', 'Successfully deleted User');
      updateUserData();
    } catch (axiosError: any) {
      notifCtx?.notif(
        'error',
        `Error deleting user: ${axiosError.response.data.message}`,
      );
    } finally {
      setIsLoading(false);
    }
  };

  const updateUser = () => {
    if (!checkValidity) return;

    if (Object.keys(userData).length === 0) {
      notifCtx?.notif('error', 'Error updating user: must modify a field');
      return;
    }

    setIsLoading(true);
    axiosInstance
      .put(`/users/${user.id}`, userData)
      .then(
        () => {
          notifCtx?.notif('success', 'Successfully updated User');
          updateUserData();
        },
        (axiosError) => {
          notifCtx?.notif(
            'error',
            `Error updating user: ${axiosError.response.data.message}`,
          );
        },
      )
      .then(() => setIsLoading(false));
  };

  const cancel = () => setUserData({});

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        padding: '12px 12px 12px 3%',
        gap: '12px',
        width: '100%',
        flexGrow: 1,
      }}
    >
      <UserRow
        existingUser={user}
        passwordRequirements={changingPasswordRequirements}
        setUserData={setUserData}
        setValidPassword={setValidPassword}
        userData={userData}
      />
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'row',
          alignItems: 'center',
          marginRight: 'auto',
          width: '100%',
          gap: '12px',
          flexGrow: 1,
        }}
      >
        <MuiButton label="update user" onClick={updateUser} variant="contained" />
        <MuiButton label="cancel" onClick={cancel} variant="text" />
        <MuiButton
          color="error"
          label="delete user"
          onClick={deleteUser}
          sx={{ marginLeft: 'auto' }}
          variant="outlined"
        />
      </Box>
    </Box>
  );
};

export default EditUserRow;
