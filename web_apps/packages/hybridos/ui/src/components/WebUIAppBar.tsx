/* eslint-disable max-len */
import {
  AppBar,
  Avatar,
  Box,
  Icon,
  Menu,
  MenuItem,
  Switch,
  TextColorType,
  Typography,
} from '@flexgen/storybook';
import { useState, ReactNode, MouseEvent } from 'react';

import { useNavigate } from 'react-router-dom';
import { useErrorContextFunctions } from 'src/contexts/ErrorContext';

import { ErrorContextFunctionsType } from 'src/contexts/ErrorContext/types';
import useAuth from 'src/hooks/useAuth';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

export interface WebUIAppBarProps {
  appData: any;
  darkMode: boolean;
  handleDarkModeChange: () => void;
}

const WebUIAppBar = ({
  appData,
  darkMode,
  handleDarkModeChange,
}: WebUIAppBarProps): JSX.Element => {
  const [anchorEl, setAnchorEl] = useState<Element | undefined>(undefined);
  const open = Boolean(anchorEl);
  const handleClick = (event: MouseEvent<HTMLDivElement>) => setAnchorEl(event.currentTarget);
  const handleClose = () => setAnchorEl(undefined);

  const axiosInstance = useAxiosWebUIInstance(true);
  const { setAuth } = useAuth();
  const LOGOUT_URL = '/logout';
  const { showErrorModal, clearErrorModal } = useErrorContextFunctions() as ErrorContextFunctionsType;
  const navigate = useNavigate();

  const logout = async () => {
    try {
      await axiosInstance.post(LOGOUT_URL, {});
      setAuth({});
      navigate('/');
      window.location.reload();
    } catch (e) {
      showErrorModal({
        title: 'Logout Failed',
        description: 'An error occurred while logging out.',
        iconType: 'error',
        extraPropsAndActions: {
          hasConfirmButton: true,
          confirmButtonLabel: 'Retry',
          confirmButtonAction: () => {
            logout();
            clearErrorModal();
          },
        },
      });
    }
  };

  return (
    <AppBar
      logo={appData.app.appBar.appLogo}
      logoSize={appData.app.appBar.appLogoSize}
      siteName={appData.app.appBar.appDisplayName}
      timeZone={appData.app.timeZone}
    >
      <Avatar onClick={handleClick} color="secondary" variant="circular">
        <Icon src="Person" />
      </Avatar>
      <Box sx={{}}>
        <Menu anchorEl={anchorEl} onClose={handleClose} open={open}>
          {appData.menuItems.map(
            (menuItem: {
              enableHover: boolean;
              children: ReactNode | string;
              divider: boolean;
              color: TextColorType;
              height: 'small' | 'large';
            }) => (
              <MenuItem
                color={menuItem.color}
                disableHover={!menuItem.enableHover}
                divider={menuItem.divider}
                height={menuItem.height}
              >
                {menuItem.children}
              </MenuItem>
            ),
          )}
          <MenuItem disableHover color="primary" divider={false} height="large">
            <Box sx={{ display: 'flex', minWidth: '180px', alignItems: 'center' }}>
              <Switch onChange={handleDarkModeChange} value={darkMode} />
              <Typography text="Dark Mode" variant="bodyL" />
            </Box>
          </MenuItem>
          <MenuItem
            divider={false}
            disableHover={false}
            height="large"
            onClick={logout}
            color={darkMode ? 'primary' : 'secondary'}
          >
            <Typography
              text="LOG OUT"
              variant="buttonMedium"
              color={darkMode ? 'primary' : 'reversed'}
            />
          </MenuItem>
        </Menu>
      </Box>
    </AppBar>
  );
};

export default WebUIAppBar;
