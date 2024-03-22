import { ThemeType } from '@flexgen/storybook';

export const datePickerSx = { width: '150px' };

export const mainContentBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'flex-start',
  flexDirection: 'column',
  width: '100%',
  backgroundColor: theme.fgc.table.color.background,
  borderRadius: '8px',
});

export const headerBoxSx = {
  maxHeight: '175px', width: '100%', overflowY: 'auto', height: '100%',
};

export const tableBoxSx = {
  height: '80%',
  width: '100%',
  overflowY: 'hidden',
  padding: '0px 16px 16px 16px',
};
