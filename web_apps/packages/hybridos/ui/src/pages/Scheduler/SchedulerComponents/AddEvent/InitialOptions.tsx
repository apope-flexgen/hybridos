/* eslint-disable */
// TODO:: fix lint
import {
  Select,
  NumericInput,
  MinuteTimePicker,
  TextField,
  DatePicker,
  ThemeType,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import dayjs from 'dayjs';
import React from 'react';

import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { addEventLabels } from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import { EditEventState, EventVariables, VariableValues } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';

export interface InitialOptionsProps {
  disableDateField?: boolean;
  // An array of event variables to display as textfields in the modal
  // allow users to edit variables
  variables: EventVariables[];
  // which direction to display the variables/time in - column or row
  displayDirection?: 'column' | 'row';
  state: EditEventState;
  dispatch: React.Dispatch<any>;
  pastEvent?: boolean;
  activeEvent?: boolean;
}

const InitialOptions: React.FunctionComponent<InitialOptionsProps> = ({
  disableDateField,
  displayDirection = 'column',
  variables,
  state,
  dispatch,
  pastEvent,
  activeEvent,
}) => {
  const theme = useTheme() as ThemeType;
  const { disableAllFields } = useSchedulerContext();

  const mappedVariables = variables.map((variable) => {
    const variableValue = state.variableValues.find(
      (obj: VariableValues) => obj.name === variable.id,
    );
    const { unit, id, name, type } = variable;
    const handleVariableChange = (e: any) => {
      const newValue = [{ name: id, value: e.target.value }];
      const newVariableValues = state.variableValues.map(
        (obj: VariableValues) => newValue.find((o) => o.name === obj.name) || obj,
      );
      dispatch({ type: 'setVariableValues', payload: newVariableValues });
    };
    const { value } = variableValue || { value: '' };

    if (type === 'Bool') {
      return (
        <Box sx={{ width: '100%' }}>
          <Select
            fullWidth
            disabled={disableAllFields || pastEvent || activeEvent}
            label={name}
            key={name}
            menuItems={['true', 'false']}
            onChange={handleVariableChange}
            value={value.toString()}
          />
        </Box>
      );
    }
    if (type === 'Int' || type === 'Float') {
      return (
        <NumericInput
          disabled={disableAllFields || pastEvent || activeEvent}
          endTextAdornment={unit}
          fullWidth
          key={name}
          label={name}
          onChange={handleVariableChange}
          validationRegEx={type === 'Float' ? 'floats' : 'integers'}
          value={value.toString()}
        />
      );
    }
    return (
      <TextField
        TextAdornment={unit}
        adornment='end'
        disabled={disableAllFields || pastEvent || activeEvent}
        fullWidth
        key={name}
        label={name}
        onChange={handleVariableChange}
        value={value.toString()}
      />
    );
  });

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: { displayDirection },
        gap: theme.fgb.editModal.spacing.padding,
        flexWrap: 'wrap',
      }}
    >
      {mappedVariables}
      <Box sx={{ width: '100%', display: 'flex', gap: '8px' }}>
        <DatePicker
          disablePast
          disabled={disableDateField || disableAllFields || pastEvent || activeEvent}
          fullWidth
          label={addEventLabels.startDate.label}
          onChange={(newDate) => {
            dispatch({ type: 'setDate', payload: newDate });
          }}
          value={state.date}
        />
        <MinuteTimePicker
          disablePast={state.date && state.date.isSame(dayjs(), 'day')}
          disabled={disableAllFields || pastEvent || activeEvent}
          width='small'
          onHourChange={(e) => {
            dispatch({ type: 'setStartHours', payload: e.target.value });
            dispatch({ type: 'setStartTime', payload: `${e.target.value}:${state.startMinutes}` });
          }}
          onMinuteChange={(e) => {
            dispatch({ type: 'setStartMinutes', payload: e.target.value });
            dispatch({ type: 'setStartTime', payload: `${state.startHours}:${e.target.value}` });
          }}
          hourValue={state.startHours}
          minuteValue={state.startMinutes}
        />
      </Box>
      <Box sx={{ width: '100%', display: 'flex', gap: '8px' }}>
        <DatePicker
          disablePast
          disabled={disableDateField || disableAllFields || pastEvent}
          fullWidth
          label={addEventLabels.endDate.label}
          onChange={(newDate) => {
            dispatch({ type: 'setEndDate', payload: newDate });
          }}
          value={state.endDate}
        />
        <MinuteTimePicker
          disablePast={state.endDate && state.endDate.isSame(dayjs(), 'day')}
          disabled={disableAllFields || pastEvent}
          width='small'
          onHourChange={(e) => {
            dispatch({ type: 'setEndHours', payload: e.target.value });
            dispatch({ type: 'setEndTime', payload: `${e.target.value}:${state.endMinutes}` });
          }}
          onMinuteChange={(e) => {
            dispatch({ type: 'setEndMinutes', payload: e.target.value });
            dispatch({ type: 'setEndTime', payload: `${state.endHours}:${e.target.value}` });
          }}
          hourValue={state.endHours}
          minuteValue={state.endMinutes}
        />
      </Box>
    </Box>
  );
};

export default InitialOptions;
