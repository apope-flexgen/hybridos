import { ThemeType } from '@flexgen/storybook';

export const rowSx = {
  display: 'flex',
  flexDirection: 'row',
  alignItems: 'center',
  justifyContent: 'space-evenly',
  gap: '12px',
  width: '100%',
  flexGrow: 1,
};

export const passwordReqsSx = {
  width: '30%',
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'left',
};

export const pageSx = {
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '12px',
  width: '100%',
  flexGrow: 1,
  height: '100%',
  marginBottom: '48px',
};

export const titleBoxSx = {
  width: '100%',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
};

export const dataTableSx = {
  display: 'flex',
  padding: '24px',
  gap: '15px',
  flexDirection: 'column',
  width: '100%',
  height: '100%',
};

export const nonDataTableSx = {
  paddingLeft: '12px',
};

export const userRowSx = {
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  padding: '12px 12px 12px 3%',
  gap: '12px',
  width: '100%',
  flexGrow: 1,
};

export const userRowButtonsSx = {
  display: 'flex',
  flexDirection: 'row',
  alignItems: 'center',
  marginRight: 'auto',
  width: '100%',
  gap: '12px',
  flexGrow: 1,
};

export const addUserRowSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '12px',
  padding: '12px',
  width: '100%',
  flexGrow: 1,
  backgroundColor: theme.fgc.dataTable.color.headerSecondaryBg,
  border: `1px solid ${theme.fgc.dataTable.color.borderColor}`,
  borderRadius: '4px',
});

export const addUserRowButtonsSx = {
  display: 'flex',
  flexDirection: 'row',
  alignItems: 'left',
  gap: '12px',
  width: '100%',
  flexGrow: 1,
  justifyContent: 'flex-end',
  paddingRight: '24px',
};

export const dataTableAndTypographySx = {
  display: 'flex',
  minHeight: '40%',
  width: '100%',
  justifyContent: 'space-between',
};

export const addUserAndTypographySx = {
  display: 'flex',
  maxHeight: '50%',
  width: '100%',
  justifyContent: 'space-between',
};

export const usersTablePaperSx = (theme: ThemeType) => ({
  height: 'fit-content',
  maxHeight: '100%',
  border: `1px solid ${theme.fgc.dataTable.color.borderColor}`,
  boxShadow: 'none',
});
