export const initWebUIConfigValue = {
  app: {
    appName: 'HybridOs',
    timeZone: 'America/New_York',
    appBar: {
      appLogo: '',
      appLogoSize: 'small',
      appDisplayName: '',
      appIcon: '',
    },
  },
  routes: [] as any[],
  menuItems: [
    {
      children: {},
      divider: false,
      enableHover: false,
      color: 'primary',
      height: 'small',
    },
  ],
  footer: {
    softwareName: '',
    version: '',
  },
};

export const AUTH_USER_TOKEN_URL = '/authenticate-user-token';
export const SITE_CONFIGURATION_URL = '/web-ui-config';
export const LAYOUTS_URL = '/layouts';
