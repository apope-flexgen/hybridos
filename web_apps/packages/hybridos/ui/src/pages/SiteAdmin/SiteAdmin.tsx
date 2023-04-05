// TODO: fix lint
/* eslint-disable no-underscore-dangle */
/* eslint-disable max-statements */
/* eslint-disable max-lines */
import {

  CardContainer,
  MuiButton,
  Typography,
  Box,
  RadioButton,
  ThemeType,
  PageLoadingIndicator,
} from '@flexgen/storybook';
import {
  useState, useEffect, useContext, useCallback,
} from 'react';
import {
  AppSettings,
  PasswordSettings,
  RadiusSettings,
  RadiusTestSettings,
} from 'shared/types/api/SiteAdmin.types';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useTheme } from 'styled-components';
import PasswordSettingsFields from './PasswordSettings';
import RadiusSettingsFields from './RadiusSettings';
import {
  initialPasswordSettings,
  initialRadiusSettings,
  initialAppSettings,
  ConnectionMethods,
  connectionMethodRadios,
  siteAdminLabels,
  APP_SETTINGS_URL,
  RADIUS_TEST_URL,
  RADIUS_FAILED_MESSAGE,
} from './SiteAdmin.constants';

const SiteAdmin = () => {
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const [connectionMethod, setConnectionMethod] = useState<ConnectionMethods>('onlyLocal');
  const [radiusTestSuccessful, setRadiusTestSuccessful] = useState<boolean>(false);

  const [
    passwordSettings,
    setPasswordSettings,
  ] = useState<PasswordSettings>(initialPasswordSettings);
  const [radiusSettings, setRadiusSettings] = useState<RadiusSettings>(initialRadiusSettings);

  const [appSettingsId, setAppSettingsId] = useState<string>('');
  const [appSettingsVersion, setAppSettingsVersion] = useState<number>(0);
  const [currentAppSettings, setCurrentAppSettings] = useState<AppSettings>(initialAppSettings);

  const [username, setUsername] = useState<string>('');
  const [password, setPassword] = useState<string>('');

  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [isRadiusTestLoading, setIsRadiusTestLoading] = useState<boolean>(false);

  const updateAppSettings = (data: AppSettings) => {
    setCurrentAppSettings(data);
    setPasswordSettings(data.password);
    setRadiusSettings(data.radius);
    setPassword('');
    setUsername('');
    setRadiusTestSuccessful(false);
    if (data.radius.is_local_auth_disabled) setConnectionMethod('onlyRadius');
    else if (data.radius.is_enabled) setConnectionMethod('localAndRadius');
    else setConnectionMethod('onlyLocal');
    setAppSettingsId(data._id);
    setAppSettingsVersion(data.__v);
    setIsLoading(false);
  };

  const updateAfterPost = (data: any) => {
    setCurrentAppSettings(data);
    setPasswordSettings(data.password);
    setRadiusSettings(data.radius);
    setPassword('');
    setUsername('');
    if (data.radius.is_local_auth_disabled) setConnectionMethod('onlyRadius');
    else if (data.radius.is_enabled) setConnectionMethod('localAndRadius');
    else setConnectionMethod('onlyLocal');
    setAppSettingsId(data._id);
    setAppSettingsVersion(data.__v);
    setRadiusTestSuccessful(false);
    setIsLoading(false);
    notifCtx?.notif('success', 'Data successfully saved');
  };

  const postNewAppSettings = async () => {
    try {
      setIsLoading(true);
      const newSettings: AppSettings = {
        _id: appSettingsId,
        password: passwordSettings,
        radius: radiusSettings,
        __v: appSettingsVersion,
      };
      const res = await axiosInstance.post(APP_SETTINGS_URL, newSettings);
      updateAfterPost(res.data);
    } finally {
      setIsLoading(false);
    }
  };

  const displayRadiusTestResults = (data: any) => {
    setIsRadiusTestLoading(false);
    if (data.message !== RADIUS_FAILED_MESSAGE) setRadiusTestSuccessful(true);
    notifCtx?.notif(data.message === RADIUS_FAILED_MESSAGE ? 'error' : 'success', data.message);
  };

  const sendRadiusTest = async () => {
    try {
      setIsRadiusTestLoading(true);
      const radiusTestSettings: RadiusTestSettings = {
        ip_address: radiusSettings.ip_address,
        port: radiusSettings.port,
        secret_phrase: radiusSettings.secret_phrase,
        wait_time: radiusSettings.wait_time,
        username,
        password,
      };
      const res = await axiosInstance.post(RADIUS_TEST_URL, radiusTestSettings);
      displayRadiusTestResults(res.data);
    } finally {
      setIsRadiusTestLoading(false);
    }
  };

  const fetchData = useCallback(async () => {
    try {
      setIsLoading(true);
      const res = await axiosInstance.get(APP_SETTINGS_URL);
      updateAppSettings(res.data);
    } finally {
      setIsLoading(false);
    }
  }, [axiosInstance]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const theme = useTheme() as ThemeType;

  return (
    <Box sx={{
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
        <Typography text={siteAdminLabels.siteAdminPageTitle} variant="headingL" />
        <Box
          sx={{
            display: 'flex',
            flexDirection: 'column',
            gap: '8px',
            alignItems: 'flex-end',
          }}
        >
          <Box sx={{ display: 'flex', gap: '16px' }}>
            <MuiButton
              color="inherit"
              label="cancel"
              onClick={() => updateAppSettings(currentAppSettings)}
              variant="text"
            />
            <MuiButton
              disabled={
                                (connectionMethod === 'onlyRadius'
                                    || connectionMethod === 'localAndRadius')
                                && !radiusTestSuccessful
                            }
              id="save_button"
              label="save"
              onClick={postNewAppSettings}
            />
          </Box>
          {(connectionMethod === 'onlyRadius' || connectionMethod === 'localAndRadius')
                        && !radiusTestSuccessful && (
                        <Typography
                          text={siteAdminLabels.needSuccessfulTestErrorMessage}
                          variant="bodyS"
                        />
          )}
        </Box>
      </Box>
      <CardContainer
        flexDirection="row"
        styleOverrides={{ width: '90%', backgroundColor: theme.fgd.primary.main_4p }}
      >
        <Box
          sx={{
            display: 'flex',
            padding: '24px',
            flexDirection: 'column',
            height: '100vf',
            width: '21%',
            backgroundColor: theme.fgd.primary.main_4p,
          }}
        >
          <Typography
            text={siteAdminLabels.authenticationMethodTitle}
            variant="bodyLBold"
          />
          {connectionMethodRadios.map((connectionMethodRadio) => (
            <RadioButton
              label={connectionMethodRadio.label}
              labelPlacement="end"
              onChange={() => {
                setConnectionMethod(connectionMethodRadio.value);
                if (connectionMethodRadio.value === 'onlyRadius') {
                  setRadiusSettings({
                    ...radiusSettings,
                    is_enabled: true,
                    is_local_auth_disabled: true,
                  });
                } else if (connectionMethodRadio.value === 'localAndRadius') {
                  setRadiusSettings({
                    ...radiusSettings,
                    is_enabled: true,
                    is_local_auth_disabled: false,
                  });
                } else {
                  setRadiusSettings({
                    ...radiusSettings,
                    is_enabled: false,
                    is_local_auth_disabled: false,
                  });
                }
              }}
              size="small"
              value={connectionMethod === connectionMethodRadio.value}
            />
          ))}
        </Box>
        <Box
          sx={{
            display: 'flex',
            flexGrow: 1,
            width: '60%',
            flexDirection: 'column',
            padding: '24px',
            backgroundColor: theme.fgd.background.paper,
          }}
        >
          {(connectionMethod === 'onlyLocal'
                        || connectionMethod === 'localAndRadius') && (
                        <PasswordSettingsFields
                          passwordSettings={passwordSettings}
                          setPasswordSettings={setPasswordSettings}
                        />
          )}
          {(connectionMethod === 'onlyRadius'
                        || connectionMethod === 'localAndRadius') && (
                        <RadiusSettingsFields
                          isRadiusTestLoading={isRadiusTestLoading}
                          password={password}
                          radiusSettings={radiusSettings}
                          radiusTestSuccessful={radiusTestSuccessful}
                          sendRadiusTest={sendRadiusTest}
                          setPassword={setPassword}
                          setRadiusSettings={setRadiusSettings}
                          setUsername={setUsername}
                          username={username}
                        />
          )}
        </Box>
      </CardContainer>
    </Box>
  );
};

export default SiteAdmin;
