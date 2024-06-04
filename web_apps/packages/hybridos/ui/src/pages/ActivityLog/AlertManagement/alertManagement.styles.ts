/* eslint-disable max-lines */
import { ThemeType, customMUIScrollbar } from '@flexgen/storybook';

export const sitesBoxSx = {
  display: 'flex',
  maxWidth: '300px',
  overflowX: 'auto',
  paddingBottom: '4px',
  '&::-webkit-scrollbar': { height: '4px' },
};

export const actionsBoxSx = { display: 'flex', gap: '8px' };

export const extraPadding = { paddingTop: '0px', height: 'auto' };

export const alertTemplateValueBox = {
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',
  width: 'max-content',
};

export const externalFormRowBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '20px',
  paddingBottom: '20px',
  alignItems: 'center',
};
export const formRowSx = {
  display: 'flex',
  width: '100%',
  gap: '32px',
  alignItems: 'flex-start',
};
export const formRowTitleAndDescriptionSx = {
  display: 'flex',
  width: '25%',
  flexDirection: 'column',
  gap: '2px',
};

export const displayFlex = { display: 'flex' };

export const templateRowTitleAndDescriptionSx = {
  display: 'flex',
  width: '40%',
  flexDirection: 'column',
  gap: '8px',
};
export const formRowContentsSx = {
  display: 'flex',
  width: '75%',
  gap: '16px',
  alignItems: 'flex-start',
  paddingTop: '12px',
  minHeight: 'max-content',
};

export const formRowContentVerticalSx = {
  display: 'flex',
  width: '75%',
  gap: '8px',
  flexDirection: 'column',
  alignItems: 'flex-start',
};
export const templateRowBoxSx = {
  display: 'flex',
  alignItems: 'flex-start',
  gap: '16px',
};

export const aliasRowBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  width: 'max-content',
  gap: '8px',
  padding: '16px',
  border: `1px solid ${theme.fgd.other.divider}`,
  backgroundColor: `${theme.fgd.primary.main}08`,
  borderRadius: '4px',
});

export const accordionRowsSx = { display: 'flex', flexDirection: 'column', gap: '0px' };
export const templateFieldsBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  width: '75%',
  gap: '12px',
  alignItems: 'flex-start',
};

export const templateSwitchSx = { display: 'flex', alignItems: 'center', gap: '4px' };

export const ruleBlockBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  padding: '24px',
  flexDirection: 'column',
  gap: '24px',
  width: 'max-content',
  border: `1px solid ${theme.fgd.other.divider}`,
  backgroundColor: `${theme.fgd.primary.main}12`,
});

export const setWidth = (width: number | string) => ({
  width: typeof width === 'string' ? width : `${width}px`,
});

export const messageBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  width: 'max-content',
  flexDirection: 'column',
  gap: '8px',
  padding: '16px',
  border: `1px solid ${theme.fgd.other.divider}`,
  borderRadius: '4px',
});

export const durationRowSx = { display: 'flex', gap: '8px', alignItems: 'center' };

export const templateToFromBoxSx = { display: 'flex', gap: '16px', width: '220px' };
export const expressionRowSx = { display: 'flex', flexDirection: 'column', gap: '16px' };
export const comparator1RowSx = {
  width: '100%',
  display: 'flex',
  justifyContent: 'space-between',
};
export const saveDisabledErrorBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '4px',
  alignItems: 'flex-end',
};

export const rightAlignedButton = {
  display: 'flex',
  width: '100%',
  justifyContent: 'flex-end',
};

export const messageFieldsBoxSx = { display: 'flex', gap: '8px', width: '100%' };

export const orgRowSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  gap: '8px',
  alignItems: 'flex-start',
  width: '100%',
  maxHeight: '250px',
  overflowY: 'auto',
  ...customMUIScrollbar(theme),
});
