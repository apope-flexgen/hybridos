import { ConfigurablePagesProps } from 'src/pages/ConfigurablePages/configurablePages.hoc';

export interface AssetsPageProps extends ConfigurablePagesProps {
  assetKey: string;
}

export type MappedAlerts = {
  alarms: JSX.Element[];
  faults: JSX.Element[];
};

export type AssetTab = 'asset' | 'batch';

export const AssetTabs = {
  asset: 'asset',
  batch: 'batch',
};
