import {
  Box, CardContainer, CardRow, Tab, Tabs,
  Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import TabContent from './TabContent';
import { BoxSX, CardContainerSX, Container } from './styles';

const UIConfig = () => {
  const [selectedTab, setSelectedTab] = useState('assets');

  return (
    <Container>
      <Typography text="UI Settings" variant="headingL" />
      <CardContainer flexDirection="column" styleOverrides={CardContainerSX}>
        <Box sx={BoxSX}>
          <CardRow>
            <Tabs
              onChange={(_, tab) => setSelectedTab(tab as string)}
              value={selectedTab}
            >
              <Tab
                label="ASSETS"
                value="assets"
              />
              <Tab
                label="DASHBOARD"
                value="dashboard"
              />
            </Tabs>
          </CardRow>
          <TabContent selectedTab={selectedTab} />
        </Box>
      </CardContainer>
    </Container>
  );
};

export default UIConfig;
