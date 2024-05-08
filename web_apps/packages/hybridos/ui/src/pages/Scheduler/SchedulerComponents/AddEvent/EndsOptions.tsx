// TODO: fix lint
/* eslint-disable max-lines */
import {
  NumericInput,
  RadioButton,
  DatePicker,
  NumericStepper,
  ThemeType,
} from '@flexgen/storybook';
import { Typography } from '@mui/material';
import { Box, ThemeProvider } from '@mui/system';
import { createMuiTheme } from 'src/pages/Scheduler/SchedulerHelpers';
import { EditEventState } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import { addEventLabels, onAfterOcrruencesArrow } from './AddEventHelpers';

export interface EndsOptionsProps {
  state: EditEventState;
  dispatch: React.Dispatch<any>;
}

const EndsOptions: React.FunctionComponent<EndsOptionsProps> = ({ state, dispatch }) => {
  const theme = useTheme() as ThemeType;
  const muiOverrides = createMuiTheme(theme);

  return (
    <ThemeProvider theme={muiOverrides}>
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          gap: theme.fgb.editModal.spacing.gap,
        }}
      >
        <Typography variant="body1">Ends</Typography>
        <RadioButton
          label={addEventLabels.endOptionsRadios.neverLabel}
          value={state.endsOption.Never}
          onChange={() => dispatch({
            type: 'setEndsOption',
            payload: {
              Never: !state.endsOption.Never,
              On: state.endsOption.On ? !state.endsOption.On : state.endsOption.On,
              After: state.endsOption.After ? !state.endsOption.After : state.endsOption.After,
            },
          })}
        />
        <Box
          sx={{
            display: 'flex',
            gap: theme.fgb.editModal.spacing.gap,
          }}
        >
          <RadioButton
            label={addEventLabels.endOptionsRadios.onLabel}
            value={state.endsOption.On}
            onChange={() => dispatch({
              type: 'setEndsOption',
              payload: {
                Never: state.endsOption.Never ? !state.endsOption.Never : state.endsOption.Never,
                On: !state.endsOption.On,
                After: state.endsOption.After ? !state.endsOption.After : state.endsOption.After,
              },
            })}
          />
          <DatePicker
            disablePast
            value={state.endOnDate}
            onChange={(newValue) => {
              if (newValue !== null) dispatch({ type: 'setEndOnDate', payload: newValue });
            }}
            disabled={!state.endsOption.On}
          />
        </Box>
        <Box
          sx={{
            display: 'flex',
            gap: theme.fgb.editModal.spacing.gap,
          }}
        >
          <RadioButton
            label={addEventLabels.endOptionsRadios.afterLabel}
            value={state.endsOption.After}
            // eslint-disable-next-line  @typescript-eslint/no-unused-expressions
            onChange={() => dispatch({
              type: 'setEndsOption',
              payload: {
                Never: state.endsOption.Never ? !state.endsOption.Never : state.endsOption.Never,
                On: state.endsOption.On ? !state.endsOption.On : state.endsOption.On,
                After: !state.endsOption.After,
              },
            })}
          />
          <NumericInput
            fullWidth
            label={addEventLabels.occurencesNumericInput.label}
            disabled={!state.endsOption.After}
            validationRegEx={addEventLabels.occurencesNumericInput.validationRegEx}
            value={state.endAfterOccurrences}
            onChange={(e) => dispatch({
              type: 'setEndAfterOccurrences',
              payload: e.target.value,
            })}
            endComponentAdorment={(
              <NumericStepper
                upArrowOnClick={() => onAfterOcrruencesArrow('up', state.endAfterOccurrences, dispatch)}
                downArrowOnClick={() => onAfterOcrruencesArrow('down', state.endAfterOccurrences, dispatch)}
              />
            )}
          />
        </Box>
      </Box>
    </ThemeProvider>
  );
};

export default EndsOptions;
