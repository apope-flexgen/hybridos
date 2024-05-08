import { PureSearch } from '@flexgen/storybook';
import { ChangeEvent, FC, useState } from 'react';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';

interface EventsSearchProps {
  filters: EventsRequestParams;
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void;
}

const searchLabel = 'Search';

const EventSearch: FC<EventsSearchProps> = ({ filters, setFilters }: EventsSearchProps) => {
  const [searchTerm, setSearchTerm] = useState('');

  const handleSearchTermChange = (event: ChangeEvent<HTMLTextAreaElement | HTMLInputElement>) => {
    const newSearchTerm = event.target.value;
    setSearchTerm(newSearchTerm);
  };

  const handleSearch = (search: string) => {
    setFilters({
      ...filters,
      search,
    });
  };

  return (
    <PureSearch
      label={searchLabel}
      onChange={handleSearchTermChange}
      onSubmit={() => handleSearch(searchTerm)}
      value={searchTerm}
    />
  );
};

export default EventSearch;
