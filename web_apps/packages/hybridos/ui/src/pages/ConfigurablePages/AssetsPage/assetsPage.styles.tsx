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

export const viewEventsButtonBoxSx = { flexGrow: 1, display: 'flex', flexDirection: 'row-reverse' };

export const expandButtonBoxSx = {
  position: 'absolute',
  bottom: 0,
  right: 0,
  zIndex: 4,
};

export const getControlOuterBoxSx = (theme: ThemeType) => ({
  overflowY: 'auto',
  width: '20%',
  height: '100%',
  backgroundColor: theme.fgd.primary.main_12p,
  position: 'absolute',
  top: 0,
  right: 0,
  padding: '15px',
});

export const getControlInnerBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  gap: '10px',
  backgroundColor: theme.fgd.background.paper,
  padding: '12px',
  borderRadius: '2px',
});

export const statusOuterBoxSx = {
  width: '100%',
  minHeight: '100%',
  display: 'flex',
  flexDirection: 'column',
  overflow: 'auto',
};

export const getAllControlsBoxSx = (theme: ThemeType) => ({
  marginBottom: '16px',
  backgroundColor: theme.fgd.background.paper,
  padding: '8px 16px 8px 16px',
  borderRadius: '2px',
})

export const getAllControlsHeaderBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
})

export const getAllControlsInnerBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  flexDirection: 'column',
  gap: '10px'
})

export const statusPointsDisplayOuterBoxSx = { display: 'flex', flexDirection: 'column' };

export const statusPointsDisplaySubheaderBoxSx = { marginTop: '1rem', marginBottom: '1rem' };
