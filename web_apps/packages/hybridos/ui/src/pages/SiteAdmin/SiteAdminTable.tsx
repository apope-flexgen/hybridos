import { Box, CardRow, Typography } from '@flexgen/storybook';
import React, { FC, ReactNode } from 'react';
import SiteAdminRow from './SiteAdminRow';

export interface SiteAdminTableProps {
  children: ReactNode;
  title: string;
}

const SiteAdminTable: FC<SiteAdminTableProps> = ({ children, title }) => (
  <>
    <CardRow alignItems="center" justifyContent="flex-start">
      <Typography text={title} variant="headingS" />
    </CardRow>
    {React.Children.map(children, (child) => (
      <SiteAdminRow>{child}</SiteAdminRow>
    ))}
    <Box sx={{ paddingBottom: '24px' }} />
  </>
);

export default SiteAdminTable;
