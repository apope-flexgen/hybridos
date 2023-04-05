import { appInitMock, layoutsMock } from './App/appInit.mock';
import authUserTokenMock from './pages/Auth/AuthUserToken.mock';
import fimsOneTimeAuthMock from './pages/Auth/FimsOneTimeAuth.mock';
import { loginMock } from './pages/Auth/Login.mock';
import logoutMock from './pages/Auth/Logout.mock';
import { mfaMock } from './pages/Auth/MFA.mock';
import { passExpMock } from './pages/Auth/PassExp.mock';
import refreshTokenMock from './pages/Auth/RefreshToken.mock';
import eventsPageMock from './pages/Events/Events.mock';
import {
  schedulerModesMock,
  schedulerEventsRoute,
  postEventsRoute,
  postModesRoute,
  deleteModeRoute,
  deleteEventsRoute,
  schedulerConfigurationMock,
  schedulerTimezonesMock,
  putConfigurationRoute,
} from './pages/Scheduler/Scheduler.mock';
import { radiusTestMock, siteAdminMock, siteAdminPostMock } from './pages/SiteAdmin/SiteAdmin.mock';

const handlers = [
  appInitMock,
  layoutsMock,
  eventsPageMock,
  schedulerModesMock,
  schedulerConfigurationMock,
  schedulerTimezonesMock,
  schedulerEventsRoute,
  deleteModeRoute,
  deleteEventsRoute,
  schedulerTimezonesMock,
  postEventsRoute,
  putConfigurationRoute,
  postModesRoute,
  authUserTokenMock,
  logoutMock,
  passExpMock,
  loginMock,
  mfaMock,
  refreshTokenMock,
  fimsOneTimeAuthMock,
  radiusTestMock,
  siteAdminMock,
  siteAdminPostMock,
];

export default handlers;
