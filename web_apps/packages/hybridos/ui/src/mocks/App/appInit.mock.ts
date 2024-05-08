import { rest } from 'msw';
import { SITE_CONFIGURATION_URL, LAYOUTS_URL } from 'src/App/helpers/constants';

const mockSiteConfig = {
  timezone: 'America/Chicago',
  ess: false,
  gen: true,
  solar: true,
  met_station: false,
  tracker: false,
  feeders: true,
  features: true,
  site: true,
  events: true,
  control_cabinet: true,
  system_status: true,
  fleet_manager_dashboard: true,
  scheduler: true,
  units: {
    W: 'M',
    Wh: 'M',
    VAR: 'M',
    VARh: 'M',
  },
  inspectorComponentsName: 'test_controller',
  site_name: 'Durham',
};

const layouts = [{ info: { key: 'ESS', name: 'ESS' } }];

const appInitCallback = (req, res, ctx) => res(ctx.json(mockSiteConfig));
export const appInitMock = rest.get(SITE_CONFIGURATION_URL, appInitCallback);

const layoutsMockCallabck = (req, res, ctx) => res(ctx.json({ data: layouts }));
export const layoutsMock = rest.get(LAYOUTS_URL, layoutsMockCallabck);
