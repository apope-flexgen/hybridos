// TODO: fix lint
/* eslint-disable react/no-unstable-nested-components */
import { darkTheme, Login } from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import HosControlFinal from 'src/assets/HosControlFinal.svg';
import HosCoordinate from 'src/assets/HosCoordinate.svg';
import LoginPage from 'src/components/LoginPage';
import MFALogin from 'src/components/MFALogin';
import PassExpLogin from 'src/components/PassExpLogin';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import QueryService from 'src/services/QueryService';
import { ThemeProvider } from 'styled-components';

const WebUILogin = (props: any) => {
  const { onLogin, user } = props;
  const axiosInstance = useAxiosWebUIInstance();
  const LOGIN_INFO_URL = '/login-info';
  const [product, setProduct] = useState<string>('');

  // restart socket when leaving page (on login)
  useEffect(() => () => {
    QueryService.cleanupSocket();
  });
  // get initial information for login page
  useEffect(() => {
    axiosInstance.get(LOGIN_INFO_URL).then((loginInfoResponse) => {
      setProduct(loginInfoResponse.data.product);
    });
  }, [axiosInstance]);

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
    if (user) {
      if (user.passwordExpired) {
        return <PassExpLogin onLogin={onLogin} user={user} />;
      }

      if (user.mfaRequired) {
        return <MFALogin onLogin={onLogin} user={user} />;
      }
    }
    return <div />;
  };

  const LoginComponentWithModal = () => (
    <>
      <Login
        loginRequestData={LOGIN_REQUEST}
        logo={product === 'FM' ? HosCoordinate : HosControlFinal}
        onLoginSuccess={onLogin}
      />
      {AltAuthModal()}
    </>
  );

  return (
    <ThemeProvider theme={darkTheme}>
      <LoginPage loginComponent={LoginComponentWithModal()} />
    </ThemeProvider>
  );
};

export default WebUILogin;
