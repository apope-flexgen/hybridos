import { Typography } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React from 'react';
import { schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';
import { siteInformationStyles as styles } from 'src/pages/Scheduler/ModeManager/Styles';

/** FIXME: should take out these props. pull from hooks instead */
interface SiteInformationProps {
  schedulerType: 'SC' | 'FM' | null;
  siteName: string | undefined;
}

const SiteInformation: React.FC<SiteInformationProps> = ({
  siteName,
  schedulerType,
}: SiteInformationProps) => (
  <Box sx={styles.outterBox}>
    <Typography variant="headingM" text={siteName || ''} />
    <Typography
      variant="tooltip"
      text={schedulerType === 'SC' ? labels.siteInformation.sc : labels.siteInformation.fm}
    />
  </Box>
);

export default SiteInformation;
