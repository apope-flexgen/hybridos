import { IconList } from '@flexgen/storybook';

export const getDashboardIcon = (itemName: string): IconList => {
  switch (itemName) {
    case 'Site Summary':
      return 'Site';
    case 'Features Summary':
      return 'SiteControls';
    case 'Storage Summary':
      return 'Storage';
    case 'Solar Summary':
      return 'Sun';
    case 'Feeder Summary':
      return 'Feeders';
    case 'Generator Summary':
      return 'Storage';

    default:
      return 'Storage';
  }
};

export const getDataPointColumns = (numDataPoints: number): number => {
  if (numDataPoints < 8) {
    return 12;
  }
  if (numDataPoints <= 12) {
    return 6;
  }
  return 4;
};

export const getCardColumns = (numDataPoints: number): number | boolean => {
  if (numDataPoints < 10) {
    return true;
  }
  if (numDataPoints <= 12) {
    return 4;
  }
  return 6;
};
