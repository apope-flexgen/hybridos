export const SITE_DIAGRAM_URL = '/dashboard/site-diagram';
export const FLEET_SITE_IDS_URL = '/site-diagram/fleet-site-ids';

export enum ASSET {
  ESS = 'ESS',
  PCS = 'PCS',
  BMS = 'BMS',
  Solar = 'Solar',
  Gen = 'Gen',
  Feeder = 'Feeder',
  Grid = 'Grid',
  Default = 'Default',
}

export const assetTypeMapping: { [key: string]: string[] } = {
  [ASSET.ESS]: ['ess', 'storage'],
  [ASSET.PCS]: ['pcs'],
  [ASSET.BMS]: ['bms'],
  [ASSET.Solar]: ['solar'],
  [ASSET.Gen]: ['gen', 'gens', 'generator', 'generators'],
  [ASSET.Feeder]: ['feed', 'feeds', 'feeder', 'feeders'],
  [ASSET.Grid]: ['grid', 'grids'],
};

export const assetIconMapping: { [key: string]: string } = {
  [ASSET.ESS]: 'BatteryHorizontal',
  [ASSET.PCS]: 'Inverter',
  [ASSET.BMS]: 'ElectricBolt',
  [ASSET.Solar]: 'SolarRounded',
  [ASSET.Gen]: 'Generator',
  [ASSET.Feeder]: 'FeederAlt',
  [ASSET.Grid]: 'Microgrid',
};

export const layoutDirection = 'TB'; // Indicates top to bottom layout

export const minZoom = 0.1; // Determines how far out the diagram can zoom

export const resizeDiagramTimeout = 200; // Delay between diagram rerender being triggered and resize
