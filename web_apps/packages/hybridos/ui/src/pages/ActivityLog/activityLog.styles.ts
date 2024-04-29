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

export const alertModalSx = {
  display: 'flex',
  flexDirection: 'column',
  padding: '0px 24px',
  gap: '10px',
  width: '400px',
};

export const alertBubbleSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  border: `1px solid ${theme.fgd.primary.main_50p}`,
  backgroundColor: theme.fgd.primary.main_12p,
  borderRadius: 50,
  minWidth: '24px',
  minHeight: '24px',
  paddingLeft: '4px',
  paddingRight: '4px',
});

export const textFieldSx = { padding: '5px 0px' };

export const modalButtonsSx = {
  display: 'flex',
  gap: '10px',
  paddingTop: '5px',
};

export const cancelButtonSx = { flex: '0 0 25%' };

export const saveButtonSx = { flex: '1' };

export const expandedRowSx = { display: 'flex', gap: '2px' };
