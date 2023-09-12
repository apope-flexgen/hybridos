import { Table } from '@flexgen/storybook';
import { Order } from '@flexgen/storybook/dist/components/PlatformSpecific/HosControl/Table/Table-Sorting';
import { ChangeEvent, FC } from 'react';
import { Event } from 'shared/types/dtos/events.dto';

declare type RowsPerPageOptions = 5 | 10 | 25 | 50 | 100 | 200;

const tableLayout = 'event';

interface EventsTableProps {
  displayData: Event[];
  currentPage: number;
  rowsPerPage: number;
  total: number;
  handleChangeRowsPerPage: (event: ChangeEvent<HTMLInputElement>) => void; // eslint-disable-line no-unused-vars
  handleChangePage: (event: unknown, newPage: number) => void; // eslint-disable-line no-unused-vars
  handleRequestSort: (event: unknown, property: keyof Event) => void; // eslint-disable-line no-unused-vars
  orderBy: keyof Event;
  order: Order;
  serverSide: boolean;
}

const EventsTable: FC<EventsTableProps> = ({
  displayData,
  currentPage,
  rowsPerPage,
  total,
  handleChangeRowsPerPage,
  handleChangePage,
  handleRequestSort,
  order,
  orderBy,
  serverSide,
}: EventsTableProps) => (
  <Table
    data={displayData}
    handleChangePage={handleChangePage}
    handleChangeRowsPerPage={handleChangeRowsPerPage}
    page={currentPage}
    rowsPerPage={rowsPerPage as RowsPerPageOptions}
    // TODO: fix lint
    // eslint-disable-next-line @typescript-eslint/ban-ts-comment
    // @ts-ignore
    serverSide={serverSide}
    tableLayout={tableLayout}
    total={total}
    handleRequestSort={handleRequestSort}
    order={order}
    orderBy={orderBy}
  />
);

export default EventsTable;
