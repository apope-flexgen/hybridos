import {
  CardRow, Typography, CardContainer, InfoDisplay, Divider,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import FilterSearchBar from 'src/pages/SystemStatus/FilterSearchBar/FilterSearchBar';
import {
  headerFiltersRow, interiorHeaderFiltersBox, summaryStatusBoxSx, summaryStatusSx,
} from 'src/pages/SystemStatus/SystemStatus.styles';
import { SystemStatusHeaderProps } from 'src/pages/SystemStatus/SystemStatus.types';

const SystemStatusHeader = ({
  systemStatusData,
  summarySystemData,
  setDisplayData,
}: SystemStatusHeaderProps) => (
  <CardRow styleOverrides={headerFiltersRow}>
    <Box sx={interiorHeaderFiltersBox}>
      <CardRow justifyContent="space-between">
        <Typography text="System Status" variant="headingM" />
        <Box sx={summaryStatusBoxSx}>
          <Box sx={summaryStatusSx}>
            <InfoDisplay helper="System Uptime" header={summarySystemData.uptime as string || '-'} color="primary" />
          </Box>
          <Box sx={summaryStatusSx}>
            <InfoDisplay helper="Last Restart" header={summarySystemData.lastRestart as string || '-'} color="primary" />
          </Box>
          <Box sx={summaryStatusSx}>
            <InfoDisplay helper="CPU Usage" header={summarySystemData.cpuUsage ? `${summarySystemData.cpuUsage.toFixed(1)}%` : '-'} color="primary" />
          </Box>
          <Box sx={summaryStatusSx}>
            <InfoDisplay helper="Memory Usage" header={summarySystemData.memoryUsage ? `${summarySystemData.memoryUsage.toFixed(1)}%` : '-'} color="primary" />
          </Box>
        </Box>
      </CardRow>
      <Divider orientation="horizontal" variant="middle" />
      <CardContainer styleOverrides={{ boxShadow: 'none', paddingTop: '15px' }}>
        <FilterSearchBar systemStatusData={systemStatusData} setDisplayData={setDisplayData} />
      </CardContainer>
    </Box>
  </CardRow>
);

export default SystemStatusHeader;
