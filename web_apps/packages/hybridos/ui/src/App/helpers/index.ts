/* eslint-disable */
// TODO: fix lint
import { isValidIconString } from '@flexgen/storybook';
import { Roles } from 'shared/types/api/Users/Users.types';
import { FLEET_MANAGER } from 'src/components/BaseApp';

export type SiteConfiguration = {
  timezone: string;
  ess: boolean;
  gen: boolean;
  solar: boolean;
  met_station: boolean;
  tracker: boolean;
  feeders: boolean;
  features: boolean;
  site: boolean;
  events: boolean;
  control_cabinet: boolean;
  fleet_manager_dashboard: boolean;
  system_status?: boolean;
  scheduler: boolean;
  product: string;
  units: {
    [key: string]: string;
  };
  inspectorComponentsName: string;
  site_name?: string;
  fleet_name?: string;
  customer?: {
    name?: string;
    server?: string;
  };
};

export type RouteObject = {
  componentName?: string;
  icon?: string;
  itemName?: string;
  path?: string;
  showDivider?: boolean;
};

export const decideAppDisplayName = (siteConfig: SiteConfiguration): string =>
  siteConfig.product === FLEET_MANAGER
    ? siteConfig.customer?.name || ''
    : siteConfig.customer?.name || siteConfig.site_name || '';

const getRoutes = (
  userRole:
    | Roles.User
    | Roles.Admin
    | Roles.Rest
    | Roles.Developer
    | Roles.Observer
    | Roles.RestReadWrite,
  customAssets: { info: { key: string; name: string; icon?: string } }[],
  siteConfiguration: SiteConfiguration,
): RouteObject[] => {
  const customAssetsArray = customAssets
    ? customAssets.map(({ info: { name, key, icon } }) => {
        const validIcon = icon && isValidIconString(icon) ? icon : 'Storage';
        return {
          componentName: 'AssetsPage',
          icon: validIcon,
          itemName: name,
          path: `/${key}`,
          assetKey: key,
        };
      })
    : [];

  const routes = [
    siteConfiguration.product === 'FM'
      ? {
          componentName: 'FleetManagerDashboard',
          icon: 'Dashboard',
          itemName: 'Dashboard',
          path: '/',
        }
      : {
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
    siteConfiguration.features && {
      componentName: 'Features',
      icon: 'BatteryCharging',
      itemName: 'Features',
      path: '/features',
    },
    siteConfiguration.events && {
      componentName: 'ActivityLog',
      icon: 'Events',
      itemName: 'Activity Log',
      path: '/activity-log',
    },
    siteConfiguration.scheduler && {
      componentName: 'Scheduler',
      icon: 'Scheduler',
      itemName: 'Scheduler',
      path: '/scheduler',
    },
    siteConfiguration.product === 'FM' &&
      siteConfiguration.fleet_name &&
      siteConfiguration.fleet_name.toLowerCase() === 'ercot' && {
        componentName: 'ErcotOverride',
        icon: 'FilterList',
        itemName: 'ERCOT Overrides',
        path: '/ercot-override',
      },
    { showDivider: true },
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
    {
      showDivider: userRole === Roles.Admin || userRole === Roles.Developer ? true : false,
    },
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
    {
      showDivider: userRole === Roles.Admin || userRole === Roles.Developer ? true : false,
    },
    (userRole === Roles.Admin || userRole === Roles.Developer) &&
      siteConfiguration.system_status && {
        componentName: 'SystemStatus',
        icon: 'SystemStatus',
        itemName: 'System Status',
        path: '/system-status',
      },
  ];

  const returnRoutes = routes.filter(
    (item) => typeof item !== 'boolean' && item !== undefined,
  ) as RouteObject[];

  const routesWithoutAdjacentDividers = returnRoutes.filter(
    (item, index) =>
      index === 0 ||
      !('showDivider' in item) ||
      ('showDivider' in item && !('showDivider' in returnRoutes[index - 1])),
  );

  return routesWithoutAdjacentDividers;
};

export default getRoutes;
