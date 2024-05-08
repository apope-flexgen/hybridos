import { ThemeType } from '@flexgen/storybook';

export const cardContainerSx = {
  alignItems: 'center',
  justifyContent: 'center',
  height: '100%',
  width: '100%',
  padding: 24,
};

export const nodeBoxSx = (theme: ThemeType) => ({
  // TODO: remove hardcoded dimensions once layouting works with dyanmic sizing; will be completed as part of DC-505
  width: '200px',
  height: '50px',
  background: theme.fgd.background.paper,
  border: `2px solid ${theme.fgd.other.outline_border}`,
  borderRadius: '8px',
  padding: '10px',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  gap: '5px',
});

export const edgeSx = (theme: ThemeType) => ({
  background: theme.fgd.status.dark,
  stroke: theme.fgd.status.dark,
});
