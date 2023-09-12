import { PageLoadingIndicator, ThemeType } from '@flexgen/storybook';
import { Order } from '@flexgen/storybook/dist/components/PlatformSpecific/HosControl/Table/Table-Sorting';
import { Box } from '@mui/material';
import { useState, ChangeEvent } from 'react';
import { Event, EventsRequestParams } from 'shared/types/dtos/events.dto';
import { headerBoxSx, mainContentBoxSx, tableBoxSx } from 'src/pages/Events/Styles';
import { useTheme } from 'styled-components';
import EventsHeader from './EventsHeader/EventsHeader';
import { buildInitialFilters } from './EventsHeader/EventsHeader-helpers';
import EventsTable from './EventsTable';

const Events = () => {
  const initialDisplayData: Event[] = [];
  const theme = useTheme() as ThemeType;
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
    const ord = isAsc ? 'desc' : 'asc';
    setOrder(ord);
    setOrderBy(property);
    setFilters({
      ...filters,
      order: ord === 'asc' ? 1 : -1,
      orderBy: property,
    });
  };

  const mainBoxSx = mainContentBoxSx(theme);
  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <EventsHeader
          filters={filters}
          setCurrentPage={setCurrentPage}
          setDisplayData={setDisplayData}
          setFilters={setFilters}
          setIsLoading={setIsLoading}
          setTotal={setTotal}
        />
      </Box>
      <Box sx={tableBoxSx}>
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
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};

export default Events;
