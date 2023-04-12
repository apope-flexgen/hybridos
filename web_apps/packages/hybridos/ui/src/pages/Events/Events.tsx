import { PageLoadingIndicator } from '@flexgen/storybook';
import { Order } from '@flexgen/storybook/dist/components/DataDisplay/Table/Table-Sorting';
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
  const [order, setOrder] = useState<Order>('desc');
  const [orderBy, setOrderBy] = useState<keyof Event>('timestamp');
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

  const handleRequestSort = (event: unknown, property: keyof Event) => {
    const isAsc = orderBy === property && order === 'asc';
    const ord = isAsc ? 'desc' : 'asc'
    setOrder(ord);
    setOrderBy(property);
    setFilters({
      ...filters,
      order: ord === 'asc' ? 1 : -1,
      orderBy: property,
    })
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
        handleRequestSort={handleRequestSort}
        order={order}
        orderBy={orderBy}
        rowsPerPage={rowsPerPage}
        serverSide
        total={total}
      />
    </Box>
  );
};

export default Events;
