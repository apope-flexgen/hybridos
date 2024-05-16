/* eslint-disable */
// TODO: fix lint
import {
  Box,
  CardContainer,
  DataTable,
  Divider,
  IconButton,
  PageLoadingIndicator,
  ThemeType,
  Typography,
} from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { PasswordOptions, Roles, initialPasswordOptions } from 'shared/types/api/Users/Users.types';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { PageProps } from 'src/pages/PageTypes';
import { APP_SETTINGS_URL } from 'src/pages/SiteAdmin/SiteAdmin.constants';
import {
  dataTableSx,
  pageSx,
  titleBoxSx,
  dataTableAndTypographySx,
  usersTablePaperSx,
} from 'src/pages/UserAdmin/UserAdmin.styles';
import { userColumns } from 'src/pages/UserAdmin/UserAdmin.constants';
import AddUserRow from './AddUserRow';
import EditUserRow from './EditUserRow';
import { useTheme } from 'styled-components';

const UserAdmin: React.FunctionComponent<PageProps> = ({ currentUser }: PageProps) => {
  const axiosInstance = useAxiosWebUIInstance(true);
  const showDeveloper = currentUser.role === Roles.Developer;

  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [userData, setUserData] = useState<any>([]);
  const [userRows, setUserRows] = useState<any>([]);
  const [passwordSettings, setPasswordSetttings] =
    useState<PasswordOptions>(initialPasswordOptions);
  const [oldPasswords, setOldPasswords] = useState<number>(1);
  const theme = useTheme() as ThemeType;

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
    <IconButton color='action' icon='EditOutlined' onClick={onClickEvent} size='small' />
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
        <Typography text='User Admin' variant='bodyXLBold' />
      </Box>
      <CardContainer direction='row' styleOverrides={{ height: '100%' }}>
        <Box sx={dataTableSx}>
          <AddUserRow
            showDeveloper={showDeveloper}
            setIsLoading={setIsLoading}
            updateUserData={updateUserData}
            passwordOptions={passwordSettings}
            oldPasswords={oldPasswords}
          />
          <Box sx={{ padding: '8px' }}>
            <Divider variant='fullWidth' orientation='horizontal' />
          </Box>
          <Box sx={dataTableAndTypographySx}>
            <Typography text='Active Users' variant='bodyLBold' sx={{ width: '20%' }} />
            <DataTable
              columns={userColumns}
              expandable
              pagination
              rowsData={userRows}
              showExpandableRowIcons={false}
              headerColor='secondary'
              paperSx={usersTablePaperSx(theme)}
            />
          </Box>
        </Box>
      </CardContainer>
    </Box>
  );
};

export default UserAdmin;
