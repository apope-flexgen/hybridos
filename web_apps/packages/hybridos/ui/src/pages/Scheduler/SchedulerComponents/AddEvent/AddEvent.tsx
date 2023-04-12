// TODO: fix lint
/* eslint-disable max-lines */
import {
  Switch, Select, ThemeType, Typography,
} from '@flexgen/storybook';
import { Box, SelectChangeEvent } from '@mui/material';
import React, {
  useEffect, useReducer, useState, useMemo,
} from 'react';

import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import {
  addEventLabels,
  createNewEvent,
  mapModesToVariables,
  isOverlappingStartTime,
  isDurationOver24Hours,
} from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import { formatStartTime } from 'src/pages/Scheduler/SchedulerComponents/CalendarGroup/CalendarGroupHelpers';
import { reducer } from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/EditEventModal-helpers';
import { useEventSchedulerContext } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import { createRepeatObject, handleVariableValues } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import { checkIfStartBeforeEnd, defaultMode, initialEditState } from 'src/pages/Scheduler/SchedulerHelpers';
import { EventVariables } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import AddClearButtons from './AddClearEvent';
import EndsOptions from './EndsOptions';
import InitialOptions from './InitialOptions';
import RepeatEvery from './RepeatEveryOptions';

const AddEvent: React.FC = () => {
  const [eventTypeItems, setEventTypeItems] = useState<string[]>([]);
  const [modeIds, setModeIds] = useState<{ name: string | undefined; id: string | undefined }[]>(
    [],
  );
  const [recurring, setRecurring] = useState(false);
  const [variables, setVariables] = useState<EventVariables[]>([]);

  const [state, dispatch] = useReducer(reducer, initialEditState);

  const { modes, disableAllFields } = useSchedulerContext();
  const { addEvent, eventsForUi } = useEventSchedulerContext();

  const theme = useTheme() as ThemeType;

  useEffect(() => {
    const tempEventTypeItems: string[] = [];
    const tempModeIds: { name: string | undefined; id: string | undefined }[] = [];
    Object.keys(modes).map((modeId: string) => {
      if (modes[modeId].name?.toLowerCase() !== defaultMode) {
        tempEventTypeItems.push(modes[modeId].name);
        tempModeIds.push({ name: modes[modeId].name, id: modeId });
      }
      return null;
    });
    setModeIds(tempModeIds);
    setEventTypeItems(tempEventTypeItems);
  }, [modes]);

  const allDisabled = useMemo(() => {
    const {
      mode, date, startTime, endTime,
    } = state;
    return mode === '' || date === null || startTime === '' || endTime === '';
  }, [state]);

  const addDisabled = useMemo(() => {
    const {
      date, startTime, endTime, endDate,
    } = state;
    return isOverlappingStartTime(date, startTime, endTime, endDate, eventsForUi)
            || isDurationOver24Hours(state)
            || checkIfStartBeforeEnd(state);
  }, [state, eventsForUi]);

  const handleClear = () => {
    setVariables([]);
    setRecurring(false);
    dispatch({ type: 'reset', payload: initialEditState });
  };

  const handleAddEvent = () => {
    const { duration, startUTC } = formatStartTime(
      state.date,
      state.startTime,
      state.endTime,
      state.endDate,
    );
    const repeat = createRepeatObject(state, state.exceptions);
    const typedVariableValues = handleVariableValues(state.variableValues, state.modeId, modes);

    const newEvent = createNewEvent(
      duration,
      state.modeId,
      startUTC,
      typedVariableValues,
      recurring ? repeat : undefined,
    );

    addEvent(newEvent);
    handleClear();
  };

  const handleEventTypeChange = (e: SelectChangeEvent<string>) => {
    const mode = modeIds.find((m) => m.name === e.target.value);
    if (mode) dispatch({ type: 'setModeId', payload: mode.id });
    dispatch({ type: 'setMode', payload: e.target.value });
    mapModesToVariables(e, modes, setVariables, dispatch);
  };

  return (
    <Box
      sx={{
        /*          width: theme.fgb.scheduler.addEventWidth,
                 */ width: '320px',
        display: 'flex',
        flexDirection: 'column',
        gap: theme.fgb.editModal.spacing.padding,
        padding: theme.fgb.editModal.spacing.padding,
      }}
    >
      <Typography text={addEventLabels.title.label} variant="headingS" />
      <Select
        disabled={disableAllFields}
        label={addEventLabels.eventType.label}
        menuItems={eventTypeItems}
        onChange={handleEventTypeChange}
        value={state.mode}
      />
      <InitialOptions dispatch={dispatch} state={state} variables={variables} />
      <Box sx={{ display: 'flex', alignItems: 'center' }}>
        <Switch
          disabled={disableAllFields}
          onChange={() => {
            setRecurring(!recurring);
          }}
          value={recurring}
        />
        <Typography text="Make Recurring" variant="bodyL" color={disableAllFields ? 'disabled' : 'primary'} />
      </Box>
      {recurring && (
        <Box
          sx={{
            display: 'flex',
            flexDirection: 'column',
            gap: '12px',
            /*  gap: theme.fgb.scheduler.gap, */
          }}
        >
          <RepeatEvery dispatch={dispatch} state={state} />
          <EndsOptions dispatch={dispatch} state={state} />
        </Box>
      )}
      <Box sx={{
        display: 'flex', flexDirection: 'column', gap: '8px', alignItems: 'flex-end', width: '100%',
      }}
      >
        <AddClearButtons
          addDisabled={disableAllFields || allDisabled || addDisabled}
          handleAddEvent={handleAddEvent}
          handleClear={handleClear}
        />
        { addDisabled
        && isOverlappingStartTime(
          state.date,
          state.startTime,
          state.endTime,
          state.endDate,
          eventsForUi,
        )
        && <Typography text="New event cannot overlap with existing events" variant="labelS" />}
      </Box>
    </Box>
  );
};

export default AddEvent;
