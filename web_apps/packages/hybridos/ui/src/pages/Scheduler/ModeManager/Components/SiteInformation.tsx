import { Typography } from '@mui/material';
import { Box } from '@mui/system';
import React from 'react';
import { schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';

/** FIXME: should take out these props. pull from hooks instead */
interface SiteInformationProps {
  schedulerType: 'SC' | 'FM' | null
  siteName: string | undefined;
}

const SiteInformation: React.FC<SiteInformationProps> = ({
  siteName,
  schedulerType,
}: SiteInformationProps) => (
  <Box sx={{ display: 'flex', flexDirection: 'row' }}>
    <Box sx={{ flexDirection: 'column' }}>
      <Typography variant="h1">{siteName}</Typography>
      <Typography variant="h2">
        {schedulerType === 'SC' ? labels.siteInformation.sc : labels.siteInformation.fm}
      </Typography>
    </Box>
  </Box>
);

export default SiteInformation;
