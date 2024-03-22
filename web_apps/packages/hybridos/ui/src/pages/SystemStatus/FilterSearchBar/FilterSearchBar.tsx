import {
  Box, CardRow, MuiButton, PureSearch, Select,
} from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { initialSystemStatusFilter } from 'src/pages/SystemStatus/SystemStatus.constants';
import { formatServiceName, toTitleCase } from 'src/pages/SystemStatus/SystemStatus.helpers';
import { filterBoxSx } from 'src/pages/SystemStatus/SystemStatus.styles';
import { FilterSearchBarProps, SystemStatusFilter, SystemStatusObject } from 'src/pages/SystemStatus/SystemStatus.types';
import useHandleSearch from 'src/pages/SystemStatus/hooks/useHandleSearch';

const FilterSearchBar = ({
  systemStatusData,
  setDisplayData,
}: FilterSearchBarProps) => {
  const [searchTerms, setSearchTerms] = useState<string>('');
  const [filters, setFilters] = useState<SystemStatusFilter>(initialSystemStatusFilter);
  const [filteredData, setFilteredData] = useState<SystemStatusObject[]>([]);

  const serviceNames = systemStatusData.map((systemStatusObject) => formatServiceName(systemStatusObject.serviceName || ''));
  const serviceStatuses = [...new Set(systemStatusData.map((systemStatusObject) => toTitleCase(systemStatusObject.serviceStatus || '')))];
  const connectionStatuses = [...new Set(systemStatusData.map((systemStatusObject) => toTitleCase(systemStatusObject.connectionStatus || '')))];

  const { handleSearch } = useHandleSearch(filteredData);

  const handleSearchOnChange = (
    e: React.ChangeEvent<HTMLTextAreaElement | HTMLInputElement>,
  ) => {
    setSearchTerms(e.target.value);
  };

  const handleSearchSubmit = useCallback(() => {
    const results = handleSearch(searchTerms);
    setDisplayData(results);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [searchTerms, handleSearch]);

  useEffect(() => {
    setSearchTerms('');
  }, [systemStatusData]);

  useEffect(() => {
    handleSearchSubmit();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [searchTerms, filteredData]);

  const handleFilter = (selectedValue: string | string[], field: 'serviceNames' | 'serviceStatus' | 'connectionStatus') => {
    setFilters({ ...filters, [field]: selectedValue });
  };

  const handleDeleteFilter = (element: string, field: 'serviceNames' | 'serviceStatus' | 'connectionStatus') => {
    const tempItems = filters[field];
    const indexOfItem = filters[field].indexOf(element);
    tempItems.splice(indexOfItem, 1);
    setFilters({ ...filters, [field]: tempItems });
  };

  useEffect(() => {
    const updatedFilteredData = systemStatusData.filter((serviceStatusObject) => {
      const compareName = formatServiceName(serviceStatusObject.serviceName || '');
      return (
        (filters.serviceStatus.length === 0
          || filters.serviceStatus.includes(toTitleCase(serviceStatusObject.serviceStatus || ''))
        )
        && (filters.connectionStatus.length === 0
            || filters.connectionStatus.includes(toTitleCase(serviceStatusObject.connectionStatus || '')))
        && (filters.serviceNames.length === 0 || filters.serviceNames.includes(compareName))
      );
    });
    setFilteredData(updatedFilteredData);
    setDisplayData(updatedFilteredData);
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [filters, systemStatusData]);

  const clearFilters = () => {
    setDisplayData(systemStatusData);
    setFilteredData(systemStatusData);
    setFilters(initialSystemStatusFilter);
  };

  return (
    <CardRow justifyContent="space-between" width="80%">
      <Box sx={filterBoxSx}>
        <Select
          label="Service Name"
          minWidth={220}
          value={filters.serviceNames}
          multiSelect
          menuItems={serviceNames}
          onChange={(e) => handleFilter(e.target.value, 'serviceNames')}
          onDelete={() => handleDeleteFilter('element', 'serviceNames')}
        />
        <Select
          label="Service Status"
          minWidth={220}
          value={filters.serviceStatus}
          multiSelect
          menuItems={serviceStatuses}
          onDelete={() => handleDeleteFilter('element', 'serviceStatus')}
          onChange={(e) => handleFilter(e.target.value, 'serviceStatus')}
        />
        <Select
          label="Connection Status"
          minWidth={220}
          value={filters.connectionStatus}
          multiSelect
          menuItems={connectionStatuses}
          onDelete={() => handleDeleteFilter('element', 'connectionStatus')}
          onChange={(e) => handleFilter(e.target.value, 'connectionStatus')}
        />
        <MuiButton variant="text" label="Clear Filters" onClick={clearFilters} />
      </Box>
      <PureSearch onChange={handleSearchOnChange} onSubmit={handleSearchSubmit} />
    </CardRow>
  );
};

export default FilterSearchBar;
