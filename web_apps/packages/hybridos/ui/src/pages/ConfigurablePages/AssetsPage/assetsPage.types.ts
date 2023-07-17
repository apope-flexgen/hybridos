import { ConfigurablePagesProps } from 'src/pages/ConfigurablePages/configurablePages.hoc';

export interface AssetsPageProps extends ConfigurablePagesProps {
  assetKey: string;
}

export type MappedAlerts = {
  alarms: JSX.Element[];
  faults: JSX.Element[];
};
