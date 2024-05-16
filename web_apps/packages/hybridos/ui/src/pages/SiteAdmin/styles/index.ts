import { ThemeType, customMUIScrollbar } from '@flexgen/storybook';

export const connectionRadiusBoxSx = {
  paddingTop: '16px',
};

export const containerBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '12px',
  width: '100%',
};

export const buttonBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '8px',
  alignItems: 'flex-end',
};

export const saveCancelBoxSx = { display: 'flex', gap: '16px' };

export const connectionMethodBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  padding: '24px',
  flexDirection: 'column',
  gap: '8px',
  backgroundColor: theme.fgd.background.paper,
});

export const contentBoxSx = (theme: ThemeType) => ({
  flexGrow: 1,
  padding: '24px',
  backgroundColor: theme.fgd.background.paper,
  height: '100%',
  overflowY: 'auto',
  ...customMUIScrollbar(theme),
});

export const titleBoxSx = {
  width: '100%',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
};

export const radiusSettingsContainerSx = (theme: ThemeType) => ({
  display: 'flex',
  gap: '16px',
  flexDirection: 'column',
  alignItems: 'center',
  justifyContent: 'flex-start',
  width: '60%',
  padding: '24px',
  backgroundColor: `${theme.fgd.primary.main}12`,
});

export const radiusTtitleSx = {
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  width: '100%',
};

export const radiusTestRequiredBadgeSx = { width: '10%', whiteSpace: 'nowrap' };

export const radiusCardsContainerSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '16px',
  width: '100%',
};
