/* eslint-disable */
// TODO: fix lint
import { Select, IconButton, ThemeType } from '@flexgen/storybook';
import { Box, SelectChangeEvent, Typography } from '@mui/material';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import { ThemeProvider } from '@mui/system';
import React, { useState, useEffect, useReducer, useCallback } from 'react';
import {
  RepeatForAPI,
  SchedulerEvent,
  schedulerEventForAPI,
} from 'shared/types/dtos/scheduler.dto';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import {
  createNewEvent,
  mapModesToVariables,
} from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import EndsOptions from 'src/pages/Scheduler/SchedulerComponents/AddEvent/EndsOptions';
import InitialOptions from 'src/pages/Scheduler/SchedulerComponents/AddEvent/InitialOptions';
import RepeatEvery from 'src/pages/Scheduler/SchedulerComponents/AddEvent/RepeatEveryOptions';
import { formatStartTime } from 'src/pages/Scheduler/SchedulerComponents/CalendarGroup/CalendarGroupHelpers';
import ApplyChanges from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/ApplyChangesTo';
import {
  getNewStartDate,
  checkIfEventInPast,
  initializeEditModal,
  reducer,
  checkIfEventIsActive,
} from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/EditEventModal-helpers';
import createMuiTheme, {
  createDialogSx,
  createInitialOptionsBoxSx,
  createTitleBoxSx,
} from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/EditEventModal-styles';
import SubmitDiscardDelete from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/SubmitDiscardDelete';
import { useEventSchedulerContext } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import {
  createRepeatObject,
  handleVariableValues,
  schedulerURLS,
} from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import {
  defaultMode,
  initialApplyChangesTo,
  initialEditState,
} from 'src/pages/Scheduler/SchedulerHelpers';
import { editEventLabels } from 'src/pages/Scheduler/SchedulerLabels';
import { EventVariables } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';

export interface EditEventModalProps {
  // Whether or not the modal is open
  open: boolean;
  // whether this modal is for a single or recurring event
  recurring?: boolean;
  // Which event the user just clicked on, provides the necessary data to fill modal
  event: SchedulerEvent;
  setEditModalOpen: React.Dispatch<React.SetStateAction<boolean>>;
}

