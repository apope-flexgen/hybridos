import {
  Box, ToggleButton, ToggleButtonGroup, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { DashboardLayout } from 'shared/types/dtos/configurablePages.dto';
import { useAppContext } from 'src/App/App';

import CardDashboard from './CardDashboard';
import DiagramDashboard from './DiagramDashboard';
import TableDashboard from './TableDashboard';

import { dashboardBoxSx, titleButtonBoxSx } from './dashboard.styles';

const Dashboard = () => {
  const storedLayout: DashboardLayout = (localStorage.getItem('dashboardLayout') as DashboardLayout) ?? DashboardLayout.CARD;
  const [layout, setLayout] = useState<DashboardLayout>(storedLayout);
  const { siteConfiguration } = useAppContext();

  const handleLayout = (e: React.MouseEvent<HTMLElement, MouseEvent>, value: DashboardLayout) => {
    setLayout(value);
    localStorage.setItem('dashboardLayout', value);
  };

  return (
    <Box sx={dashboardBoxSx}>
      <Box sx={titleButtonBoxSx}>
        <Typography text="Dashboard" variant="bodyXLBold" />
        <ToggleButtonGroup
          size="small"
          value={layout}
          color="secondary"
          exclusive
          required
          onChange={handleLayout}
        >
          <ToggleButton value={DashboardLayout.TABLE} color="secondary">
            Table
          </ToggleButton>
          <ToggleButton value={DashboardLayout.CARD} color="secondary">
            Card
          </ToggleButton>
          {siteConfiguration?.site_diagram && (
            <ToggleButton color="secondary" value={DashboardLayout.DIAGRAM}>
              Diagram
            </ToggleButton>
          )}
        </ToggleButtonGroup>
      </Box>
      {layout === DashboardLayout.CARD && <CardDashboard />}
      {layout === DashboardLayout.TABLE && <TableDashboard />}
      {siteConfiguration?.site_diagram && layout === DashboardLayout.DIAGRAM && (
        <DiagramDashboard />
      )}
    </Box>
  );
};

export default Dashboard;
