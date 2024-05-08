import { ConfigurablePagesProps } from 'src/pages/ConfigurablePages/configurablePages.hoc';
import {
  AlertState,
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
} from 'src/pages/ConfigurablePages/configurablePages.types';

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

export interface SingleAssetProps {
  assetName: string;
  statusChildren: ConfigurableComponentFunction[];
  maintenanceActionsChildren: ConfigurableComponentFunction[];
  assetState: ConfigurablePageStateStructure;
  alertState: AlertState[string];
  maintModeStatus?: boolean;
  maintenanceActionsState: boolean;
}
