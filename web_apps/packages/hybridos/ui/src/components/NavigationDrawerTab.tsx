// TODO: fix lint
/* eslint-disable react/no-unused-prop-types */
import { DrawerTab } from '@flexgen/storybook';
import { useNavigate, useLocation } from 'react-router-dom';
import { RouteProps } from './Routes';

export interface DrawerTabProps extends Omit<RouteProps, 'componentName'> {
  /** whether this tab is currently selected */
  isSelected?: boolean
  /** holds whether the tab is currently open */
  open: boolean
  /** navigate, usually on click  */
  handleClick?: (path: string) => void
  /** whether to show a divider */
  showDivider?: boolean
}

// TODO: Export interface from DrawerTab component so it can be reused here
const NavigationDrawerTab = ({
  icon, itemName, open, path,
}: DrawerTabProps): JSX.Element => {
  const navigate = useNavigate();
  const handleClick = (route: string) => {
    navigate(route);
  };
  const location = useLocation();
  return (
    <DrawerTab
      handleClick={(route) => handleClick(route)}
      icon={icon}
      isSelected={path === location.pathname}
      itemName={itemName}
      open={open}
      path={path}
    />
  );
};

export default NavigationDrawerTab;
