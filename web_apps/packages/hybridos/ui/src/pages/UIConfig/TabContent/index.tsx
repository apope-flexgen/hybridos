import Assets from './Assets';
import Dashboard from './Dashboard';
import { TabContentProps } from './types';

const TabContent = ({ selectedTab }: TabContentProps) => (
  <>
    {selectedTab === 'assets' && <Assets />}
    {selectedTab === 'dashboard' && <Dashboard />}
  </>
);

export default TabContent;
