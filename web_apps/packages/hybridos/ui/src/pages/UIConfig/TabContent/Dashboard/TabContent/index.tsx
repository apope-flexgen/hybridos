import Info from './Info';
import NoDashboards from './NoDashboards';
import Statuses from './Statuses';
import { Container } from './styles';
import { TabContentProps } from './types';

const TabContent = ({ selectedTab }: TabContentProps) => {
  if (!selectedTab) return <NoDashboards />;

  return (
    <Container>
      {selectedTab === 'info' && <Info />}
      {selectedTab === 'statuses' && <Statuses />}
    </Container>
  );
};

export default TabContent;
