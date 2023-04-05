import {
  AppBar,
  darkTheme,
  lightTheme,
  Icon,
  Footer,
  Avatar,
  Menu,
  MenuItem,
  PageLayout,
  DualIconButton,
} from '@flexgen/storybook';
import { TextColorType } from '@flexgen/storybook/dist/types/commonTypes';
import Box from '@mui/material/Box';
import { useState, ReactNode, MouseEvent } from 'react';
import { BrowserRouter as Router } from 'react-router-dom';
import NotifProvider from 'src/contexts/NotifContext';
import { ThemeProvider } from 'styled-components';
import ErrorModal from './ErrorModal';
import NavigationDrawer from './NavigationDrawer';
import AppRoutes from './Routes';
import ToastNotif from './ToastNotif';

const BaseApp = (
  appInitMock: any,
  pageDictionary: any,
  currentUser: any,
  product: string | null,
  statusBar?: JSX.Element,
): JSX.Element => {
  const [darkMode, setDarkMode] = useState(false);
  const [anchorEl, setAnchorEl] = useState<Element | undefined>(undefined);
  const open = Boolean(anchorEl);
  const handleClick = (event: MouseEvent<HTMLDivElement>) => setAnchorEl(event.currentTarget);
  const handleClose = () => setAnchorEl(undefined);
  const toggleDarkMode = () => { setDarkMode(!darkMode); };
  const {
    app: {
      timeZone, 
      appBar: {
        appLogo,
        appLogoSize,
        appDisplayName,
      },
    },
    routes,
    menuItems,
    footer: {
      softwareName,
      version
    }
  } = appInitMock;

  // TODO: export AppBar types
  return (
    <Router>
      <ThemeProvider theme={darkMode ? darkTheme : lightTheme}>
        <Box sx={{
          display: 'flex', padding: 0, flex: 1, minHeight: '100vh',
        }}
        >
          <AppBar
            logo={appLogo}
            logoSize={appLogoSize}
            siteName={appDisplayName}
            timeZone={timeZone}
          >
            <ThemeProvider theme={darkTheme}>
              <DualIconButton
                color={darkMode ? 'info' : 'warning'}
                onClick={toggleDarkMode}
                primaryIcon="LightMode"
                secondaryIcon="DarkMode"
              />
            </ThemeProvider>
            <Avatar onClick={handleClick}>
              <Icon src="AccountCircle" />
            </Avatar>
            <Box sx={{}}>
              <Menu
                anchorEl={anchorEl}
                onClose={handleClose}
                open={open}
              >
                {menuItems.map(
                  (menuItem: {
                    enableHover: boolean
                    children: ReactNode | string
                    divider: boolean
                    color: TextColorType
                    height: 'small' | 'large'
                  }) => (
                    <MenuItem
                      color={menuItem.color}
                      dense
                      disableHover={!menuItem.enableHover}
                      divider={menuItem.divider}
                      height={menuItem.height}
                    >
                      {menuItem.children}
                    </MenuItem>
                  ),
                )}
              </Menu>
            </Box>
          </AppBar>
          <NavigationDrawer routes={routes} />
          <ErrorModal />
          <NotifProvider>
            <ToastNotif />
            <PageLayout statusBar={statusBar}>
              <AppRoutes
                currentUser={currentUser}
                product={product}
                pageDictionary={pageDictionary}
                routes={routes}
              />
            </PageLayout>
          </NotifProvider>
        </Box>
        <Footer softwareName={softwareName} version={`Version ${version}`} />
      </ThemeProvider>
    </Router>
  );
};

export default BaseApp;
