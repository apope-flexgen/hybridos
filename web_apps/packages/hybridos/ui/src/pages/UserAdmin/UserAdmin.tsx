/* eslint-disable max-lines */
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
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import AddUserRow from './AddUserRow';
import EditUserRow from './EditUserRow';

const UserAdmin = () => {
  const axiosInstance = useAxiosWebUIInstance();

  const [isLoading, setIsLoading] = useState<boolean>(false);

  const [userData, setUserData] = useState<any>([]);
  const [userRows, setUserRows] = useState<any>([]);

  const [showAddUser, setShowAddUser] = useState<boolean>(false);

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
    <IconButton color="primary" icon="Edit" onClick={onClickEvent} size="small" />
  );

  useEffect(() => {
    updateUserData();
  }, [updateUserData]);

  useEffect(() => {
    const rowData = userData.map((user: any) => ({
      id: user.id,
      user: user.username,
      role: user.role,
      expandRowButton: editButton,
      expandRowContent: (
        <EditUserRow
          setIsLoading={setIsLoading}
          updateUserData={updateUserData}
          user={user}
        />
      ),
    }));
    setUserRows(rowData);
  }, [updateUserData, userData]);

  const userColumns = [
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

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        gap: '12px',
        width: '100%',
        flexGrow: 1,
      }}
    >
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
      <Box
        sx={{
          width: '90%',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        <Typography text="User Admin" variant="headingL" />
      </Box>
      <CardContainer
        flexDirection="row"
        styleOverrides={{
          width: '90%',
        }}
      >
        <Box
          sx={{
            display: 'flex',
            padding: '12px',
            gap: '15px',
            flexDirection: 'column',
            width: '100%',
          }}
        >
          <Typography text="MANAGE USERS" variant="headingS" />
          <DataTable
            columns={userColumns}
            expandable
            pagination
            rowsData={userRows}
            showExpandableRowIcons={false}
          />
          <MuiButton
            label="Add New User"
            onClick={() => setShowAddUser(!showAddUser)}
            startIcon="Add"
            sx={{ width: 'fit-content' }}
            variant="outlined"
          />
          {showAddUser && (
            <AddUserRow
              setIsLoading={setIsLoading}
              setShowAddUser={setShowAddUser}
              updateUserData={updateUserData}
            />
          )}
        </Box>
      </CardContainer>
    </Box>
  );
};

export default UserAdmin;
