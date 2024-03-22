import { Box, ToggleButton, ToggleButtonGroup } from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import { DashboardLayout } from 'shared/types/dtos/configurablePages.dto';

import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { SITE_DIAGRAM_URL } from 'src/pages/ConfigurablePages/Dashboard/dashboard.constants';
import CardDashboard from './CardDashboard';
import TableDashboard from './TableDashboard';
import { dashboardBoxSx } from './dashboard.styles';

const Dashboard = () => {
  const storedLayout: DashboardLayout = (localStorage.getItem('dashboardLayout') as DashboardLayout) ?? DashboardLayout.CARD;
  const [layout, setLayout] = useState<DashboardLayout>(storedLayout);

  const axiosInstance = useAxiosWebUIInstance();

  // TODO: move into site diagram page once it's created
  const getInitialData = async () => {
    axiosInstance.get(SITE_DIAGRAM_URL).then((res) => {
      console.log(res.data);
    });
  };

  useEffect(() => {
    getInitialData();
  }, []);

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
