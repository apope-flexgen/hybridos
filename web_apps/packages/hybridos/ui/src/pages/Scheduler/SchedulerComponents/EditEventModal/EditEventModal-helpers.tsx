/* eslint-disable max-lines */
import dayjs from 'dayjs';
import { SchedulerEvent } from 'shared/types/dtos/scheduler.dto';
import { VariableValues, DaysSelected } from 'src/pages/Scheduler/SchedulerTypes';

export const setUpVariablesValues = (event: SchedulerEvent, dispatch: React.Dispatch<any>) => {
  const tempVariableValues: VariableValues[] = [];
  if (!event.variables) return;
  Object.keys(event.variables).forEach((key) => {
    tempVariableValues.push({
      name: key,
      // TODO: fix lint
      // eslint-disable-next-line @typescript-eslint/ban-ts-comment
      // @ts-ignore
      value: (typeof event.variables[key] === 'string') ? event.variables[key] : event.variables[key].toString(),
    });
  });
  dispatch({ type: 'setVariableValues', payload: tempVariableValues });
};

export const getNextRepeat = (event: SchedulerEvent) => {
  let nextRepeat = '';
  if (event.repeat !== undefined) {
    if (event.repeat.cycle === 'week') {
      const binary = `0${Number(event.repeat.day_mask).toString(2)}`;
      const numArray = binary.split('');

      numArray.some((day, index) => {
        if (day === '1') {
          nextRepeat = dayjs(event.repeat?.start).day(index).format();
          if (dayjs(nextRepeat).isAfter(dayjs(event.repeat?.start))) {
            return nextRepeat;
          }
        }
        return null;
      });
      if (nextRepeat === event.repeat?.start && event.repeat.frequency) {
        const startOfNextWeek = dayjs(event.repeat.start)
          .add(event.repeat.frequency, 'weeks')
          .startOf('week');
        numArray.some((day, index) => {
          if (day === '1') {
            nextRepeat = startOfNextWeek.day(index).format();
            return nextRepeat;
          }
          return null;
        });
      }
    } else if (event.repeat.cycle === 'day' && event.repeat.frequency) {
      return dayjs(event.repeat.start).add(event.repeat.frequency, 'day').format();
    }
  }
  return nextRepeat;
};

export const createInitialDays = (numArray: string[]) => ({
  sun: numArray[0] === '1',
  mon: numArray[1] === '1',
  tues: numArray[2] === '1',
  wed: numArray[3] === '1',
  thurs: numArray[4] === '1',
  fri: numArray[5] === '1',
  sat: numArray[6] === '1',
});

export const applyChangesRadios = [
  { label: 'This Event Only', value: 'thisEventOnly' },
  { label: 'This Event and All Events in this Series', value: 'allInSeries' },
];

export function padStart(num: number, size: number) {
  const s = `000000${num}`;
  return s.substr(s.length - size);
}

export const getNewStartDate = (startTime: string, repeatStart: string): string => {
  const date = dayjs(repeatStart).format('MM-DD-YYYY');
  return dayjs(`${date} ${startTime}`).format();
};

export const initializeEventRepeat = (event: SchedulerEvent, dispatch: React.Dispatch<any>) => {
  // eslint-disable-next-line @typescript-eslint/no-unused-expressions
  if (event.repeat && event.repeat.frequency) {
    // eslint-disable-next-line @typescript-eslint/no-unused-expressions
    event.repeat.cycle === 'day'
      ? dispatch({
        type: 'setRepeatEveryIncrement',
        payload: { Days: true, Weeks: false },
      })
      : dispatch({
        type: 'setRepeatEveryIncrement',
        payload: { Days: false, Weeks: true },
      });

    dispatch({
      type: 'setRepeatEveryValue',
      payload: event.repeat.frequency.toString(),
    });

    if (event.repeat.end_time && event.repeat.end_time !== '2000-01-01T00:00:00Z') {
      dispatch({ type: 'setEndOnDate', payload: dayjs(event.repeat.end_time) });
      dispatch({
        type: 'setEndsOption',
        payload: { Never: false, On: true, After: false },
      });
    } else if (
      event.repeat.end_count
      && event.repeat.end_count !== 0
      && event.repeat.end_count !== -1
    ) {
      dispatch({
        type: 'setEndAfterOccurrences',
        payload: event.repeat.end_count.toString(),
      });
      dispatch({
        type: 'setEndsOption',
        payload: { Never: false, On: false, After: true },
      });
    } else {
      dispatch({
        type: 'setEndsOption',
        payload: { Never: true, On: false, After: false },
      });
    }
    if (event.repeat && event.repeat.day_mask) {
      const binary = Number(event.repeat.day_mask).toString(2);
      const paddedBinary = padStart(parseInt(binary, 10), 7);
      const numArray = paddedBinary.split('');
      const initialDays: DaysSelected = createInitialDays(numArray);
      dispatch({ type: 'setDaysSelected', payload: initialDays });
    }
  }
};

export const initializeEditModal = (
  event: SchedulerEvent,
  dispatch: React.Dispatch<any>,
  modeIds: { name: string | undefined; id: string | undefined }[],
) => {
  const mode = modeIds.find((m) => m.id === event.mode);
  const endTime = dayjs(event.start_time).add(event.duration, 'm');
  dispatch({
    type: 'setStartTime',
    payload: dayjs(event.start_time).format('HH:mm'),
  });
  dispatch({
    type: 'setEndTime',
    payload: dayjs(event.start_time).add(event.duration, 'minutes').format('HH:mm'),
  });
  dispatch({ type: 'setDate', payload: dayjs(event.start_time) });
  dispatch({ type: 'setEndDate', payload: dayjs(endTime) });
  dispatch({ type: 'setMode', payload: mode?.name });
  dispatch({ type: 'setModeId', payload: mode?.id });
  setUpVariablesValues(event, dispatch);
  initializeEventRepeat(event, dispatch);
};

export function reducer(state: any, action: any) {
  switch (action.type) {
    case 'setStartTime':
      return { ...state, startTime: action.payload };
    case 'setEndTime':
      return { ...state, endTime: action.payload };
    case 'setEndDate':
      return { ...state, endDate: action.payload };
    case 'setDate':
      return { ...state, date: action.payload };
    case 'setMode':
      return { ...state, mode: action.payload };
    case 'setModeId':
      return { ...state, modeId: action.payload };
    case 'setVariableValues':
      return { ...state, variableValues: action.payload };
    case 'setRepeatEveryValue':
      return { ...state, repeatEveryValue: action.payload };
    case 'setRepeatEveryIncrement':
      return { ...state, repeatEveryIncrement: action.payload };
    case 'setDaysSelected':
      return { ...state, daysSelected: action.payload };
    case 'setEndsOption':
      return { ...state, endsOption: action.payload };
    case 'setEndOnDate':
      return { ...state, endOnDate: action.payload };
    case 'setEndAfterOccurrences':
      return { ...state, endAfterOccurrences: action.payload };
    case 'setDiscardClicked':
      return { ...state, discardClicked: action.payload };
    case 'reset':
      return action.payload;
    default:
      throw new Error();
  }
}
