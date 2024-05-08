/* eslint-disable */
//TODO: fix lint
import { TimeZones } from '@flexgen/storybook';
import { useState, useEffect, useCallback } from 'react';
import { Roles } from 'shared/types/api/Users/Users.types';
import { ApiMode, Configuration, EventsObject, Connected } from 'shared/types/dtos/scheduler.dto';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { schedulerURLS } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import {
  ModalStateType,
  SchedulerModalTypes,
} from 'src/pages/Scheduler/SchedulerComponents/Modal/Helpers';
import {
  initialSCConfig,
  initialFMConfig,
  SchedulerTypes,
} from 'src/pages/Scheduler/SchedulerConfiguration/Helpers';
import { schedulerLabels } from 'src/pages/Scheduler/SchedulerHelpers';
import { SchedulerTabs, FleetManagerSite } from 'src/pages/Scheduler/SchedulerTypes';
import QueryService from 'src/services/QueryService';

export default function useScheduler(currentUser: { role: string }, product: string | undefined) {
  const { role } = currentUser;
  const [siteId, setSiteId] = useState<string>();
  const [fmSites, setFMSites] = useState<FleetManagerSite[]>([]);
  const [siteName, setSiteName] = useState<string>();
  const [isLoading, setIsLoading] = useState<boolean>(true);
  const [schedulerTab, setSchedulerTab] = useState<SchedulerTabs>(
    schedulerLabels.schedulerTab.value,
  );

  // NEW: all data will be handled in this hook, passed down through the context into necessary components
  const [timezones, setTimezones] = useState<TimeZones[]>([]);
  const [schedulerConfiguration, setSchedulerConfiguration] = useState<Configuration | null>();
  const [modes, setModes] = useState<ApiMode>({});
  const [events, setEvents] = useState<EventsObject>({});
  const [connected, setConnected] = useState<Connected>({});
  const [schedulerType, setSchedulerType] = useState<SchedulerTypes>();

  const [unsavedChange, setUnsavedChange] = useState<boolean>(false);
  const [navigationFlag, setNavigationFlag] = useState<boolean>(false);

  const [modalOpen, setModalOpen] = useState<boolean>(false);

  const [modalState, setModalState] = useState({
    type: 'unsavedChanges' as SchedulerModalTypes,
    onClose: () => setModalOpen(false),
    primaryActions: undefined,
    secondaryActions: undefined,
  } as ModalStateType);

  const displayTabs = role === Roles.Admin || role === Roles.Developer;
  const disableAllFields = role === Roles.Observer;

  const [configured, setConfigured] = useState(false);
  const [disableModeManager, setDisableModeManager] = useState<boolean>(false);

  useEffect(() => {
    const disabled =
      schedulerConfiguration?.scheduler_type === 'SC' &&
      schedulerConfiguration?.web_sockets?.server?.enabled;
    setDisableModeManager(disabled || false);
  }, [schedulerConfiguration]);

  const axiosInstance = useAxiosWebUIInstance();

  const defaultFleetName = 'Fleet Manager';

  const updateConfigurationData = useCallback(
    (data: Configuration) => {
      const configData = data as Configuration;
      setSchedulerConfiguration(configData);
      if (configData.scheduler_type === 'SC') {
        setSiteId(configData.local_schedule?.id);
        setSiteName(configData.local_schedule?.name);
      } else if (configData.scheduler_type === 'FM') {
        if (configData?.web_sockets?.clients) {
          const siteExists =
            configData.web_sockets.clients.find((obj: any) => obj.id === siteId) ||
            configData.local_schedule?.id === siteId;
          if (!siteExists) setSiteId(undefined);
          const siteArray: FleetManagerSite[] = [];
          configData.web_sockets.clients.map((site: FleetManagerSite) =>
            siteArray.push({ id: site.id, name: site.name }),
          );
          if (configData?.local_schedule?.name && configData?.local_schedule?.id) {
            siteArray.push({
              id: configData?.local_schedule?.id,
              name: configData?.local_schedule?.name,
              fleetSchedule: true,
            });
          }
          setFMSites(siteArray);
        }

        if (!configData?.local_schedule?.name || configData?.local_schedule?.name.length === 0) {
          setSiteName(defaultFleetName);
        } else setSiteName(configData.local_schedule?.name);
      }
    },
    [siteId],
  );

  const updateTimeZoneData = useCallback(
    (data: any) => {
      const siteTimeZoneArr = siteId ? [data[siteId] as TimeZones] : [];
      setTimezones(siteTimeZoneArr);
    },
    [siteId],
  );

  const contextValue = {
    siteId: siteId || '',
    setSiteId,
    fmSites,
    timezone: timezones,
    admin: displayTabs,
    setSchedulerTab,
    siteName: siteName || '',
    config: schedulerConfiguration || null,
    modes: modes || {},
    events: events || {},
    setEvents,
    connected: connected || {},
    unsavedChange,
    setUnsavedChange,
    navigationFlag,
    setNavigationFlag,
    modalOpen,
    setModalOpen,
    modalState,
    setModalState,
    disableAllFields,
    disableModeManager,
  };

  const validateConfig = async (res: any) => {
    if (!res.data || Object.keys(res.data).length === 0 || !res.data.scheduler_type) {
      try {
        const uiSchedulerType = product as SchedulerTypes;
        if (uiSchedulerType === 'FM') {
          const fmConfig = await axiosInstance.post(
            schedulerURLS.postConfiguration,
            initialFMConfig,
          );
          updateConfigurationData(initialFMConfig);
          setSchedulerType('FM');
          setSiteName(defaultFleetName);
          setConfigured(true);
        } else if (uiSchedulerType === 'SC') {
          setConfigured(false);
          setSchedulerType('SC');
          updateConfigurationData(initialSCConfig);
        }
      } finally {
      }
    } else if (res.data.scheduler_type) {
      // Valid FM. (FM only needs a schedulerType field)
      if (res.data.scheduler_type === 'FM') {
        updateConfigurationData(res.data);
        setConfigured(true);
        setSchedulerType('FM');
      } else if (res.data.scheduler_type === 'SC') {
        // Config is a valid SC. (localSchedule.id & localSchedule.name)
        const ls = res.data.local_schedule;
        if (ls && ls.id && ls.name) {
          updateConfigurationData(res.data);
          setConfigured(true);
          setSchedulerType('SC');
        } else {
          updateConfigurationData(initialSCConfig);
          setConfigured(false);
          setSchedulerType('SC');
        }
      }
    }
  };

  // on initial page load, get site configuration, timezones
  // get initial site configuration
  useEffect(() => {
    setIsLoading(true);
    const getConfig = async () => {
      try {
        const data = await axiosInstance.get(schedulerURLS.getConfiguration);
        validateConfig(data);
        setIsLoading(false);
      } finally {
      }
    };
    getConfig();
  }, [axiosInstance, updateConfigurationData]);

  // get initial timezoneconnected, modes, events
  useEffect(() => {
    if (configured) {
      axiosInstance.get(schedulerURLS.getTimezones).then((timezonesResponse) => {
        updateTimeZoneData(timezonesResponse.data);
        axiosInstance.get(schedulerURLS.getConnected).then((connectedResponse) => {
          setConnected(connectedResponse.data);
          axiosInstance.get(schedulerURLS.getModes).then((modesResponse) => {
            setModes(modesResponse.data);
            axiosInstance.get(schedulerURLS.getEvents).then((eventsResponse) => {
              setEvents(eventsResponse.data);
              setIsLoading(false);
            });
          });
        });
      });
    }
    // TODO: fix eslint ignore
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [axiosInstance, configured, siteId]);

  // handle data coming in on socket
  const handleDataOnSocket = useCallback(
    (data: any) => {
      if (data[schedulerURLS.getConfiguration]) {
        updateConfigurationData(data[schedulerURLS.getConfiguration]);
      }
      if (data[schedulerURLS.getTimezones]) updateTimeZoneData(data[schedulerURLS.getTimezones]);
      if (data[schedulerURLS.getConnected]) setConnected(data[schedulerURLS.getConnected]);
      if (data[schedulerURLS.getEvents]) setEvents(data[schedulerURLS.getEvents]);
      if (data[schedulerURLS.getModes]) setModes(data[schedulerURLS.getModes]);
    },
    [updateConfigurationData, updateTimeZoneData],
  );

  // start listening to web sockets
  useEffect(() => {
    QueryService.getSchedulerPage(
      [
        schedulerURLS.getModes,
        schedulerURLS.getEvents,
        schedulerURLS.getConfiguration,
        schedulerURLS.getConnected,
        schedulerURLS.getTimezones,
      ],
      handleDataOnSocket,
    );
  }, [axiosInstance, updateConfigurationData, handleDataOnSocket]);

  useEffect(() => {
    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  return {
    contextValue,
    displayTabs,
    isLoading,
    setIsLoading,
    setSchedulerTab,
    configured,
    setConfigured,
    schedulerTab,
    schedulerType,
    navigationFlag,
    setNavigationFlag,
    modalOpen,
    setModalOpen,
    modalState,
    setModalState,
    unsavedChange,
    setUnsavedChange,
  };
}
