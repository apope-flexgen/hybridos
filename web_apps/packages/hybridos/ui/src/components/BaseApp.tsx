/* eslint-disable max-lines */
// TODO: fix lint
import {
  darkTheme, lightTheme, Footer, PageLayout, Typography,
} from '@flexgen/storybook';
import Box from '@mui/material/Box';
import isEqual from 'lodash.isequal';
import {
  useCallback, useEffect, useMemo, useState,
} from 'react';
import { BrowserRouter as Router } from 'react-router-dom';
import { useAppContext } from 'src/App/App';
import getRoutes, { decideAppDisplayName } from 'src/App/helpers';
import { SITE_CONFIGURATION_URL, initWebUIConfigValue } from 'src/App/helpers/constants';
import HosControlFinal from 'src/assets/HosControlFinal.svg';
import HosCoordinateFinal from 'src/assets/HosCoordinate.svg';
import SiteStatusWrapper from 'src/components/SiteStatusWrapper';
import WebUIAppBar from 'src/components/WebUIAppBar';
import NotifProvider from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import {
  AssetsPage,
  Solar,
  Scheduler,
  Dashboard,
  ActivityLog,
  Feeders,
  Features,
  Generators,
  ESS,
  Site,
  UIConfig,
  SiteAdmin,
  UserAdmin,
  ErcotOverride,
  SystemStatus,
} from 'src/pages';
import QueryService from 'src/services/QueryService';
import { ThemeProvider } from 'styled-components';
import ErrorModal from './ErrorModal';
import NavigationDrawer from './NavigationDrawer';
import AppRoutes from './Routes';
import ToastNotif from './ToastNotif';

const PageDictionary = {
  AssetsPage,
  Solar,
  Scheduler,
  Dashboard,
  ActivityLog,
  Feeders,
  Generators,
  Features,
  ESS,
  Site,
  UIConfig,
  SiteAdmin,
  UserAdmin,
  ErcotOverride,
  SystemStatus,
};

export const SITE_CONTROLLER = 'SC';
export const FLEET_MANAGER = 'FM';
export const SITE_CONTROLLER_NAME = 'HybridOS Control';
export const FLEET_MANAGER_NAME = 'HybridOS Coordinate';

const BaseApp = (): JSX.Element => {
  const isDarkMode = localStorage.getItem('darkMode');
  const [darkMode, setDarkMode] = useState<boolean>(isDarkMode === 'true');

  const axiosInstance = useAxiosWebUIInstance(true);
  const {
    siteConfiguration,
    currentUser,
    product,
    setSiteConfiguration,
    setProduct,
    layouts,
    setLayouts,
  } = useAppContext();

  const statusBar = siteConfiguration !== null && siteConfiguration.site_status_bar ? (
    <SiteStatusWrapper />
  ) : undefined;

  const routes: any = useMemo(() => {
    if (currentUser && layouts && siteConfiguration) {
      return getRoutes(currentUser.role, layouts, siteConfiguration);
    }
    return [];
  }, [currentUser, layouts, siteConfiguration]);

  const fetchData = useCallback(async () => {
    const siteConfigRes = await axiosInstance.get(SITE_CONFIGURATION_URL);
    const newSiteConfiguration = siteConfigRes.data;
    setSiteConfiguration(newSiteConfiguration);
    setProduct(newSiteConfiguration.product);
  }, [axiosInstance, setProduct, setSiteConfiguration]);

  const appInit = useMemo(() => {
    if (siteConfiguration && currentUser) {
      return {
        app: {
          appName: 'HybridOS',
          timeZone: siteConfiguration.timezone,
          appBar: {
            appLogo:
              siteConfiguration.product === FLEET_MANAGER ? HosCoordinateFinal : HosControlFinal,
            appLogoSize: siteConfiguration.product === FLEET_MANAGER ? 'large' : 'small',
            appDisplayName: decideAppDisplayName(siteConfiguration),
            appIcon: 'tbd', // TODO: should be included in FlexGen Component Lib
          },
        },
        routes,
        menuItems: [
          {
            children: (
              <Typography
                text={currentUser.username || ''}
                variant="bodyL"
                sx={{ paddingLeft: '8px' }}
              />
            ),
            divider: false,
            enableHover: false,
            color: 'primary',
            height: 'large',
          },
          {
            children: (
              <Typography
                text={currentUser.role.toUpperCase() || ''}
                variant="bodyS"
                sx={{ paddingLeft: '8px' }}
              />
            ),
            divider: true,
            enableHover: false,
            color: 'primary',
            height: 'small',
          },
        ],
        footer: {
          softwareName:
            siteConfiguration.product === FLEET_MANAGER ? FLEET_MANAGER_NAME : SITE_CONTROLLER_NAME,
          version: '12',
        },
      };
    }
    return initWebUIConfigValue;
  }, [currentUser, routes, siteConfiguration]);

  const handleDarkModeChange = () => {
    setDarkMode((prevDarkMode) => {
      localStorage.setItem('darkMode', String(!prevDarkMode));
      return !prevDarkMode;
    });
  };

  const handleNewMessage = useCallback(
    (newInformationFromSocket: any) => {
      const parsedData = newInformationFromSocket.data ?? [];
      setLayouts((prevLayouts) => {
        if (!isEqual(prevLayouts, parsedData)) {
          return parsedData;
        }
        return prevLayouts;
      });
    },
    [setLayouts],
  );

  useEffect(() => {
    QueryService.getLayouts(handleNewMessage);
  }, [handleNewMessage]);

  useEffect(
    () => () => {
      QueryService.cleanupSocket();
    },
    [],
  );

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  // TODO: export AppBar types
  return (
    <Router>
      <ThemeProvider theme={darkMode ? darkTheme : lightTheme}>
        <Box
          sx={{
            display: 'flex',
            padding: 0,
            flex: 1,
            minHeight: '100vh',
          }}
        >
          <WebUIAppBar
            appData={appInit}
            darkMode={darkMode}
            handleDarkModeChange={handleDarkModeChange}
          />
          <ErrorModal />
          <NotifProvider>
            <ToastNotif />
            <PageLayout
              statusBar={statusBar}
              navigationDrawer={<NavigationDrawer routes={routes} />}
            >
              {layouts && (
                <AppRoutes
                  currentUser={currentUser}
                  product={product}
                  pageDictionary={PageDictionary}
                  routes={routes}
                />
              )}
            </PageLayout>
          </NotifProvider>
        </Box>
        <Footer
          softwareName={appInit.footer.softwareName}
          version={`Version ${appInit.footer.version}`}
        />
      </ThemeProvider>
    </Router>
  );
};

export default BaseApp;
