import { ThemeType } from '@flexgen/storybook';

export const datePickerSx = { width: '150px' };

export const mainContentBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'flex-start',
  flexDirection: 'column',
  width: '100%',
  boxShadow: theme.fgb.pageHeader.sizing.boxShadow,
  backgroundColor: theme.fgc.table.color.background,
});

export const headerBoxSx = { height: '20%', width: '100%', overflowY: 'auto' };

export const tableBoxSx = { height: '80%', width: '100%', overflowY: 'hidden' };
