import { Box, Checkbox, SeverityType } from '@flexgen/storybook';
import { FC, useState } from 'react';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';
import { initialSeverities, severitiesProperties } from './EventSeverities-helpers';
import { SeveritiesStateObject } from './EventSeveritiesTypes';

interface EventSeveritiesProps {
  filters: EventsRequestParams;
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void;
}

const EventSeverities: FC<EventSeveritiesProps> = ({
  filters,
  setFilters,
}: EventSeveritiesProps) => {
  const [severities, setSeverities] = useState<SeveritiesStateObject>(initialSeverities);

  const updateSeverityFilter = (
    selected: boolean,
    severity: SeverityType,
    filts: EventsRequestParams,
    setFilts: (value: React.SetStateAction<EventsRequestParams>) => void,
  ) => {
    if (selected) {
      filts.severity?.push(severity);
    } else {
      const indexOfSeverity = filts.severity?.indexOf(severity);
      if (indexOfSeverity !== undefined && indexOfSeverity !== -1) {
        filts.severity?.splice(indexOfSeverity, 1);
      }
    }
    setFilts({ ...filts, severity: filts.severity });
  };

  return (
    <Box>
      {Object.entries(severitiesProperties).map(([key, value]) => {
        const severity = key as keyof SeveritiesStateObject;
        return (
          <Checkbox
            color="primary"
            label={value.label}
            onChange={() => {
              setSeverities({
                ...severities,
                [severity]: !severities[severity],
              });
              updateSeverityFilter(!severities[severity], severity, filters, setFilters);
            }}
            value={severities[severity]}
          />
        );
      })}
    </Box>
  );
};

export default EventSeverities;
