import {
  Box, CardContainer, CardRow, Tab, Tabs, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { PageProps } from 'src/pages/PageTypes';
import TabContent from './TabContent';
import { BoxSX, CardContainerSX, Container } from './styles';

const UIConfig = ({ product }: PageProps) => {
  const tabs = [<Tab label="ASSETS" value="assets" />, <Tab label="DASHBOARD" value="dashboard" />];
  const filteredTabs = product === 'FM' ? tabs.filter((tab) => tab.props.value !== 'assets') : tabs;

  const initialTab = product === 'FM' ? 'dashboard' : 'assets';
  const [selectedTab, setSelectedTab] = useState(initialTab);

  return (
    <Container>
      <Typography text="UI Settings" variant="headingL" />
      <CardContainer direction="column" styleOverrides={CardContainerSX}>
        <Box sx={BoxSX}>
          <CardRow>
            <Tabs onChange={(_, tab) => setSelectedTab(tab as string)} value={selectedTab}>
              {...filteredTabs}
            </Tabs>
          </CardRow>
          <TabContent selectedTab={selectedTab} />
        </Box>
      </CardContainer>
    </Container>
  );
};

export default UIConfig;
