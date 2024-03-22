import { ThemeType } from '@flexgen/storybook';

export const mainContentBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'flex-start',
  flexDirection: 'column',
  width: '100%',
  boxShadow: theme.fgb.pageHeader.sizing.boxShadow,
  backgroundColor: theme.fgc.table.color.background,
});

export const titleBoxSx = {
  width: '100%',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
};

export const systemStatusTableSx = {
  display: 'flex',
  flexDirection: 'column',
  minWidth: '350px',
  padding: '0px 24px 24px 24px',
  flex: 1,
  gap: '16px',
  maxHeight: '100%',
};

export const tableBoxSx = { height: '85%', width: '100%', overflowY: 'hidden' };
export const headerFiltersRow = { boxShadow: 'none', height: '30%', alignItems: 'flex-start' };
export const interiorHeaderFiltersBox = { marginTop: '16px', width: '100%' };
export const filterBoxSx = { display: 'flex', gap: '8px' };
export const iconSize = { width: '30px', height: '30px' };
export const actionBoxSx = { display: 'flex', gap: '6px', alignItems: 'center' };
export const sortedTableHeaderSx = { display: 'flex', gap: '2px', alignItems: 'center' };
export const headerBoxSx = { width: '100%', overflowY: 'auto' };
export const summaryStatusSx = { display: 'flex', flexDirection: 'column' };
export const summaryStatusBoxSx = { display: 'flex', alignItems: 'flex-end', gap: '20px' };
