import { ThemeType } from '@flexgen/storybook';

export const cardContainerSx = {
  alignItems: 'center',
  justifyContent: 'center',
  height: '100%',
  width: '100%',
  padding: 24,
};

export const titleSelectSx = {
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  width: '100%',
};

export const diagramBoxSx = (theme: ThemeType, isLoading: boolean) => ({
  border: `1px solid ${theme.fgd.other.divider}`,
  borderRadius: '8px',
  width: '100%',
  height: '90%',
  opacity: isLoading ? 0 : 100,
});

export const boxSx = (isLoading: boolean) => ({
  width: '100%',
  height: '100%',
  opacity: isLoading ? 0 : 100,
});

export const contactorSx = {
  display: 'flex',
  gap: '8px',
  alignItems: 'center',
};

export const iconTextBoxSx = (statusComponentsLength: number) => ({
  display: 'flex',
  gap: '8px',
  padding: '8px',
  alignItems: 'center',
  paddingBottom: statusComponentsLength !== 0 ? '0px' : '8px',
});

export const statusesBoxSx = {
  width: '100%',
  display: 'flex',
  padding: '0px 10px 10px 10px',
  gap: '4px',
  flexDirection: 'column',
};

export const nodeBoxSx = (theme: ThemeType) => ({
  // TODO: remove hardcoded dimensions once layouting works with dyanmic sizing; will be completed as part of DC-505
  width: '225px',
  minHeight: '50px',
  background: theme.fgd.background.paper,
  border: `2px solid ${theme.fgd.other.outline_border}`,
  borderRadius: '8px',
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center',
  alignItems: 'center',
  gap: '8px',
});

export const edgeSx = (theme: ThemeType) => ({
  background: theme.fgd.status.dark,
  stroke: theme.fgd.status.dark,
});
