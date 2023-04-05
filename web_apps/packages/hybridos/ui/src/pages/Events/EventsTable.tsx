import { Table } from '@flexgen/storybook';
import { ChangeEvent, FC } from 'react';
import { Event } from 'shared/types/dtos/events.dto';

declare type RowsPerPageOptions = 5 | 10 | 25 | 50 | 100 | 200;

const tableHeading = 'EVENTS';
const tableHeadingIcon = 'Events';
const tableLayout = 'event';

interface EventsTableProps {
  displayData: Event[],
  currentPage: number,
  rowsPerPage: number,
  total: number,
  handleChangeRowsPerPage: (event: ChangeEvent<HTMLInputElement>) => void, // eslint-disable-line no-unused-vars
  handleChangePage: (event: unknown, newPage: number) => void, // eslint-disable-line no-unused-vars
  serverSide: boolean,
}

const EventsTable: FC<EventsTableProps> = ({
  displayData,
  currentPage,
  rowsPerPage,
  total,
  handleChangeRowsPerPage,
  handleChangePage,
  serverSide,
}: EventsTableProps) => (
  <Table
    data={displayData}
    handleChangePage={handleChangePage}
    handleChangeRowsPerPage={handleChangeRowsPerPage}
    headingIcon={tableHeadingIcon}
    page={currentPage}
    rowsPerPage={rowsPerPage as RowsPerPageOptions}
    // TODO: fix lint
    // eslint-disable-next-line @typescript-eslint/ban-ts-comment
    // @ts-ignore
    serverSide={serverSide}
    tableHeading={tableHeading}
    tableLayout={tableLayout}
    total={total}
  />
);

export default EventsTable;
