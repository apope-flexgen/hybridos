import { Box, MuiButton } from '@flexgen/storybook';
import { FunctionComponent, useContext, useState } from 'react';
import { PasswordOptions } from 'shared/types/api/Users/Users.types';
import { userRowButtonsSx, userRowSx } from 'src/pages/UserAdmin/UserAdmin.styles';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import UserRow from 'src/pages/UserAdmin/UserRow';

export interface RowContentsProps {
  user: any
  updateUserData: () => void
  setIsLoading: (state: boolean) => void
  showDeveloper?: boolean
  passwordOptions: PasswordOptions
  oldPasswords: number
}

const EditUserRow: FunctionComponent<RowContentsProps> = ({
  user,
  updateUserData,
  setIsLoading,
  showDeveloper,
  passwordOptions,
  oldPasswords
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
    <Box sx={userRowSx}>
      <UserRow
        showDeveloper={showDeveloper}
        existingUser={user}
        setUserData={setUserData}
        setValidPassword={setValidPassword}
        userData={userData}
        passwordOptions={passwordOptions}
        oldPasswords={oldPasswords}
      />
      <Box sx={userRowButtonsSx}>
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
