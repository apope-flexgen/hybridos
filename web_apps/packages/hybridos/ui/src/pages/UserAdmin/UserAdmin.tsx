/* eslint-disable */
// TODO: fix lint
import {
  Box,
  CardContainer,
  DataTable,
  IconButton,
  MuiButton,
  PageLoadingIndicator,
  Typography,
} from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { PasswordOptions, Roles, initialPasswordOptions } from 'shared/types/api/Users/Users.types';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { PageProps } from 'src/pages/PageTypes';
import { APP_SETTINGS_URL } from 'src/pages/SiteAdmin/SiteAdmin.constants';
import {
  dataTableSx,
  nonDataTableSx,
  pageSx,
  titleBoxSx,
} from 'src/pages/UserAdmin/UserAdmin.styles';
import { userColumns } from 'src/pages/UserAdmin/UserAdmin.constants';
import AddUserRow from './AddUserRow';
import EditUserRow from './EditUserRow';

const UserAdmin: React.FunctionComponent<PageProps> = ({ currentUser }: PageProps) => {
  const axiosInstance = useAxiosWebUIInstance(true);
  const showDeveloper = currentUser.role === Roles.Developer;

  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [userData, setUserData] = useState<any>([]);
  const [userRows, setUserRows] = useState<any>([]);
  const [showAddUser, setShowAddUser] = useState<boolean>(false);
  const [passwordSettings, setPasswordSetttings] =
    useState<PasswordOptions>(initialPasswordOptions);
  const [oldPasswords, setOldPasswords] = useState<number>(1);

  const updateUserData = useCallback(async () => {
    try {
      setIsLoading(true);
      const res = await axiosInstance.get('/users');
      setUserData(res.data.users);
    } finally {
      setIsLoading(false);
    }
  }, [axiosInstance]);

  const editButton = ({ onClickEvent }: any) => (
    <IconButton color='primary' icon='Edit' onClick={onClickEvent} size='small' />
  );

  const fetchPasswordData = useCallback(async () => {
    try {
      const res: any = await axiosInstance.get(APP_SETTINGS_URL);

      const passwordOptions: PasswordOptions = {
        passwordMinLength: res.data.password.minimum_password_length,
        passwordMaxLength: res.data.password.maximum_password_length,
        lowercase: res.data.password.lowercase,
        uppercase: res.data.password.uppercase,
        digit: res.data.password.digit,
        special: res.data.password.special,
      };

      setPasswordSetttings(passwordOptions);
      setOldPasswords(res.data.password.old_passwords);
    } finally {
    }
  }, [axiosInstance]);

  useEffect(() => {
    updateUserData();
    fetchPasswordData();
  }, [updateUserData, fetchPasswordData]);

  useEffect(() => {
    const rowData = userData.map((user: any) => ({
      id: user.id,
      user: user.username,
      role: user.role,
      expandRowButton: editButton,
      expandRowContent: (
        <EditUserRow
          showDeveloper={showDeveloper}
          setIsLoading={setIsLoading}
          updateUserData={updateUserData}
          user={user}
          passwordOptions={passwordSettings}
          oldPasswords={oldPasswords}
        />
      ),
    }));
    setUserRows(rowData);
  }, [updateUserData, userData, fetchPasswordData, passwordSettings, oldPasswords]);

  return (
    <Box sx={pageSx}>
      <PageLoadingIndicator isLoading={isLoading} type='primary' />
      <Box sx={titleBoxSx}>
        <Typography text='User Admin' variant='headingL' />
      </Box>
      <CardContainer direction='row'>
        <Box sx={dataTableSx}>
          <Box sx={nonDataTableSx}>
            <Typography text='MANAGE USERS' variant='headingS' />
          </Box>
          <DataTable
            columns={userColumns}
            expandable
            pagination
            rowsData={userRows}
            showExpandableRowIcons={false}
          />
          <Box sx={nonDataTableSx}>
            <MuiButton
              label='Add New User'
              onClick={() => setShowAddUser(!showAddUser)}
              startIcon='Add'
              sx={{ width: 'fit-content' }}
              variant='outlined'
            />
            {showAddUser && (
              <AddUserRow
                showDeveloper={showDeveloper}
                setIsLoading={setIsLoading}
                setShowAddUser={setShowAddUser}
                updateUserData={updateUserData}
                passwordOptions={passwordSettings}
                oldPasswords={oldPasswords}
              />
            )}
          </Box>
        </Box>
      </CardContainer>
    </Box>
  );
};

export default UserAdmin;
