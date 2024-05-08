// TODO: fix lint
/* eslint-disable max-lines, no-param-reassign */
import dayjs from 'dayjs';
import { rest } from 'msw';
import { SchedulerEvent, Configuration, ApiMode } from 'shared/types/dtos/scheduler.dto';
import schedulerConfigurationData from './schedulerConfigurationData';
import schedulerEventsData from './schedulerEventsData';
import modesData from './schedulerModesData';
import timezonesData from './schedulerTimezonesData';

const checkIfBeforeEndDate = (currentEndTime: dayjs.Dayjs, endTime: string) => {
  const endCheck = dayjs(endTime);
  return currentEndTime.isBefore(endCheck);
};

const checkIfBeforeEndCount = (numOccurences: number, endCount: number) => numOccurences < endCount;

function padStart(num: number, size: number) {
  const s = `000000${num}`;
  return s.substr(s.length - size);
}
const checkIfInExceptions = (exceptionsArray: string[], startTime: string) => {
  let returnVar = false;
  // eslint-disable-next-line array-callback-return
  exceptionsArray.map((exception) => {
    if (dayjs(startTime).isSame(dayjs(exception), 'day')) returnVar = true;
  });
  return returnVar;
};

const createRecurringEvents = (
  event: SchedulerEvent,
  recurringStartTime: dayjs.Dayjs,
  endTimeAsDayJs: dayjs.Dayjs,
  numOccurences: number,
  tempEventsData: SchedulerEvent[],
  startTimeAsDayJs: dayjs.Dayjs,
  startTimeFromEvent: dayjs.Dayjs,
) => {
  if (event.repeat?.frequency) {
    if (event.repeat.cycle === 'day') {
      numOccurences += 1;
      recurringStartTime = recurringStartTime.add(event.repeat.frequency, 'days');
      const recurringEndTime = recurringStartTime.add(event.duration, 'minutes');
      if (
        recurringStartTime.isAfter(startTimeAsDayJs)
        && recurringEndTime.isBefore(endTimeAsDayJs)
        && ((event.repeat.end_time && checkIfBeforeEndDate(recurringEndTime, event.repeat.end_time))
          || (event.repeat.end_count
            && checkIfBeforeEndCount(numOccurences, event.repeat.end_count)))
        && (event.repeat.exceptions === undefined
          || !checkIfInExceptions(event.repeat.exceptions, recurringStartTime.format()))
      ) {
        const recurringEventInframe: SchedulerEvent = {
          id: Math.floor(Math.random() * 1000000000).toString(),
          duration: event.duration,
          mode: event.mode,
          start_time: recurringStartTime.format(),
          variables: event.variables,
          repeat: event.repeat,
        };

        tempEventsData.push(recurringEventInframe);
      }
    } else if (event.repeat.cycle === 'week') {
      // TODO: PAD START WITH HOWEVER MANY ZEROS IT TAKES TO GET THIS DIGIT TO 7 IN LENGTH
      const binary = `${Number(event.repeat.day_mask).toString(2)}`;
      const paddedBinary = padStart(parseInt(binary, 10), 7);
      const numArray = paddedBinary.split('');
      // eslint-disable-next-line @typescript-eslint/no-loop-func
      numArray.map((day, index) => {
        if (day === '1') {
          recurringStartTime = recurringStartTime.day(index);
          const recurringEndTime = recurringStartTime.add(event.duration, 'minutes');

          if (
            recurringStartTime.isAfter(startTimeFromEvent.add(1, 'minute'))
            && recurringEndTime.isBefore(endTimeAsDayJs)
            && ((event.repeat?.end_time
              && checkIfBeforeEndDate(recurringEndTime, event.repeat.end_time))
              || (event.repeat?.end_count
                && checkIfBeforeEndCount(numOccurences, event.repeat.end_count)))
            && (event.repeat?.exceptions === undefined
              || !checkIfInExceptions(event.repeat.exceptions, recurringStartTime.format()))
          ) {
            const recurringEventInframe: SchedulerEvent = {
              id: Math.floor(Math.random() * 1000000000).toString(),
              duration: event.duration,
              mode: event.mode,
              start_time: recurringStartTime.format(),
              variables: event.variables,
              repeat: event.repeat,
            };
            tempEventsData.push(recurringEventInframe);
          }
          numOccurences += 1;
        }
        return recurringStartTime;
      });
      recurringStartTime = recurringStartTime.add(event.repeat.frequency, 'weeks');
    }

    if (recurringStartTime.isBefore(endTimeAsDayJs)) {
      createRecurringEvents(
        event,
        recurringStartTime,
        endTimeAsDayJs,
        numOccurences,
        tempEventsData,
        startTimeAsDayJs,
        startTimeFromEvent,
      );
    }
  }
};

