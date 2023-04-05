import { MuiButton, ThemeType } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React, { useState, useMemo } from 'react';
import { isDurationOver24Hours } from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import { checkIfStartBeforeEnd } from 'src/pages/Scheduler/SchedulerHelpers';
import { editEventLabels } from 'src/pages/Scheduler/SchedulerLabels';
import { EditEventState } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import ConfirmCancel from './ConfirmCancel';
import { createButtonBoxSx } from './EditEventModal-styles';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';

export interface SubmitDiscardDeleteProps {
  // handle deleting event, passed down from parent
  handleDelete: (series: boolean) => void
  // handle discarding edits to an event, passed down from parent
  handleDiscard: () => void
  // handle saving edits to an event, passed down from parent
  handleSave: () => void
  // whether the event clicked on is recurring
  recurring: boolean
  state: EditEventState
}

const SubmitDiscardDelete: React.FunctionComponent<SubmitDiscardDeleteProps> = ({
  handleDiscard,
  handleSave,
  handleDelete,
  recurring,
  state,
}) => {
  const theme = useTheme() as ThemeType;
  const buttonBoxSx = createButtonBoxSx(theme);
  const [deleteEventClicked, setDeleteEventClicked] = useState(false);
  const [deleteSeriesClicked, setDeleteSeriesClicked] = useState(false);
  
  const { disableAllFields } = useSchedulerContext();

  const saveDisabled = useMemo(() => isDurationOver24Hours(state)
          || checkIfStartBeforeEnd(state), [state]);

  return (
    <Box sx={buttonBoxSx}>
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          gap: theme.fgb.editModal.spacing.gap,
        }}
      >
        <Box sx={{ display: 'flex', gap: theme.fgb.editModal.spacing.gap }}>
          <MuiButton
            color={editEventLabels.deleteButton.color}
            disabled={disableAllFields}
            label={editEventLabels.deleteButton.eventLabel}
            onClick={() => {
              setDeleteEventClicked(true);
              setDeleteSeriesClicked(false);
            }}
            startIcon="Trash"
            variant={editEventLabels.deleteButton.variant}
          />
          {deleteEventClicked && (
            <ConfirmCancel
              cancelLabel={editEventLabels.confirmCancelButton.cancelLabel}
              confirmLabel={editEventLabels.confirmCancelButton.confirmLabel}
              onCancelClick={() => setDeleteEventClicked(false)}
              onConfirmClick={() => handleDelete(false)}
            />
          )}
        </Box>
        {recurring && (
        <Box sx={{ display: 'flex', gap: theme.fgb.editModal.spacing.gap }}>
          <MuiButton
            color={editEventLabels.deleteButton.color}
            disabled={disableAllFields}
            label={editEventLabels.deleteButton.recurringLabel}
            onClick={() => {
              setDeleteSeriesClicked(true);
              setDeleteEventClicked(false);
            }}
            startIcon="DeleteSeries"
            variant={editEventLabels.deleteButton.variant}
          />
          {deleteSeriesClicked && (
          <ConfirmCancel
            cancelLabel={editEventLabels.confirmCancelButton.cancelLabel}
            confirmLabel={editEventLabels.confirmCancelButton.confirmLabel}
            onCancelClick={() => setDeleteSeriesClicked(false)}
            onConfirmClick={() => handleDelete(true)}
          />
          )}
        </Box>
        )}
      </Box>

      <Box sx={{ display: 'flex', gap: theme.fgb.editModal.spacing.gap }}>
        <MuiButton
          disabled={disableAllFields}
          label={editEventLabels.discardButton.label}
          onClick={handleDiscard}
          variant={editEventLabels.discardButton.variant}
        />
        <MuiButton
          disabled={saveDisabled || disableAllFields}
          label={editEventLabels.saveButton.label}
          onClick={handleSave}
        />
      </Box>
    </Box>
  );
};

export default SubmitDiscardDelete;
