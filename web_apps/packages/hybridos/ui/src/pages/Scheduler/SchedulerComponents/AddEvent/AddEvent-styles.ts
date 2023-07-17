import { ThemeType } from '@flexgen/storybook';

export const fullBoxSx = (theme: ThemeType) => (
  {
    width: '320px',
    display: 'flex',
    flexDirection: 'column',
    gap: theme.fgb.editModal.spacing.padding,
    padding: theme.fgb.editModal.spacing.padding,
  }
);

export const buttonsAndErrorsSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '8px',
  alignItems: 'flex-end',
  width: '100%',
};

export const repeatEverySx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',
};
