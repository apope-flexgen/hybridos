import dayjs from 'dayjs';
import { rest } from 'msw';
import eventsData from './eventsData';

const eventsPageMock = rest.get('/api/events', (req, res, ctx) => {
  const startTime = req.url.searchParams.get('startTime');
  const startTimeAsDayJs = dayjs(startTime, 'YYYY-MM-DD HH:mm:ss');
  const endTime = req.url.searchParams.get('endTime');
  const endTimeAsDayJs = dayjs(endTime, 'YYYY-MM-DD HH:mm:ss');
  const source = req.url.searchParams.get('source');
  const severities = req.url.searchParams.getAll('severity');
  const search = req.url.searchParams.get('search');

  const filtered = eventsData.filter((dataRow) => {
    const dayJsTimestamp = dayjs(dataRow.timestamp, 'YYYY-MM-DD HH:mm:ss');
    if (startTime != null && !dayJsTimestamp.isAfter(startTimeAsDayJs)) {
      return false;
    }
    if (endTime != null && !dayJsTimestamp.isBefore(endTimeAsDayJs)) {
      return false;
    }
    if (source != null && dataRow.source !== source) {
      return false;
    }
    if (severities.length !== 0 && !severities.includes(dataRow.severity)) {
      return false;
    }
    if (search && dataRow.message.indexOf(search) === -1) {
      return false;
    }
    return true;
  });

  return res(ctx.json(filtered));
});

export default eventsPageMock;
