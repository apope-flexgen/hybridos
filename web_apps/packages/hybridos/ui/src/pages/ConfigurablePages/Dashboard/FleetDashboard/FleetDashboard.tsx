import { Box, Typography } from '@flexgen/storybook';
import TableDashboard from 'src/pages/ConfigurablePages/Dashboard/TableDashboard';
import { tableBoxSx } from './FleetDashboard.styles';

const FleetDashboard = () => (
  <Box sx={tableBoxSx}>
    <Typography text="Fleet Overview" variant="bodyXLBold" sx={{ paddingBottom: '20px' }} />
    <TableDashboard />
  </Box>
);

export default FleetDashboard;
