// TODO: fix lint
/* eslint-disable max-statements */
import { SeverityType } from '@flexgen/storybook';
import dayjs from 'dayjs';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';

export const cardHeading = 'SORT ACTIVITY LOG';

export const buildURI = (filters: EventsRequestParams): string => {
  const filtersArray = [];
  if (filters.startTime != null) {
    filtersArray.push(['startTime', filters.startTime]);
  }
  if (filters.endTime != null) {
    filtersArray.push(['endTime', filters.endTime]);
  }
  if (filters.severity != null) {
    filters.severity.forEach((singleSeverity: string) => filtersArray.push(['severity', singleSeverity]));
  }
  if (filters.source != null) {
    filtersArray.push(['source', filters.source]);
  }
  if (filters.search?.length) {
    filtersArray.push(['search', filters.search]);
  }
  if (filters.limit != null) {
    filtersArray.push(['limit', filters.limit.toString()]);
  }
  if (filters.order != null) {
    filtersArray.push(['order', filters.order.toString()]);
  }
  if (filters.orderBy != null) {
    filtersArray.push(['orderBy', filters.orderBy]);
  }
  if (filters.page != null) {
    const page = filters.page === 0 ? 1 : filters.page + 1;
    filtersArray.push(['page', page.toString()]);
  }

  const filtersWithAmpersand = new URLSearchParams(filtersArray);
  const URI = `/events?${filtersWithAmpersand}`;

  return URI;
};

export const buildInitialFilters = (): EventsRequestParams => {
  const now = dayjs();
  const tenMinutesAgo = now.subtract(10, 'minute').format('YYYY/MM/DD HH:mm:ss');
  const initialFilters = {
    startTime: tenMinutesAgo,
    endTime: now.format('YYYY/MM/DD HH:mm:ss'),
    severity: ['Fault', 'Alarm'] as SeverityType[],
    source: undefined,
    search: '',
    limit: 10,
    page: 0,
    order: -1,
    orderBy: 'timestamp',
  };

  return initialFilters;
};
