export enum ASSET {
  ESS = 'ESS',
  Solar = 'Solar',
  Gen = 'Gen',
  Feeder = 'Feeder',
  Grid = 'Grid',
  Default = 'Default',
}

export const assetTypeMapping: { [key: string]: string[] } = {
  [ASSET.ESS]: ['ess', 'storage'],
  [ASSET.Solar]: ['solar'],
  [ASSET.Gen]: ['gen', 'gens', 'generator', 'generators'],
  [ASSET.Feeder]: ['feed', 'feeds', 'feeder', 'feeders'],
  [ASSET.Grid]: ['grid', 'grids'],
};

export const assetIconMapping: { [key: string]: string } = {
  [ASSET.ESS]: 'BatteryHorizontal',
  [ASSET.Solar]: 'SolarRounded',
  [ASSET.Gen]: 'Generator',
  [ASSET.Feeder]: 'FeederAlt',
  [ASSET.Grid]: 'Microgrid',
};

export const layoutDirection = 'TB'; // Indicates top to bottom layout
