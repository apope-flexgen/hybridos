import { ConfigurablePagesProps } from 'src/pages/ConfigurablePages/configurablePages.hoc';

export interface AssetsPageProps extends ConfigurablePagesProps {
  category: string
}

export type MappedAlerts = {
  alarms: JSX.Element[]
  faults: JSX.Element[]
};
