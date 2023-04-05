// TODO: fix lint
/* eslint-disable react/no-array-index-key */
import { Drawer } from '@flexgen/storybook';
import { useState } from 'react';
import NavigationDrawerTab from './NavigationDrawerTab';
import { RouteProps, RoutesProps } from './Routes';

const NavigationDrawer = ({
  routes,
}: Omit<RoutesProps, 'pageDictionary' | 'currentUser' | 'product'>): JSX.Element => {
  const [open, setOpen] = useState(true);
  // TODO: It's nicer UX to have drawer open/close on mouseover vs explicitly clicking:
  //  TODO: Should fix in component vs haq
  return (
    <div>
      <Drawer handleOpenClose={() => setOpen(!open)} open={open}>
        {routes.map(({ icon, itemName, path }: RouteProps, i) => (
          <NavigationDrawerTab
            icon={icon}
            itemName={itemName}
            key={i}
            open={open}
            path={path}
          />
        ))}
      </Drawer>
    </div>
  );
};

export default NavigationDrawer;
