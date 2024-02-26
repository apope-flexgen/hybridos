import BatchControls from './BatchControls';
import Controls from './Controls';
import Info from './Info';
import NoAssets from './NoAssets';
import Statuses from './Statuses';
import Summary from './Summary';
import SummaryControl from './SummaryControls';
import { Container } from './styles';
import { TabContentProps } from './types';

const TabContent = ({ selectedTab }: TabContentProps) => {
  if (!selectedTab) return <NoAssets />;

  return (
    <Container>
      {selectedTab === 'info' && <Info />}
      {selectedTab === 'statuses' && <Statuses />}
      {selectedTab === 'controls' && <Controls />}
      {selectedTab === 'summary' && <Summary />}
      {selectedTab === 'summaryControls' && <SummaryControl />}
      {selectedTab === 'batchControls' && <BatchControls />}
    </Container>
  );
};

export default TabContent;
