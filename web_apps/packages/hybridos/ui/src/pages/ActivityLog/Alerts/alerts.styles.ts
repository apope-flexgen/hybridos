import { ThemeType } from '@flexgen/storybook';

export const alertsTableSx = {
  display: 'flex',
  flexDirection: 'column',
  minWidth: '350px',
  padding: '0px 24px 24px 24px',
  flex: 1,
  gap: '16px',
  maxHeight: '100%',
};

export const headerBoxSx = {
  width: '100%',
};

export const tableBoxSx = {
  width: '100%',
  overflowY: 'hidden',
  height: '100%',
  padding: '16px',
};

export const dataTableBox = { width: '100%', height: '100%', overflowY: 'hidden' };

export const expandedRowBoxSx = {
  display: 'flex', flexDirection: 'column', width: '100%', gap: '8px', padding: '2.5rem 0.5rem',
};
export const expandedRowContentSx = {
  display: 'flex', flexDirection: 'column', gap: '2px',
};

export const resolveActiveAlertBannerSx = (theme: ThemeType) => ({
  display: 'flex',
  padding: '15px',
  marginBottom: '20px',
  borderRadius: '8px',
  backgroundColor: theme.fgc.table.color.alarmLight,
});

export const alertBannerIconSx = {
  display: 'flex',
  alignItems: 'center',
  paddingRight: '15px',
};

export const alertBannerMessageSx = {
  display: 'flex',
  flexDirection: 'column',
};
