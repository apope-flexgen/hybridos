import { Box, Typography, ThemeType } from '@flexgen/storybook';
import { useTheme } from 'styled-components';

const UnconfiguredContainer = () => {
  const theme = useTheme() as ThemeType;

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        gap: '1rem',
        minHeight: '500px',
        width: '100%',
        flexGrow: 1,
        backgroundColor: theme.fgd.primary.main_30p,
      }}
    >
      <Typography text="Select a site to view and edit overrides" variant="bodyL" />
    </Box>
  );
};

export default UnconfiguredContainer;
