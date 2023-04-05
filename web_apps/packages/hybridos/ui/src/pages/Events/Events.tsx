import { PageLoadingIndicator } from '@flexgen/storybook';
import { Box } from '@mui/material';
import { useState, ChangeEvent } from 'react';
import { Event, EventsRequestParams } from 'shared/types/dtos/events.dto';
import EventsHeader from './EventsHeader/EventsHeader';
import { buildInitialFilters } from './EventsHeader/EventsHeader-helpers';
import EventsTable from './EventsTable';

const Events = () => {
  const initialDisplayData: Event[] = [];

  const [isLoading, setIsLoading] = useState(false);
  const [displayData, setDisplayData] = useState(initialDisplayData);
  const [currentPage, setCurrentPage] = useState(0);
  const [rowsPerPage, setRowsPerPage] = useState(10);
  const [total, setTotal] = useState(0);
  const initialFilters: EventsRequestParams = buildInitialFilters();
  const [filters, setFilters] = useState(initialFilters);

  const handleChangeRowsPerPage = (event: ChangeEvent<HTMLInputElement>) => {
    setRowsPerPage(+event.target.value);
    setCurrentPage(0);
    setFilters({
      ...filters,
      page: 0,
      limit: +event.target.value,
    });
  };

  const handleChangePage = (event: unknown, newPage: number) => {
    setCurrentPage(newPage);
    setFilters({
      ...filters,
      page: newPage,
    });
  };

  return (
    <Box
      sx={{
        display: 'flex',
        alignItems: 'flex-start',
        flexDirection: 'column',
        gap: '0.75rem',
        width: '100%',
      }}
    >
      <PageLoadingIndicator isLoading={isLoading} type="tertiary" />
      <EventsHeader
        filters={filters}
        setCurrentPage={setCurrentPage}
        setDisplayData={setDisplayData}
        setFilters={setFilters}
        setIsLoading={setIsLoading}
        setTotal={setTotal}
      />
      <EventsTable
        currentPage={currentPage}
        displayData={displayData}
        handleChangePage={handleChangePage}
        handleChangeRowsPerPage={handleChangeRowsPerPage}
        rowsPerPage={rowsPerPage}
        serverSide
        total={total}
      />
    </Box>
  );
};

export default Events;