export const schedulerEventsRoute = rest.get('/api/scheduler/events/:siteId', (req, res, ctx) => {
  const startTime = req.url.searchParams.get('startTime');
  const startTimeAsDayJs = dayjs(startTime, 'YYYY-MM-DD HH:mm:ss');
  const endTime = req.url.searchParams.get('endTime');
  const endTimeAsDayJs = dayjs(endTime, 'YYYY-MM-DD HH:mm:ss');
  const tempEventsData: SchedulerEvent[] = [];

  // eslint-disable-next-line array-callback-return
  schedulerEventsData.map((event) => {
    const startTimeFromEvent = dayjs(event.start_time);
    const endTimeFromEvent = startTimeFromEvent.add(event.duration, 'minutes');
    if (startTimeFromEvent.isAfter(startTimeAsDayJs) && endTimeFromEvent.isBefore(endTimeAsDayJs)) {
      tempEventsData.push(event);
    }
    if (event.repeat !== undefined) {
      const recurringStartTime = startTimeFromEvent;
      const numOccurences = 0;

      createRecurringEvents(
        event,
        recurringStartTime,
        endTimeAsDayJs,
        numOccurences,
        tempEventsData,
        startTimeAsDayJs,
        startTimeFromEvent,
      );
    }
  });

  return res(ctx.json(tempEventsData));
});

export const schedulerModesMock = rest.get('/api/scheduler/modes', (req, res, ctx) => res(ctx.json(modesData)));

export const schedulerTimezonesMock = rest.get('/api/scheduler/timezones', (req, res, ctx) => res(ctx.json(timezonesData)));

export const schedulerConfigurationMock = rest.get(
  '/api/scheduler/configuration',
  (req, res, ctx) => res(ctx.json(schedulerConfigurationData)),
);

export const putConfigurationRoute = rest.post<Configuration | null>(
  '/scheduler/configuration',
  async (req, res, ctx) => {
    const body = await req.json();
    const id = body.local_schedule.name.toLowerCase().replaceAll('/', '').replaceAll(' ', '');
    Object.assign(schedulerConfigurationData, body);
    if (schedulerConfigurationData.local_schedule) {
      schedulerConfigurationData.local_schedule.id = id;
    }
    return res(ctx.json(schedulerConfigurationData));
  },
);

export interface DeleteEventBody {
  startTime: string;
}

export const postModesRoute = rest.post<[string, ApiMode]>(
  '/scheduler/modes',
  async (req, res, ctx) => {
    const body = await req.json();

    const bodyID = body[0];
    const modeValues = body[1];
    const item = Object.keys(modesData).find((modeId: string) => modeId === bodyID);

    if (item) modesData[item] = modeValues;
    else modesData[bodyID] = modeValues;

    return res(ctx.json(modesData));
  },
);

export const deleteModeRoute = rest.delete<any>('scheduler/modes', async (req, res, ctx) => {
  const body = await req.json();
  const index = Object.keys(modesData).findIndex((modeId: string) => modeId === body);
  if (index !== -1) {
    delete modesData[body];
    schedulerEventsData.forEach((obj: SchedulerEvent, eventIndex: number) => {
      if (obj.mode === body) {
        schedulerEventsData.splice(eventIndex, 1);
      }
    });
  }

  return res(ctx.json(modesData));
});

export const postEventsRoute = rest.post<Event>(
  '/scheduler/events/:siteId',
  async (req, res, ctx) => {
    const body = await req.json();
    schedulerEventsData.push(body);
    return res(ctx.json(schedulerEventsData));
  },
);

export const deleteEventsRoute = rest.delete<DeleteEventBody>(
  '/scheduler/events/:siteId',
  async (req, res, ctx) => {
    const body = await req.json();
    const index = schedulerEventsData.findIndex((obj: SchedulerEvent) => obj.id === body.id);
    if (index === -1) {
      const groupIndex = schedulerEventsData.findIndex((obj) => obj.repeat?.id === body.id);
      schedulerEventsData.splice(groupIndex, 1);
    } else schedulerEventsData.splice(index, 1);

    return res(ctx.json(schedulerEventsData));
  },
);
