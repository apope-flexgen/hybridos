import {
  Divider, Box, CardRow, Typography,
} from '@flexgen/storybook';
import React, {
  FC, useEffect, useRef, useContext,
} from 'react';
import { Event, EventsRequestParams } from 'shared/types/dtos/events.dto';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import EventDateTime from 'src/pages/ActivityLog/Events/EventFilters/EventDateTime/EventDateTime';
import EventSearch from 'src/pages/ActivityLog/Events/EventFilters/EventSearch';
import EventSeverities from 'src/pages/ActivityLog/Events/EventFilters/EventSeverities/EventSeverities';
import EventsDownload from 'src/pages/ActivityLog/Events/EventsDownload/EventsDownload';
import { buildURI } from './EventsHeader-helpers';

interface EventsProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>;
  setDisplayData: React.Dispatch<React.SetStateAction<Event[]>>;
  setTotal: React.Dispatch<React.SetStateAction<number>>;
  setCurrentPage: React.Dispatch<React.SetStateAction<number>>;
  setFilters: React.Dispatch<React.SetStateAction<any>>;
  filters: EventsRequestParams;
}

const EventsHeader: FC<EventsProps> = ({
  setIsLoading,
  setDisplayData,
  setCurrentPage,
  setFilters,
  setTotal,
  filters,
}: EventsProps) => {
  function usePrevious(value: EventsRequestParams) {
    const ref = useRef<EventsRequestParams>();
    useEffect(() => {
      ref.current = value;
    });
    return ref.current;
  }

  const axiosInstance = useAxiosWebUIInstance();
  const previousFilters = usePrevious(filters);
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  useEffect(() => {
    let pageToDisplay = filters.page ?? 0;
    const pageHasNotChanged = previousFilters?.page === filters.page;
    setIsLoading(true);
    if (pageHasNotChanged) {
      // Filters other than page have been changed, so reset to page 0.
      // TODO: fix lint
      // eslint-disable-next-line no-param-reassign
      filters.page = 0;
      pageToDisplay = filters.page;
    }

    const URI = buildURI(filters);

    axiosInstance
      .get(URI)
      .then((res) => {
        setCurrentPage(pageToDisplay);
        setTotal(res.data.count);
        setDisplayData(res.data.data);
      })
      .catch((err) => {
        if (err.response?.data?.statusCode === 408) {
          notifCtx?.notif(
            'error',
            'Events request timed out. Please narrow filter criteria and/or try again.',
          );
        }
      })
      .finally(() => {
        setIsLoading(false);
      });
    // TODO: fix lint
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [filters]);

  return (
    <Box
      sx={{
        marginTop: '16px',
        width: '100%',
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      <CardRow justifyContent="space-between">
        <Typography text="Events" variant="bodyXLBold" />
        <EventSearch filters={filters} setFilters={setFilters} />
      </CardRow>
      <Divider variant="fullWidth" orientation="horizontal" />
      <CardRow styleOverrides={{ paddingTop: '2rem' }}>
        <EventDateTime filters={filters} setFilters={setFilters} />
        <EventSeverities filters={filters} setFilters={setFilters} />
        <EventsDownload filters={filters} setIsLoading={setIsLoading} />
      </CardRow>
    </Box>
  );
};

export default EventsHeader;
