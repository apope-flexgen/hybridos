import { MuiButton, ThemeType } from '@flexgen/storybook';
import { Box } from '@mui/material';
import React from 'react';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { ButtonSizes, ButtonVariants } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import { addEventLabels } from './AddEventHelpers';

interface AddClearProps {
  handleClear?: () => void;
  handleAddEvent: () => void;
  addDisabled: boolean;
}

const AddClearButons: React.FC<AddClearProps> = ({
  handleClear,
  handleAddEvent,
  addDisabled,
}: AddClearProps) => {
  const theme = useTheme() as ThemeType;
  const { disableAllFields } = useSchedulerContext();

  return (
    <Box
      sx={{
        display: 'flex',
        gap: theme.fgb.editModal.spacing.gap,
        justifyContent: 'flex-end',
      }}
    >
      <MuiButton
        disabled={disableAllFields}
        label={addEventLabels.clearButton.label}
        onClick={handleClear}
        size={addEventLabels.clearButton.size as ButtonSizes}
        variant={addEventLabels.clearButton.variant as ButtonVariants}
      />
      <MuiButton
        disabled={addDisabled || disableAllFields}
        label={addEventLabels.addEventButton.label}
        onClick={handleAddEvent}
        size={addEventLabels.addEventButton.size as ButtonSizes}
        startIcon="Add"
      />
    </Box>
  );
};

export default AddClearButons;
