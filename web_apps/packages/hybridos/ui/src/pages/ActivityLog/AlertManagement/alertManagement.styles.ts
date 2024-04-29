import { ThemeType } from '@flexgen/storybook';

export const sitesBoxSx = {
  display: 'flex',
  maxWidth: '300px',
  overflowX: 'auto',
  paddingBottom: '4px',
  '&::-webkit-scrollbar': { height: '4px' },
};

export const actionsBoxSx = { display: 'flex', gap: '8px' };

export const extraPadding = { paddingTop: '16px', height: 'auto' };

export const externalFormRowBoxSx = {
  display: 'flex', flexDirection: 'column', gap: '20px', paddingBottom: '20px', alignItems: 'center',
};
export const formRowSx = {
  display: 'flex', width: '100%', gap: '32px', alignItems: 'flex-start',
};
export const formRowTitleAndDescriptionSx = {
  display: 'flex', width: '25%', flexDirection: 'column', gap: '2px',
};

export const templateRowTitleAndDescriptionSx = {
  display: 'flex', width: '40%', flexDirection: 'column', gap: '8px',
};
export const formRowContentsSx = {
  display: 'flex', width: '75%', gap: '16px', alignItems: 'center', minHeight: '60px',
};
export const templateRowBoxSx = { display: 'flex', alignItems: 'flex-start', gap: '16px' };

export const accordionRowsSx = { display: 'flex', flexDirection: 'column', gap: '0px' };
export const templateFieldsBoxSx = {
  display: 'flex', width: '75%', flexDirection: 'column', gap: '12px', alignItems: 'flex-start',
};

export const templateSwitchSx = { display: 'flex', alignItems: 'center', gap: '4px' };

export const ruleBlockBoxSx = (theme: ThemeType) => ({
  display: 'flex', padding: '24px', flexDirection: 'column', gap: '24px', backgroundColor: theme.fgd.primary.main_8p, width: '80%',
});
export const setWidth = (width: number | string) => ({
  width: typeof width === 'string' ? width : `${width}px`,
});

export const messageBoxSx = {
  width: '80%', display: 'flex', flexDirection: 'column', gap: '4px',
};

export const durationRowSx = { display: 'flex', gap: '8px', alignItems: 'center' };

export const templateToFromBoxSx = { display: 'flex', gap: '16px', width: '220px' };
export const expressionRowSx = { display: 'flex', flexDirection: 'column', gap: '16px' };
export const comparator1RowSx = { width: '100%', display: 'flex', justifyContent: 'space-between' };
export const saveDisabledErrorBoxSx = {
  display: 'flex', flexDirection: 'column', gap: '4px', alignItems: 'flex-end',
};

export const rightAlignedButton = { display: 'flex', width: '100%', justifyContent: 'flex-end' };
