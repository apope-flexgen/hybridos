/* eslint-disable */
// TODO: fix lint
import { darkTheme, Login } from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import HosControlFinal from 'src/assets/HosControlFinal.svg';
import HosCoordinate from 'src/assets/HosCoordinate.svg';
import LoginPage from 'src/components/LoginPage';
import MFALogin from 'src/components/MFALogin/MFALogin';
import PassExpLogin from 'src/components/PassExpLogin';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import QueryService from 'src/services/QueryService';
import { ThemeProvider } from 'styled-components';
import { useAppContext } from 'src/App/App';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import { SITE_CONFIGURATION_URL } from 'src/App/helpers/constants';
import useAuth from 'src/hooks/useAuth';

export type LoginInfo = {
  product: string;
  siteName?: string;
  customer?: string;
  server?: string;
}

const WebUILogin = (props: any) => {
  const axiosInstance = useAxiosWebUIInstance(true);
  const LOGIN_INFO_URL = '/login-info';
  const [loginInfo, setLoginInfo] = useState<LoginInfo>({product: "SC"});

  const { setAuth } = useAuth();
  const { currentUser, setCurrentUser, setLoggedIn, setSiteConfiguration, setProduct } = useAppContext();

  const onLogin = (user: any) => {
    const realTimeService = RealTimeService.Instance;
    setAuth({ accessToken: user.accessToken });
    realTimeService.setAccessToken(user.accessToken);
    setCurrentUser(user);
    if (!user.passwordExpired && !user.mfaRequired) {
      setLoggedIn(true);
      axiosInstance
        .get(SITE_CONFIGURATION_URL, { headers: { Authorization: user.accessToken } })
        .then((siteConfigRes) => {
          const updatedSiteConfiguration = siteConfigRes.data;
          setSiteConfiguration(updatedSiteConfiguration);
          setProduct(updatedSiteConfiguration.product);
        });
    }
  };

  // restart socket when leaving page (on login)
  useEffect(() => {
    return () => {
      QueryService.cleanupSocket();
    }
  }, []);

  // get initial information for login page
  useEffect(() => {
    axiosInstance.get(LOGIN_INFO_URL).then((loginInfoResponse) => {
      setLoginInfo(loginInfoResponse.data);
    });
  }, []);

  // TODO: Load from config.
  const LOGIN_REQUEST = {
    resource: '/api/login',
    options: {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      credentials: 'include' as RequestCredentials,
    },
  };
  const AltAuthModal = () => {
    if (currentUser) {
      if (currentUser.passwordExpired) {
        return <PassExpLogin onLogin={onLogin} user={currentUser} />;
      }

      if (currentUser.mfaRequired) {
        return <MFALogin onLogin={onLogin} user={currentUser} />;
      }
    }
    return <div />;
  };

  const LoginComponentWithModal = () => {
    return (
      <>
        <Login
          loginRequestData={LOGIN_REQUEST}
          logo={loginInfo.product === 'FM' ? HosCoordinate : HosControlFinal}
          onLoginSuccess={onLogin}
          customerName={loginInfo.customer || undefined}
          siteName={loginInfo.siteName || undefined}
          hardware={loginInfo.server || undefined}
        />
        {AltAuthModal()}
      </>
    );
  };

  return (
    <ThemeProvider theme={darkTheme}>
      <LoginPage loginComponent={LoginComponentWithModal()} />
    </ThemeProvider>
  );
};

export default WebUILogin;
