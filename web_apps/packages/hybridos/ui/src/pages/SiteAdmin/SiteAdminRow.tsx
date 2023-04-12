import { Box, CardRow, Divider } from '@flexgen/storybook';
import { FC, ReactNode } from 'react';

export interface SiteAdminRowProps {
  children: ReactNode;
}

const SiteAdminRow: FC<SiteAdminRowProps> = ({ children }) => (
  <>
    <CardRow alignItems="center" justifyContent="space-between" width="100%">
      {children}
    </CardRow>
    <Divider orientation="horizontal" variant="fullWidth" />
    <Box sx={{ paddingBottom: '16px' }} />
  </>
);

export default SiteAdminRow;
