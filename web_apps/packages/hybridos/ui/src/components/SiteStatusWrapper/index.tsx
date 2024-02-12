import { Box, SiteStatusBar, ThemeType } from '@flexgen/storybook';
import { DataProps } from '@flexgen/storybook/dist/components/PlatformSpecific/HosControl/SiteStatusBar/Status';
import { useState, useEffect, useCallback } from 'react';
import { generateSiteStatusSx } from 'src/components/SiteStatusWrapper/siteStatus.styles';
import QueryService from 'src/services/QueryService';
import { useTheme } from 'styled-components';

// FIXME: where should this live?
export type SiteStateEnum = 'Init' | 'Ready' | 'Startup' | 'Running' | 'Shutdown' | 'Error';

const SiteStatusWrapper = () => {
  const theme = useTheme() as ThemeType;
  const siteStatusSx = generateSiteStatusSx(theme);

  const [activeFaults, setActiveFaults] = useState(0);
  const [activeAlarms, setActiveAlarms] = useState(0);
  const [siteState, setSiteState] = useState<SiteStateEnum | undefined>(undefined);
  const [data, setData] = useState<{ [uri: string]: DataProps }>({});
  const [siteStatusLabel, setSiteStatusLabel] = useState<string | undefined>(undefined);

  const handleNewMessage = useCallback((newInformationFromSocket: any) => {
    const parsedData = newInformationFromSocket.data;
    const { baseData, dataPoints } = parsedData;

    if (baseData?.siteStatusLabel !== undefined) {
      setSiteStatusLabel(baseData.siteStatusLabel);
    }

    if (baseData?.activeFaults !== undefined) {
      setActiveFaults(parseInt(baseData.activeFaults, 10));
    }

    if (baseData?.activeAlarms !== undefined) {
      setActiveAlarms(parseInt(baseData.activeAlarms, 10));
    }

    if (baseData?.siteState !== undefined) {
      setSiteState(baseData.siteState);
    }

    if (dataPoints !== undefined && Object.keys(dataPoints).length !== 0) {
      setData(
        (prevState) => ({
          ...prevState,
          ...dataPoints,
        } as { [uri: string]: DataProps }),
      );
    }
  }, []);

  useEffect(() => {
    QueryService.getSiteStatusBar(handleNewMessage);
  }, [handleNewMessage]);

  return (
    <Box sx={siteStatusSx}>
      <SiteStatusBar
        activeAlarms={activeAlarms}
        activeFaults={activeFaults}
        data={data}
        siteName={siteStatusLabel || ''}
        siteStatus={siteState || ''}
      />
    </Box>
  );
};

export default SiteStatusWrapper;
