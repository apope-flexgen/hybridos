import { MuiButton, ThemeType } from '@flexgen/storybook';
import { Typography } from '@mui/material';
import { Box } from '@mui/system';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { schedulerLabels } from 'src/pages/Scheduler/SchedulerHelpers';
import { useTheme } from 'styled-components';

const UnconfiguredContainer = () => {
  const theme = useTheme() as ThemeType;
  const { admin, setSchedulerTab } = useSchedulerContext();

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        gap: '1rem',
        minHeight: '750px',
        width: '100%',
        flexGrow: 1,
      }}
    >
      <Typography
        sx={{
          color: theme.fgc.radio.color.text,
          fontFamily: theme.fgb.editModal.fonts.header.fontFamily,
          fontSize: theme.fgb.scheduler.padding,
        }}
      >
        {admin
          ? schedulerLabels.unconfiguredContainer.adminLabel
          : schedulerLabels.unconfiguredContainer.userLabel}
      </Typography>
      {admin && (
        <MuiButton
          color={schedulerLabels.unconfiguredContainer.adminButton.color}
          label={schedulerLabels.unconfiguredContainer.adminButton.label}
          onClick={() => {
            setSchedulerTab(schedulerLabels.schedulerConfigTab.value);
          }}
          size={schedulerLabels.unconfiguredContainer.adminButton.size}
        />
      )}
    </Box>
  );
};

export default UnconfiguredContainer;
