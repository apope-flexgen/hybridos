import {
  RadioButton,
  NumericInput,
  NumericStepper,
  SelectorContainer,
  Circle,
  ThemeType,
} from '@flexgen/storybook';
import { Typography } from '@mui/material';
import { Box, ThemeProvider } from '@mui/system';
import dayjs from 'dayjs';
import React, { useEffect, useState } from 'react';
import { SchedulerEvent } from 'shared/types/dtos/scheduler.dto';
import { createMuiTheme, daysList } from 'src/pages/Scheduler/SchedulerHelpers';
import { EditEventState } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import { addEventLabels, dayNames, onArrow } from './AddEventHelpers';

export interface RepeatEveryOptionsProps {
  state: EditEventState
  dispatch: React.Dispatch<any>
  event?: SchedulerEvent
}

const RepeatEvery: React.FunctionComponent<RepeatEveryOptionsProps> = ({
  state, dispatch, event,
}) => {
  const theme = useTheme() as ThemeType;
  const muiOverrides = createMuiTheme(theme);
  const [dayOfWeekSelected, setDayOfWeekSelected] = useState<number>(0);

  useEffect(() => {
    if (event && event.repeat) setDayOfWeekSelected(dayjs(event.repeat.start).day());
    else if (state.date !== undefined && state.date?.day() !== undefined) {
      setDayOfWeekSelected(state.date?.day());
    }
  }, [state.date, event]);

  return (
    <ThemeProvider theme={muiOverrides}>
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          gap: theme.fgb.editModal.spacing.gap,
        }}
      >
        <Typography variant="body1">Repeat Every</Typography>
        <NumericInput
          endComponentAdorment={(
            <NumericStepper
              downArrowOnClick={() => onArrow('down', state.repeatEveryValue, dispatch)}
              upArrowOnClick={() => onArrow('up', state.repeatEveryValue, dispatch)}
            />
                      )}
          fullWidth
          label=""
          onChange={(e) => dispatch({ type: 'setRepeatEveryValue', payload: e.target.value })}
          placeholder={addEventLabels.repeatEvery.placeholder}
          validationRegEx={addEventLabels.repeatEvery.validationRegEx}
          value={state.repeatEveryValue}
        />
        <RadioButton
          label={addEventLabels.repeatEveryRadios.dayLabel}
          onChange={() => dispatch({
            type: 'setRepeatEveryIncrement',
            payload: {
              Days: !state.repeatEveryIncrement.Days,
              Weeks: !state.repeatEveryIncrement.Weeks,
            },
          })}
          value={state.repeatEveryIncrement.Days}
        />
        <RadioButton
          label={addEventLabels.repeatEveryRadios.weekLabel}
          onChange={() => dispatch({
            type: 'setRepeatEveryIncrement',
            payload: {
              Days: !state.repeatEveryIncrement.Days,
              Weeks: !state.repeatEveryIncrement.Weeks,
            },
          })}
          value={state.repeatEveryIncrement.Weeks}
        />
        <Typography hidden={!state.repeatEveryIncrement.Weeks} variant="body1">
          On
        </Typography>
        <SelectorContainer
          disabled={!state.repeatEveryIncrement.Weeks}
          key="week-day-container"
          value={state.daysSelected}
        >
          {daysList.map((day) => (
            <Circle
              key={day.name}
              label={day.label}
              onClick={() => dispatch({
                type: 'setDaysSelected',
                payload: {
                  ...state.daysSelected,
                  [dayNames[dayOfWeekSelected]]: true,
                  [day.name]:
                                            dayNames[dayOfWeekSelected] !== day.name
                                              ? !state.daysSelected[day.name]
                                              : true,
                },
              })}
              selected={state.daysSelected[day.name]}
            />
          ))}
        </SelectorContainer>
      </Box>
    </ThemeProvider>
  );
};

export default RepeatEvery;