// Main component/functionality
const EditEventModal: React.FunctionComponent<EditEventModalProps> = ({
  open,
  recurring = false,
  event,
  setEditModalOpen,
}) => {
  const theme = useTheme() as ThemeType;
  const muiOverrides = createMuiTheme(theme);

  const [modeIds, setModeIds] = useState<{ name: string | undefined; id: string | undefined }[]>(
    [],
  );
  const [eventTypeItems, setEventTypeItems] = useState<string[]>([]);
  const [variables, setVariables] = useState<EventVariables[]>([]);
  const [applyChangesTo, setApplyChangesTo] = useState(initialApplyChangesTo);
  const [editRecurring, setEditRecurring] = useState(false);
  const [discardClicked, setDiscardClicked] = useState<boolean>(false);
  const [state, dispatch] = useReducer(reducer, initialEditState);

  const [pastEvent, setPastEvent] = useState<boolean>(false);
  const [activeEvent, setActiveEvent] = useState<boolean>(false);

  const { modes, events, disableAllFields, timezone } = useSchedulerContext();
  const { siteId, addException, updateEvent, addEvent } = useEventSchedulerContext();

  // eslint-disable-next-line @typescript-eslint/ban-ts-comment
  // @ts-ignore
  const axiosInstance = useAxiosWebUIInstance();

  const deleteEvent = useCallback(
    (eventId: string) => {
      axiosInstance.delete(`${schedulerURLS.deleteEvent}/${siteId}/${eventId}`);
    },
    [axiosInstance, siteId],
  );

  const handleSaveWithRepeat = (duration: number, startUTC: string, repeat: RepeatForAPI) => {
    const parentEvent =
      events !== null
        ? events[siteId].find((eventFromAPI) => eventFromAPI.repeat?.id === event.repeat?.id)
        : undefined;
    const recurringDateWithNewStart = parentEvent?.start_time
      ? getNewStartDate(state.startTime, parentEvent.start_time, timezone[0])
      : getNewStartDate(state.startTime, startUTC, timezone[0]);
    const typedVariableValues = handleVariableValues(state.variableValues, state.modeId, modes);

    if (applyChangesTo.allInSeries) {
      const newEvent: schedulerEventForAPI = createNewEvent(
        duration,
        state.modeId,
        recurringDateWithNewStart,
        typedVariableValues,
        repeat,
      );
      if (parentEvent?.id !== undefined) updateEvent(parentEvent.id, newEvent);
    } else if (parentEvent?.id !== undefined) {
      axiosInstance
        .post(`${schedulerURLS.addEvent}/${siteId}/${parentEvent.id}/exceptions`, {
          data: event.start_time,
        })
        .then(() => {
          const newEvent: schedulerEventForAPI = createNewEvent(
            duration,
            state.modeId,
            startUTC,
            typedVariableValues,
          );
          addEvent(newEvent);
        });
    }
    onClose();
  };

  const handleSave = () => {
    const { duration, startUTC } = formatStartTime(
      state.date,
      state.startTime,
      state.endTime,
      state.endDate,
      timezone[0] || 'America/New_York',
    );
    const typedVariableValues = handleVariableValues(state.variableValues, state.modeId, modes);

    if (!(event.repeat?.end_count === 1)) {
      const repeat = createRepeatObject(
        state,
        event.repeat?.exceptions ? event.repeat?.exceptions : undefined,
        event.repeat?.id,
      );
      handleSaveWithRepeat(duration, startUTC, repeat);
    } else {
      const newEvent: schedulerEventForAPI = createNewEvent(
        duration,
        state.modeId,
        startUTC,
        typedVariableValues,
      );
      if (event.id !== undefined) updateEvent(event.id, newEvent);
    }
    onClose();
  };

  const handleAddException = (eventWithoutException: SchedulerEvent, startTime: string) => {
    if (
      eventWithoutException.repeat !== undefined &&
      eventWithoutException.repeat.id !== undefined &&
      events !== null
    ) {
      const parentEvent = events[siteId].find(
        (eventFromAPI) => eventFromAPI.repeat?.id === eventWithoutException.repeat?.id,
      );
      if (parentEvent?.id !== undefined) {
        addException(parentEvent.id, startTime);
      }
    }
  };

  const handleDelete = (series: boolean) => {
    if (series && events !== null) {
      const parentEvent = events[siteId].find(
        (eventFromAPI) => eventFromAPI.repeat?.id === event.repeat?.id,
      );
      if (parentEvent?.id !== undefined) deleteEvent(parentEvent.id);
    } else if (!series && !(event.repeat?.end_count === 1)) {
      handleAddException(event, event.start_time);
    } else if (event.id !== undefined) deleteEvent(event.id);
    onClose();
  };

  const handleModeChange = (e: SelectChangeEvent<string>) => {
    const newMode = modeIds.find((mode) => mode.name === e.target.value);
    if (newMode) dispatch({ type: 'setModeId', payload: newMode.id });
    dispatch({ type: 'setMode', payload: e.target.value });
    mapModesToVariables(e, modes, setVariables, dispatch);
  };

  const dialogSx = createDialogSx(theme);
  const initialOptionsBoxSx = createInitialOptionsBoxSx(theme);
  const titleBoxSx = createTitleBoxSx(theme);

  // load edit modal with inital values from the event passed in
  useEffect(() => {
    setPastEvent(checkIfEventInPast(event, timezone[0] || 'America/New_York'));
    setActiveEvent(checkIfEventIsActive(event));

    const tempModeIds: { name: string | undefined; id: string | undefined }[] = [];
    Object.keys(modes).map((modeId: string) => {
      if (modes[modeId].name.toLowerCase() !== defaultMode) {
        tempModeIds.push({ name: modes[modeId].name, id: modeId });
      }
      return modeId;
    });
    setModeIds(tempModeIds);
    setEditRecurring(false);
    setApplyChangesTo(initialApplyChangesTo);

    initializeEditModal(event, dispatch, tempModeIds, timezone[0] || 'America/New_York');
  }, [event, discardClicked, modes, events]);

  // if the user changes the mode, update the varaibles
  useEffect(() => {
    const tempEventTypeItems: string[] = [];
    const tempModeIds: { name: string | undefined; id: string | undefined }[] = [];
    Object.keys(modes).map((modeId: string) => {
      if (modes[modeId].name.toLowerCase() !== defaultMode) {
        tempEventTypeItems.push(modes[modeId].name);
        tempModeIds.push({ name: modes[modeId].name, id: modeId });
      }
      if (modes[modeId].name === state.mode) setVariables(modes[modeId].variables);
      return null;
    });
    setModeIds(tempModeIds);
    setEventTypeItems(tempEventTypeItems);
  }, [state.mode, modes]);

  const onClose = () => {
    setDiscardClicked(true);
    setEditModalOpen(false);
    initializeEditModal(event, dispatch, modeIds, timezone[0]);
  };

  let modalTitle = editEventLabels.title.eventLabel;
  if (pastEvent) modalTitle = editEventLabels.title.pastEventLabel;
  else if (activeEvent) modalTitle = editEventLabels.title.activeEventLabel;

  return (
    <ThemeProvider theme={muiOverrides}>
      <Dialog PaperProps={{ sx: dialogSx }} onClose={onClose} open={open}>
        <DialogContent>
          <Box sx={titleBoxSx}>
            <Typography variant='h5'>{modalTitle}</Typography>
            <IconButton icon={editEventLabels.closeButton.icon} onClick={onClose} />
          </Box>
          <Box sx={initialOptionsBoxSx}>
            <Select
              fullWidth
              disabled={disableAllFields || pastEvent || activeEvent}
              placeholder=''
              label={editEventLabels.eventType.label}
              menuItems={eventTypeItems}
              onChange={handleModeChange}
              value={state.mode}
            />
            <InitialOptions
              disableDateField={applyChangesTo.allInSeries}
              dispatch={dispatch}
              displayDirection='row'
              state={state}
              variables={variables}
              pastEvent={pastEvent}
              activeEvent={activeEvent}
            />
          </Box>
          {recurring && (
            <Box sx={{ paddingTop: '24px' }}>
              <Typography variant='body1'>{editEventLabels.applyChangesTo.text}</Typography>
              <ApplyChanges
                applyChangesTo={applyChangesTo}
                editRecurring={editRecurring}
                setApplyChangesTo={setApplyChangesTo}
                setEditRecurring={setEditRecurring}
                pastEvent={pastEvent}
                activeEvent={activeEvent}
              />
              {editRecurring && (
                <Box sx={{ display: 'flex', gap: '24px' }}>
                  <RepeatEvery event={event} dispatch={dispatch} state={state} />
                  <EndsOptions dispatch={dispatch} state={state} />
                </Box>
              )}
            </Box>
          )}
          <SubmitDiscardDelete
            handleDelete={handleDelete}
            handleDiscard={() => setDiscardClicked(!discardClicked)}
            handleSave={handleSave}
            state={state}
            recurring={recurring}
            event={event}
            pastEvent={pastEvent}
          />
        </DialogContent>
      </Dialog>
    </ThemeProvider>
  );
};

export default EditEventModal;
