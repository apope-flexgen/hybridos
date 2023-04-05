// TODO: fix lint
/* eslint-disable react/no-unstable-nested-components, no-nested-ternary, max-lines */
import { PageLoadingIndicator, lightTheme } from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { SiteConfiguration } from 'shared/types/dtos/siteConfig.dto';
import HosControlFinal from 'src/assets/HosControlFinal.svg';
import HosCoordinateFinal from 'src/assets/HosCoordinate.svg';
import BaseApp from 'src/components/BaseApp';
import LogoutButton from 'src/components/LogoutButton';
import SiteStatusWrapper from 'src/components/SiteStatusWrapper';
import useAuth from 'src/hooks/useAuth';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import {
  Solar,
  Scheduler,
  Dashboard,
  Home,
  Events,
  Login,
  Feeders,
  Generators,
  ESS,
  Site,
  UIConfig,
  SiteAdmin,
  UserAdmin,
  ErcotOverride,
} from 'src/pages';
import SocketConnectionManager from 'src/services/SocketConnectionManager';
import { ThemeProvider } from 'styled-components';
import getRoutes from './helpers';
import {
  AUTH_USER_TOKEN_URL, initAppValue, LAYOUTS_URL, SITE_CONFIGURATION_URL,
} from './helpers/constants';

const PageDictionary = {
  Solar,
  Scheduler,
  Dashboard,
  Home,
  Events,
  Feeders,
  Generators,
  ESS,
  Site,
  UIConfig,
  SiteAdmin,
  UserAdmin,
  ErcotOverride,
};

const App = (): JSX.Element => {
  const SITE_CONTROLLER = 'SC';
  const FLEET_MANAGER = 'FM';
  const SITE_CONTROLLER_NAME = 'HybridOS Control';
  const FLEET_MANAGER_NAME = 'HybridOS Coordinate';

  const [appInit, setAppInit] = useState(initAppValue);
  const { setAuth } = useAuth();
  const [currentUser, setCurrentUser] = useState<any>(undefined);
  const [product, setProduct] = useState<string | null>(null);
  const [loggedIn, setLoggedIn] = useState<boolean>(false);
  const [isLoading, setLoading] = useState<boolean>(true);
  const [siteConfiguration, setSiteConfiguration] = useState<SiteConfiguration | null>(null);
  const axiosInstance = useAxiosWebUIInstance();

  const fetchData = useCallback(async () => {
    try {
      setLoading(true);
      const res = await axiosInstance.get(AUTH_USER_TOKEN_URL);
      const user = res.data;
      const { role } = user;
      const { username } = user;
      const siteConfigRes = await axiosInstance.get(SITE_CONFIGURATION_URL);
      const newSiteConfiguration = siteConfigRes.data;
      setSiteConfiguration(newSiteConfiguration);
      setProduct(newSiteConfiguration.product);
      const layoutsRes = await axiosInstance.get(LAYOUTS_URL);
      const layouts = layoutsRes.data.data;
      setLoggedIn(true);
      setAppInit({
        app: {
          appName: 'Hybridos Control',
          timeZone: newSiteConfiguration.timezone,
          appBar: {
            appLogo: product === FLEET_MANAGER
              ? HosCoordinateFinal
              : HosControlFinal,
            appLogoSize: product === FLEET_MANAGER ? 'large' : 'small',
            // TODO: Figure out what name to display for fleet manager
            appDisplayName: product === FLEET_MANAGER
              ? ''
              : newSiteConfiguration.site_name,
            appIcon: 'tbd', // TODO: should be included in FlexGen Component Lib
          },
        },
        routes: getRoutes(newSiteConfiguration, role, layouts),
        menuItems: [
          {
            children: username,
            divider: true,
            enableHover: false,
            color: 'primary',
            height: 'small',
          },
          {
            children: role,
            divider: false,
            enableHover: false,
            color: 'primary',
            height: 'small',
          },
          {
            children: <LogoutButton />,
            divider: false,
            enableHover: true,
            color: 'secondary',
            height: 'large',
          },
        ],
        footer: {
          softwareName: product === FLEET_MANAGER
          ? FLEET_MANAGER_NAME
          : SITE_CONTROLLER_NAME,
          version: '11.1.0',
        },
      });
      setCurrentUser(user);
    } finally {
      setLoading(false);
    }
  }, [axiosInstance]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const onLogin = (user: any) => {
    setAuth({ accessToken: user.accessToken });
    SocketConnectionManager.setAccessToken(user.accessToken);
    setCurrentUser(user);
    if (!user.passwordExpired && !user.mfaRequired) {
      setLoggedIn(true);
      axiosInstance.get(
        SITE_CONFIGURATION_URL,
        { headers: { Authorization: user.accessToken } },
      ).then((siteConfigRes) => {
        const updatedSiteConfiguration = siteConfigRes.data;
        setSiteConfiguration(updatedSiteConfiguration);
        axiosInstance.get(
          LAYOUTS_URL,
          { headers: { Authorization: user.accessToken } },
        ).then((layoutsRes) => {
          const layouts = layoutsRes.data.data;
          setAppInit({
            app: {
              appName: 'HybridOS',
              timeZone: updatedSiteConfiguration.timezone,
              appBar: {
                appLogo: updatedSiteConfiguration.product === FLEET_MANAGER
                  ? HosCoordinateFinal
                  : HosControlFinal,
                appLogoSize: updatedSiteConfiguration.product === FLEET_MANAGER ? 'large' : 'small',
                // TODO: Figure out what name to display for fleet manager
                appDisplayName: updatedSiteConfiguration.product === FLEET_MANAGER
                  ? ''
                  : updatedSiteConfiguration.site_name,
                appIcon: 'tbd', // TODO: should be included in FlexGen Component Lib
              },
            },
            routes: getRoutes(updatedSiteConfiguration, user.role, layouts),
            menuItems: [
              {
                children: user.username || '',
                divider: true,
                enableHover: false,
                color: 'primary',
                height: 'small',
              },
              {
                children: user.role || '',
                divider: false,
                enableHover: false,
                color: 'primary',
                height: 'small',
              },
              {
                children: <LogoutButton />,
                divider: false,
                enableHover: true,
                color: 'secondary',
                height: 'large',
              },
            ],
            footer: {
              softwareName: updatedSiteConfiguration.product === FLEET_MANAGER
              ? FLEET_MANAGER_NAME
              : SITE_CONTROLLER_NAME,
              version: '11.1.0',
            },
          });
        });
      });
    }
  };

  const LoadingIndicator = () => (
    <ThemeProvider theme={lightTheme}>
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </ThemeProvider>
  );

  const siteStatusBar = (siteConfiguration !== null
                          && siteConfiguration.product === SITE_CONTROLLER)
    ? <SiteStatusWrapper siteName="Horizon Power" />
    : undefined;

  // FIXME: where do we get sitename
  const Base = BaseApp(appInit, PageDictionary, currentUser, product, siteStatusBar || undefined);

  return isLoading ? (
    LoadingIndicator()
  ) : loggedIn ? (
    Base
  ) : (
    <Login onLogin={onLogin} user={currentUser} />
  );
};

export default App;
