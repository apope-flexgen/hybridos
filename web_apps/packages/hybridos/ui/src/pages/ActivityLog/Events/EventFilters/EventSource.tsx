import { Select } from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import { FC, useState } from 'react';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';
import { Sources } from 'src/pages/ActivityLog/Events/EventsHeader/EventsHeaderTypes';

interface EventsSourceProps {
  filters: EventsRequestParams;
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void;
}

const sourceOptions = ['Site', 'COPS', 'Assets', 'Modbus Client', 'Modbus Server', 'Any'];
const sourceLabel = 'Source';

const EventSource: FC<EventsSourceProps> = ({ filters, setFilters }: EventsSourceProps) => {
  const [source, setSource] = useState('');

  const handleSourceChange = (
    event: SelectChangeEvent,
    setEventsSource: React.Dispatch<React.SetStateAction<string>>,
    filts: EventsRequestParams,
    setFilts: (value: React.SetStateAction<EventsRequestParams>) => void,
  ) => {
    const newSource = event.target.value as string;
    setEventsSource(newSource);
    setFilts({
      ...filts,
      source: newSource === 'Any' ? undefined : (newSource as Sources),
    });
  };

  return (
    <Select
      label={sourceLabel}
      menuItems={sourceOptions}
      onChange={(event: SelectChangeEvent<string>) => {
        handleSourceChange(event, setSource, filters, setFilters);
      }}
      value={source}
    />
  );
};

export default EventSource;
