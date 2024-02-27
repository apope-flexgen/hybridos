/* eslint-disable max-lines */
import { ThemeType } from '@flexgen/storybook';
import { Box } from '@mui/material';

export const FadeGradient = ({
  boxColor,
  boxColorTransparent,
  paperColor,
  paperColorTransparent,
}: {
  boxColor: string;
  boxColorTransparent: string;
  paperColor: string;
  paperColorTransparent: string;
}) => (
  <>
    <Box
      sx={{
        height: '50%',
        width: '100%',
        position: 'absolute',
        bottom: 0,
        left: 0,
        zIndex: 3,
        backgroundImage: `linear-gradient(${boxColorTransparent}, ${boxColor})`,
      }}
    />
    <Box
      sx={{
        height: '50%',
        width: '100%',
        position: 'absolute',
        bottom: 0,
        left: 0,
        backgroundImage: `linear-gradient(${paperColorTransparent}, ${paperColor})`,
        zIndex: 2,
      }}
    />
  </>
);

export const getOuterBoxSx = (expandable: boolean, expanded: boolean, boxColor: string) => ({
  position: 'relative',
  display: 'flex',
  flexDirection: 'column',
  backgroundColor: boxColor,
  borderRadius: '6px',
  width: '100%',
  overflow: expandable && !expanded ? 'hidden' : 'auto',
  maxHeight: expandable && !expanded ? '10rem' : 'none',
  cursor: 'pointer',
});

export const headerBoxSx = {
  display: 'flex',
  flexDirection: 'row',
  alignItems: 'center',
  margin: '1rem',
  gap: '1rem',
  marginBottom: '0',
};

export const alertContainerSx = {
  display: 'flex',
  flexDirection: 'row',
  gap: '2rem',
  marginTop: '1rem',
};

export const getTabsContainerSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  width: 'min-content',
  backgroundColor: theme.fgd.primary.main_12p,
  height: '100%',
});

export const tabsAndStatusContainerSx = {
  display: 'flex', flexDirection: 'row', height: '100%', padding: '1.5rem',
};

export const internalTabsAndStatusContainerSx = {
  display: 'flex',
  flexDirection: 'row',
  height: '1px',
  minHeight: '100%',
  width: '100%',
};

export const viewEventsButtonBoxSx = { flexGrow: 1, display: 'flex', flexDirection: 'row-reverse' };

export const expandButtonBoxSx = {
  position: 'absolute',
  bottom: 0,
  right: 0,
  zIndex: 4,
};

export const getControlOuterBoxSx = (theme: ThemeType) => ({
  overflowY: 'auto',
  width: '100%',
  height: '100%',
  backgroundColor: theme.fgd.primary.main_12p,
  top: 0,
  right: 0,
  padding: '15px',
  paddingBottom: '52px',
});

export const getControlInnerBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',
  backgroundColor: theme.fgd.background.paper,
  padding: '12px',
  borderRadius: '2px',
});

export const getStatusOuterBoxSx = (theme: ThemeType) => ({
  width: '100%',
  minHeight: '100%',
  display: 'flex',
  padding: '24px',
  flexDirection: 'column',
  backgroundColor: theme.fgc.pageHeader.color.background,
});

export const getAllControlsBoxSx = (theme: ThemeType) => ({
  marginBottom: '16px',
  backgroundColor: theme.fgd.background.paper,
  padding: '8px 16px 8px 16px',
  borderRadius: '2px',
});

export const getAllControlsHeaderBoxSx = () => ({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
});

export const getAllControlsInnerBoxSx = () => ({
  display: 'flex',
  flexDirection: 'column',
  gap: '10px',
});

export const statusAndMaintenanceActionsBoxSx = { display: 'flex', flexDirection: 'column', gap: '10px' };

export const statusPadding = { padding: '20px 15px 20px 15px' };

export const statusPointsDisplayOuterBoxSx = { display: 'flex', flexDirection: 'column' };

export const statusPointsDisplaySubheaderBoxSx = { marginBottom: '12px', width: '100%', padding: '20px 15px 0px 15px' };

export const maintenanceActionsBoxSx = {
  display: 'flex', flexDirection: 'column', gap: '12px', width: '100%', padding: '24px 16px 24px 16px',
};

export const batchActionsOuterBoxSx = { display: 'flex', flexDirection: 'column', gap: '12px' };
export const batchActionsTextBoxSx = { display: 'flex', flexDirection: 'column', gap: '4px' };
export const assetControlOuterBoxSx = {
  margin: '5px', display: 'flex', flexDirection: 'column', gap: '8px',
};
