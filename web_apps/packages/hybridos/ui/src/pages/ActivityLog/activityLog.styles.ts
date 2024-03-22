import { ThemeType } from '@flexgen/storybook';

export const mainContentBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'flex-start',
  flexDirection: 'column',
  paddingLeft: '24px',
  paddingRight: '24px',
  paddingTop: '12px',
  borderRadius: '8px',
  height: '100%',
  overflowY: 'auto',
  width: '100%',
  boxShadow: theme.fgb.pageHeader.sizing.boxShadow,
  backgroundColor: theme.fgc.table.color.background,
});

export const tabBoxSx = {
  display: 'flex',
  paddingTop: '12px',
  height: '90%',
  width: '100%',
  borderRadius: '8px',
};

export const eventsBoxSx = (theme: ThemeType) => ({
  width: '100%', height: '100%', overflowY: 'auto', display: 'flex', boxShadow: theme.fgb.pageHeader.sizing.boxShadow, borderRadius: '8px',
});
