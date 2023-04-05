/* eslint-disable max-lines */
import { Roles } from 'shared/types/api/Users/Users.types';

const getRoutes = (
  siteConfiguration: {
    timezone: string
    ess: boolean
    gen: boolean
    solar: boolean
    met_station: boolean
    tracker: boolean
    feeders: boolean
    features: boolean
    site: boolean
    events: boolean
    control_cabinet: boolean
    fleet_manager_dashboard: boolean
    scheduler: boolean
    product: string
    units: {
      [key: string]: string
    }
    inspectorComponentsName: string
    site_name?: string
    fleet_name?: string
  },
  userRole:
  | Roles.User
  | Roles.Admin
  | Roles.Rest
  | Roles.Developer
  | Roles.Observer
  | Roles.RestReadWrite
  | undefined,
  customAssets: { info: { key: string; name: string } }[] | undefined,
) => {
  const customAssetsArray = customAssets
    ? customAssets.map(({ info: { name, key } }) => ({
      componentName: name,
      icon: 'Storage',
      itemName: name,
      path: `/${key}`,
    }))
    : [];

  const routes = [
    {
      componentName: 'Dashboard',
      icon: 'Dashboard',
      itemName: 'Dashboard',
      path: '/',
    },
    siteConfiguration.site && {
      componentName: 'Site',
      icon: 'Site',
      itemName: 'Site',
      path: '/site',
    },
    siteConfiguration.events && {
      componentName: 'Events',
      icon: 'Events',
      itemName: 'Events',
      path: '/events',
    },
    siteConfiguration.scheduler && {
      componentName: 'Scheduler',
      icon: 'Scheduler',
      itemName: 'Scheduler',
      path: '/scheduler',
    },
    (siteConfiguration.product === 'FM'
    && siteConfiguration.fleet_name
    && siteConfiguration.fleet_name.toLowerCase() === 'ercot'
    && {
      componentName: 'ErcotOverride',
      icon: 'FilterList',
      itemName: 'ERCOT Overrides',
      path: '/ercot-override',
    }),
    siteConfiguration.ess && {
      componentName: 'ESS',
      icon: 'ViewModule',
      itemName: 'ESS',
      path: '/ess',
    },
    siteConfiguration.solar && {
      componentName: 'Solar',
      icon: 'SpaceDashboard',
      itemName: 'Solar',
      path: '/solar',
    },
    siteConfiguration.feeders && {
      componentName: 'Feeders',
      icon: 'Feeders',
      itemName: 'Feeders',
      path: '/feeders',
    },
    siteConfiguration.gen && {
      componentName: 'Generators',
      icon: 'Storage',
      itemName: 'Generators',
      path: '/generators',
    },
    ...customAssetsArray,
    (userRole === Roles.Admin || userRole === Roles.Developer) && {
      componentName: 'UserAdmin',
      icon: 'UserAdmin',
      itemName: 'User Admin',
      path: '/user-admin',
    },
    (userRole === Roles.Admin || userRole === Roles.Developer) && {
      componentName: 'SiteAdmin',
      icon: 'SiteAdmin',
      itemName: 'Site Admin',
      path: '/site-admin',
    },
    (userRole === Roles.Admin || userRole === Roles.Developer) && {
      componentName: 'UIConfig',
      icon: 'UISettings',
      itemName: 'UI Config',
      path: '/ui-config',
    },
  ];

  return routes.filter((item) => typeof item !== 'boolean');
};

export default getRoutes;
