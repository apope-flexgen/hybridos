import { rest } from 'msw';

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

export const appInitMock = rest.get('/app', (req, res, ctx) => res(ctx.json(mockSiteConfig)));

export const layoutsMock = rest.get('/layouts', (req, res, ctx) => res(ctx.json({ data: layouts })));
