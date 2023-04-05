import { SiteStatusBar } from '@flexgen/storybook';
import { DataProps } from '@flexgen/storybook/dist/components/Molecules/SiteStatusBar/Status';
import { useState, useEffect, useCallback } from 'react';
import QueryService from 'src/services/QueryService';

export type SiteStatusWrapperProps = {
  siteName: string
};

// FIXME: where should this live?
export type SiteStateEnum = 'Init' | 'Ready' | 'Startup' | 'Running' | 'Shutdown' | 'Error';

const SiteStatusWrapper = ({ siteName }: SiteStatusWrapperProps) => {
  const [activeFaults, setActiveFaults] = useState(0);
  const [activeAlarms, setActiveAlarms] = useState(0);
  const [siteState, setSiteState] = useState<SiteStateEnum>('Error');
  const [data, setData] = useState<{ [uri: string]: DataProps }>({});

  const handleNewMessage = useCallback((newInformationFromSocket: MessageEvent) => {
    const parsedData = JSON.parse(newInformationFromSocket.data);

    if (parsedData.activeFaults !== undefined) {
      setActiveFaults(parsedData.activeFaults);
    }

    if (parsedData.activeAlarms !== undefined) {
      setActiveAlarms(parsedData.activeAlarms);
    }

    if (parsedData.siteState !== undefined) {
      setSiteState(parsedData.siteState);
    }

    if (parsedData.dataPoints !== undefined) {
      setData((prevState) => ({
        ...prevState,
        ...parsedData.dataPoints,
      }));
    }
  }, []);

  useEffect(() => {
    QueryService.getSiteStatusBar(handleNewMessage);
  }, [handleNewMessage]);

  return (
    <SiteStatusBar
      activeAlarms={activeAlarms}
      activeFaults={activeFaults}
      data={data}
      siteName={siteName}
      siteStatus={siteState}
    />
  );
};

export default SiteStatusWrapper;
