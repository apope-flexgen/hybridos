import { ThemeType } from '@flexgen/storybook';

export const generateSiteStatusSx = (theme: ThemeType) => ({
  paddingTop: '8px',
  backgroundColor: theme.fgc.statusBar.color.background,
});
