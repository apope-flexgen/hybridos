// TODO: fix lint
/* eslint-disable react/no-array-index-key */
import {
  Box, Divider, Drawer, darkTheme,
} from '@flexgen/storybook';
import { useState } from 'react';
import { ThemeProvider } from 'styled-components';
import NavigationDrawerTab from './NavigationDrawerTab';
import { RouteProps, RoutesProps } from './Routes';

const NavigationDrawer = ({
  routes,
}: Omit<RoutesProps, 'pageDictionary' | 'currentUser' | 'product'>): JSX.Element => {
  const [open, setOpen] = useState(true);
  return (
    <Drawer handleOpenClose={() => setOpen(!open)} open={open}>
      {routes.map(({
        icon, itemName, path, showDivider,
      }: RouteProps, i) => (showDivider ? (
        <ThemeProvider theme={darkTheme}>
          <Box sx={{ padding: '8px' }}>
            <Divider orientation="horizontal" variant="fullWidth" />
          </Box>
        </ThemeProvider>
      ) : (
        <NavigationDrawerTab icon={icon} itemName={itemName} key={i} open={open} path={path} />
      )))}
    </Drawer>
  );
};

export default NavigationDrawer;
