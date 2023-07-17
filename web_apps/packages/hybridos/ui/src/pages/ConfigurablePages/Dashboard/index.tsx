import { Box, ToggleButton, ToggleButtonGroup } from '@flexgen/storybook';
import { useState } from 'react';
import { DashboardLayout } from 'shared/types/dtos/configurablePages.dto';

import CardDashboard from './CardDashboard';
import TableDashboard from './TableDashboard';
import { dashboardBoxSx } from './dashboard.styles';

const Dashboard = () => {
  const storedLayout: DashboardLayout = (localStorage.getItem('dashboardLayout') as DashboardLayout) ?? DashboardLayout.CARD;
  const [layout, setLayout] = useState<DashboardLayout>(storedLayout);

  const handleLayout = (event: any, value: any) => {
    setLayout(value);
    localStorage.setItem('dashboardLayout', value);
  };

  return (
    <Box sx={dashboardBoxSx}>
      <ToggleButtonGroup size="small" value={layout} exclusive required onChange={handleLayout}>
        <ToggleButton value={DashboardLayout.TABLE}>Table</ToggleButton>
        <ToggleButton value={DashboardLayout.CARD}>Card</ToggleButton>
      </ToggleButtonGroup>

      {layout === DashboardLayout.CARD && <CardDashboard />}
      {layout === DashboardLayout.TABLE && <TableDashboard />}
    </Box>
  );
};

export default Dashboard;
