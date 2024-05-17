import {
  Box, ToggleButton, ToggleButtonGroup, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { DashboardLayout } from 'shared/types/dtos/configurablePages.dto';
import { useAppContext } from 'src/App/App';
import { SITE_CONTROLLER } from 'src/components/BaseApp';
import CardDashboard from './CardDashboard';
import DiagramDashboard from './DiagramDashboard';
import TableDashboard from './TableDashboard';
import { dashboardBoxSx, titleButtonBoxSx } from './dashboard.styles';

const Dashboard = () => {
  const { product, siteConfiguration } = useAppContext();

  const validLayouts = product === SITE_CONTROLLER
    ? [
      DashboardLayout.TABLE,
      DashboardLayout.CARD,
      ...(siteConfiguration?.site_diagram ? [DashboardLayout.DIAGRAM] : []),
    ]
    : [
      DashboardLayout.TABLE,
      ...(siteConfiguration?.site_diagram ? [DashboardLayout.DIAGRAM] : []),
    ];
  // eslint-disable-next-line max-len
  const defaultLayout: DashboardLayout = product === SITE_CONTROLLER ? DashboardLayout.CARD : DashboardLayout.TABLE;
  const storedLayout: DashboardLayout = (localStorage.getItem('dashboardLayout') as DashboardLayout) || defaultLayout;
  const [layout, setLayout] = useState<DashboardLayout>(
    validLayouts.includes(storedLayout) ? storedLayout : defaultLayout,
  );

  const handleLayout = (e: React.MouseEvent<HTMLElement, MouseEvent>, value: DashboardLayout) => {
    setLayout(value);
    localStorage.setItem('dashboardLayout', value);
  };

  const showToggleButtons = product === SITE_CONTROLLER || siteConfiguration?.site_diagram;

  return (
    <Box sx={dashboardBoxSx}>
      <Box sx={titleButtonBoxSx}>
        <Typography text="Dashboard" variant="bodyXLBold" />
        {showToggleButtons && (
          <ToggleButtonGroup size="small" value={layout} exclusive required onChange={handleLayout}>
            <ToggleButton value={DashboardLayout.TABLE}>Table</ToggleButton>
            {product === SITE_CONTROLLER && (
              <ToggleButton value={DashboardLayout.CARD}>Card</ToggleButton>
            )}
            {siteConfiguration?.site_diagram && (
              <ToggleButton value={DashboardLayout.DIAGRAM}>Diagram</ToggleButton>
            )}
          </ToggleButtonGroup>
        )}
      </Box>
      {layout === DashboardLayout.TABLE && <TableDashboard />}
      {layout === DashboardLayout.CARD && <CardDashboard />}
      {siteConfiguration?.site_diagram && layout === DashboardLayout.DIAGRAM && (
        <DiagramDashboard />
      )}
    </Box>
  );
};

export default Dashboard;
