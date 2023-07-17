/* eslint-disable no-underscore-dangle, max-statements, max-lines, max-len */
// TODO: fix lint
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
  SiteAdmins,
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
  initialsiteAdmins,
  ConnectionMethods,
  connectionMethodRadios,
  siteAdminLabels,
  APP_SETTINGS_URL,
  RADIUS_TEST_URL,
  RADIUS_FAILED_MESSAGE,
} from './SiteAdmin.constants';
import {
  buttonBoxSx,
  connectionMethodBoxSx,
  connectionRadiusBoxSx,
  containerBoxSx,
  contentBoxSx,
  saveCancelBoxSx,
  titleBoxSx,
} from './styles';

const SiteAdmin = () => {
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const [connectionMethod, setConnectionMethod] = useState<ConnectionMethods>('onlyLocal');
  const [radiusTestSuccessful, setRadiusTestSuccessful] = useState<boolean>(false);

  const [passwordSettings, setPasswordSettings] = useState<PasswordSettings>(initialPasswordSettings);
  const [radiusSettings, setRadiusSettings] = useState<RadiusSettings>(initialRadiusSettings);

  const [siteAdminsId, setsiteAdminsId] = useState<string>('');
  const [siteAdminsVersion, setsiteAdminsVersion] = useState<number>(0);
  const [currentsiteAdmins, setCurrentsiteAdmins] = useState<SiteAdmins>(initialsiteAdmins);

  const [username, setUsername] = useState<string>('');
  const [password, setPassword] = useState<string>('');

  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [isRadiusTestLoading, setIsRadiusTestLoading] = useState<boolean>(false);

  const updatesiteAdmins = (data: SiteAdmins) => {
    setCurrentsiteAdmins(data);
    setPasswordSettings(data.password);
    setRadiusSettings(data.radius);
    setPassword('');
    setUsername('');
    setRadiusTestSuccessful(false);
    if (data.radius.is_local_auth_disabled) setConnectionMethod('onlyRadius');
    else if (data.radius.is_enabled) setConnectionMethod('localAndRadius');
    else setConnectionMethod('onlyLocal');
    setsiteAdminsId(data._id);
    setsiteAdminsVersion(data.__v);
    setIsLoading(false);
  };

  const updateAfterPost = (data: any) => {
    setCurrentsiteAdmins(data);
    setPasswordSettings(data.password);
    setRadiusSettings(data.radius);
    setPassword('');
    setUsername('');
    if (data.radius.is_local_auth_disabled) setConnectionMethod('onlyRadius');
    else if (data.radius.is_enabled) setConnectionMethod('localAndRadius');
    else setConnectionMethod('onlyLocal');
    setsiteAdminsId(data._id);
    setsiteAdminsVersion(data.__v);
    setRadiusTestSuccessful(false);
    setIsLoading(false);
    notifCtx?.notif('success', 'Data successfully saved');
  };

  const postNewsiteAdmins = async () => {
    try {
      setIsLoading(true);
      const newSettings: SiteAdmins = {
        _id: siteAdminsId,
        password: passwordSettings,
        radius: radiusSettings,
        __v: siteAdminsVersion,
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
      updatesiteAdmins(res.data);
    } finally {
      setIsLoading(false);
    }
  }, [axiosInstance]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const theme = useTheme() as ThemeType;
  const connectionBoxSx = connectionMethodBoxSx(theme);
  const contentContainerSx = contentBoxSx(theme);

  return (
    <Box sx={containerBoxSx}>
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
      <Box sx={titleBoxSx}>
        <Typography text={siteAdminLabels.siteAdminPageTitle} variant="headingL" />
        <Box sx={buttonBoxSx}>
          <Box sx={saveCancelBoxSx}>
            <MuiButton
              color="inherit"
              label="cancel"
              onClick={() => updatesiteAdmins(currentsiteAdmins)}
              variant="text"
            />
            <MuiButton
              disabled={
                ((connectionMethod === 'onlyRadius' || connectionMethod === 'localAndRadius')
                  && !radiusTestSuccessful)
                || passwordSettings.maximum_password_length < passwordSettings.minimum_password_length
              }
              id="save_button"
              label="save"
              onClick={postNewsiteAdmins}
            />
          </Box>
          {(connectionMethod === 'onlyRadius' || connectionMethod === 'localAndRadius')
            && !radiusTestSuccessful && (
              <Typography text={siteAdminLabels.needSuccessfulTestErrorMessage} variant="bodyS" />
          )}
        </Box>
      </Box>
      <CardContainer
        flexDirection="row"
        styleOverrides={{
          backgroundColor: theme.fgd.primary.main_4p,
          height: ' calc(100% - 72px)',
        }}
      >
        <Box sx={connectionBoxSx}>
          <Typography text={siteAdminLabels.authenticationMethodTitle} variant="bodyLBold" />
          {connectionMethodRadios.map((connectionMethodRadio) => (
            <Box sx={connectionRadiusBoxSx}>
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
            </Box>
          ))}
        </Box>
        <Box sx={contentContainerSx}>
          {(connectionMethod === 'onlyLocal' || connectionMethod === 'localAndRadius') && (
            <PasswordSettingsFields
              passwordSettings={passwordSettings}
              setPasswordSettings={setPasswordSettings}
            />
          )}
          {(connectionMethod === 'onlyRadius' || connectionMethod === 'localAndRadius') && (
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
