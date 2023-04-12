/* eslint-disable max-lines */
import {
  Select, NumericInput, TimePicker, TextField, DatePicker, ThemeType, Typography,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import dayjs from 'dayjs';
import React from 'react';

import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { addEventLabels, isDurationOver24Hours } from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEventHelpers';
import { checkIfStartBeforeEnd } from 'src/pages/Scheduler/SchedulerHelpers';
import { EditEventState, EventVariables, VariableValues } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';

export interface InitialOptionsProps {
  disableDateField?: boolean
  // An array of event variables to display as textfields in the modal
  // allow users to edit variables
  variables: EventVariables[]
  // which direction to display the variables/time in - column or row
  displayDirection?: 'column' | 'row'
  state: EditEventState
  dispatch: React.Dispatch<any>
}

const InitialOptions: React.FunctionComponent<InitialOptionsProps> = ({
  disableDateField,
  displayDirection = 'column',
  variables,
  state,
  dispatch,
}) => {
  const theme = useTheme() as ThemeType;
  const { disableAllFields } = useSchedulerContext();

  const mappedVariables = variables.map((variable) => {
    const variableValue = state.variableValues.find(
      (obj: VariableValues) => obj.name === variable.id,
    );
    const {
      unit, id, name, type,
    } = variable;
    const { value } = variableValue || { value: '' };
    if (type === 'Bool') {
      return (
        <Select
          fullWidth
          disabled={disableAllFields}
          label={name}
          key={name}
          menuItems={['true', 'false']}
          onChange={(e) => {
            const newValue = [{ name: id, value: e.target.value }];
            const newVariableValues = state.variableValues.map(
              (obj: VariableValues) => newValue.find((o) => o.name === obj.name) || obj,
            );
            dispatch({ type: 'setVariableValues', payload: newVariableValues });
          }}
          value={value.toString()}
        />
      );
    }
    if (type === 'Int' || type === 'Float') {
      return (
        <NumericInput
          endComponentAdornment={<></>}
          endTextAdornment={unit}
          disabled={disableAllFields}
          fullWidth
          key={name}
          label={name}
          onChange={(e) => {
            const newValue = [{ name: id, value: e.target.value }];
            const newVariableValues = state.variableValues.map(
              (obj: VariableValues) => newValue.find((o) => o.name === obj.name) || obj,
            );
            dispatch({ type: 'setVariableValues', payload: newVariableValues });
          }}
          validationRegEx={type === 'Float' ? 'floats' : 'integers'}
          value={value.toString()}
        />
      );
    }
    return (
      <TextField
        TextAdornment={unit}
        adornment="end"
        disabled={disableAllFields}
        fullWidth
        key={name}
        label={name}
        onChange={(e) => {
          const newValue = [{ name: id, value: e.target.value }];
          const newVariableValues = state.variableValues.map(
            (obj: VariableValues) => newValue.find((o) => o.name === obj.name) || obj,
          );
          dispatch({ type: 'setVariableValues', payload: newVariableValues });
        }}
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
          disabled={disableDateField || disableAllFields}
          fullWidth
          label={addEventLabels.startDate.label}
          onChange={(newDate) => {
            dispatch({ type: 'setDate', payload: newDate });
          }}
          value={state.date}
        />
        <TimePicker
          disablePast={state.date?.isSame(dayjs(), 'day')}
          disabled={disableAllFields}
          fullWidth
          label="Start Time"
          onChange={(e) => {
            dispatch({ type: 'setStartTime', payload: e.target.value });
          }}
          value={state.startTime}
        />
      </Box>
      <Box sx={{ width: '100%', display: 'flex', gap: '8px' }}>
        <DatePicker
          disablePast
          disabled={disableDateField || disableAllFields}
          fullWidth
          label={addEventLabels.endDate.label}
          onChange={(newDate) => {
            dispatch({ type: 'setEndDate', payload: newDate });
          }}
          value={state.endDate}
        />
        <TimePicker
          disablePast={state.endDate?.isSame(dayjs(), 'day')}
          disabled={disableAllFields}
          fullWidth
          label="End Time"
          onChange={(e) => {
            dispatch({ type: 'setEndTime', payload: e.target.value });
          }}
          value={state.endTime}
        />
      </Box>
      {
        checkIfStartBeforeEnd(state)
        && <Typography text={addEventLabels.startAfterEndError.label} variant="labelS" />
      }
      {
        isDurationOver24Hours(state)
        && <Typography text={addEventLabels.durationOver24Error.label} variant="labelS" />
      }
    </Box>
  );
};

export default InitialOptions;
