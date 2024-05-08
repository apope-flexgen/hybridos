/* eslint-disable */
// TODO: fix lint
import { MuiButton, ThemeType, Typography } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React, { useState, useMemo } from 'react';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import {
  isDurationOver24Hours,
  addEventLabels,
  isOverlappingStartTime,
} from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import { checkIfEndInPast, checkIfStartBeforeEnd } from 'src/pages/Scheduler/SchedulerHelpers';
import { editEventLabels } from 'src/pages/Scheduler/SchedulerLabels';
import { EditEventState } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import ConfirmCancel from './ConfirmCancel';
import { createButtonBoxSx } from './EditEventModal-styles';
import { useEventSchedulerContext } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import { SchedulerEvent } from 'shared/types/dtos/scheduler.dto';

export interface SubmitDiscardDeleteProps {
  // handle deleting event, passed down from parent
  handleDelete: (series: boolean) => void;
  // handle discarding edits to an event, passed down from parent
  handleDiscard: () => void;
  // handle saving edits to an event, passed down from parent
  handleSave: () => void;
  // whether the event clicked on is recurring
  recurring: boolean;
  state: EditEventState;
  pastEvent: boolean;
  event?: SchedulerEvent;
}

const SubmitDiscardDelete: React.FunctionComponent<SubmitDiscardDeleteProps> = ({
  handleDiscard,
  handleSave,
  handleDelete,
  recurring,
  state,
  pastEvent,
  event,
}) => {
  const theme = useTheme() as ThemeType;
  const buttonBoxSx = createButtonBoxSx(theme);
  const [deleteEventClicked, setDeleteEventClicked] = useState(false);
  const [deleteSeriesClicked, setDeleteSeriesClicked] = useState(false);

  const { disableAllFields, timezone } = useSchedulerContext();
  const { eventsForUi } = useEventSchedulerContext();

  const saveDisabled = useMemo(() => {
    return (
      isDurationOver24Hours(state) ||
      isOverlappingStartTime(state, eventsForUi, timezone[0] || 'America/New_York', event) ||
      checkIfEndInPast(state, timezone[0]) ||
      checkIfStartBeforeEnd(state)
    );
  }, [state]);

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
            disabled={disableAllFields || pastEvent}
            label={editEventLabels.deleteButton.eventLabel}
            onClick={() => {
              setDeleteEventClicked(true);
              setDeleteSeriesClicked(false);
            }}
            startIcon='Trash'
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
              disabled={disableAllFields || pastEvent}
              label={editEventLabels.deleteButton.recurringLabel}
              onClick={() => {
                setDeleteSeriesClicked(true);
                setDeleteEventClicked(false);
              }}
              startIcon='DeleteSeries'
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
      <Box
        sx={{
          display: 'flex',
          gap: '8px',
          flexDirection: 'column',
          alignItems: 'flex-end',
        }}
      >
        <Box sx={{ display: 'flex', gap: theme.fgb.editModal.spacing.gap }}>
          <MuiButton
            disabled={disableAllFields || pastEvent}
            label={editEventLabels.discardButton.label}
            onClick={handleDiscard}
            variant={editEventLabels.discardButton.variant}
          />
          <MuiButton
            disabled={saveDisabled || disableAllFields || pastEvent}
            label={editEventLabels.saveButton.label}
            onClick={handleSave}
          />
        </Box>
        {saveDisabled && checkIfEndInPast(state, timezone[0]) && !pastEvent && (
          <Typography text={addEventLabels.endInPastError.label} variant='labelS' color='error' />
        )}
        {saveDisabled &&
          isOverlappingStartTime(state, eventsForUi, timezone[0] || 'America/New_York', event) && (
            <Typography
              text={addEventLabels.cannotOverlapError.label}
              variant='labelS'
              color='error'
            />
          )}
        {saveDisabled && checkIfStartBeforeEnd(state) && (
          <Typography
            text={addEventLabels.startAfterEndError.label}
            variant='labelS'
            color='error'
          />
        )}
        {saveDisabled && isDurationOver24Hours(state) && (
          <Typography
            text={addEventLabels.durationOver24Error.label}
            variant='labelS'
            color='error'
          />
        )}
      </Box>
    </Box>
  );
};

export default SubmitDiscardDelete;
