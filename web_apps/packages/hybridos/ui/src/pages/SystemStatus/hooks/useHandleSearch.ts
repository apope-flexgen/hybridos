import { formatServiceName } from 'src/pages/SystemStatus/SystemStatus.helpers';
import { SystemStatusObject } from 'src/pages/SystemStatus/SystemStatus.types';

const useHandleSearch = (systemStatusData: SystemStatusObject[]) => {
  const handleSearch = (searchTerms: string) => {
    const searchTermsLC = searchTerms.toLowerCase();
    const results: SystemStatusObject[] = systemStatusData.filter(
      (systemStatusObject) => formatServiceName(systemStatusObject.serviceName || '')
        .toLowerCase()
        .includes(searchTermsLC)
        || systemStatusObject.serviceStatus?.toLowerCase().includes(searchTermsLC)
        || systemStatusObject.connectionStatus?.toLowerCase().includes(searchTermsLC)
        || String(systemStatusObject.cpuUsage)?.toLowerCase().includes(searchTermsLC)
        || String(systemStatusObject.memoryUsage)?.toLowerCase().includes(searchTermsLC),
    );

    return results;
  };

  return { handleSearch };
};

export default useHandleSearch;
