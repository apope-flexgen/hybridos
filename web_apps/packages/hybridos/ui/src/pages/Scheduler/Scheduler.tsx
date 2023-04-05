/* eslint-disable max-lines */
import {
  CardContainer,
  PageLoadingIndicator,
  Tab,
  Tabs,
  TimeZones,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import {
  createContext, useContext,
} from 'react';
import {
  ApiMode, Configuration, Connected, EventsObject,
} from 'shared/types/dtos/scheduler.dto';
import { PageProps } from 'src/pages/PageTypes';
import { ModalStateType } from 'src/pages/Scheduler/SchedulerComponents/Modal/Helpers';
import SchedulerModal from 'src/pages/Scheduler/SchedulerComponents/Modal/Modal';
import TabContent from './SchedulerComponents/TabContent';
import { schedulerTabs } from './SchedulerHelpers';
import { SchedulerTabs, FleetManagerSite } from './SchedulerTypes';
import useScheduler from './hooks';

interface EventSchedulerContextValue {
  siteId: string
  fmSites: FleetManagerSite[]
  setSiteId: (newValue: string) => void
  timezone: TimeZones[]
  admin: boolean
  setSchedulerTab: (newTab: SchedulerTabs) => void
  siteName: string
  config: Configuration | null | undefined
  modes: ApiMode
  events: EventsObject | null
  setEvents: (events: EventsObject) => void
  connected: Connected | null
  navigationFlag: boolean
  setNavigationFlag: (newValue: boolean) => void
  modalOpen: boolean
  setModalOpen: (newValue: boolean) => void
  modalState: ModalStateType | null,
  setModalState: any
  unsavedChange: boolean
  setUnsavedChange: (newValue: boolean) => void
  disableAllFields: boolean
}

const EventSchedulerContext = createContext<EventSchedulerContextValue>({
  siteId: '',
  fmSites: [],
  setSiteId: () => {},
  timezone: [],
  admin: true,
  setSchedulerTab: () => {},
  siteName: '',
  config: null,
  modes: {},
  events: {},
  setEvents: () => {},
  connected: {},
  navigationFlag: false,
  setNavigationFlag: () => {},
  modalOpen: false,
  setModalOpen: () => {},
  modalState: null,
  setModalState: null,
  unsavedChange: false,
  setUnsavedChange: () => {},
  disableAllFields: false,
});

export function useSchedulerContext() {
  return useContext(EventSchedulerContext);
}

const Scheduler: React.FunctionComponent<PageProps> = ({ currentUser, product }: PageProps) => {
  console.log(product);
  const {
    displayTabs,
    contextValue,
    setSchedulerTab,
    configured,
    schedulerTab,
    schedulerType,
    isLoading,
    setIsLoading,
    setNavigationFlag,
    setModalOpen,
    modalOpen,
    modalState,
    setModalState,
    unsavedChange,
    setUnsavedChange,
    disableAllFields
  } = useScheduler(currentUser, product);

  const handleTabChange = (newValue: any) => {
    if (unsavedChange) {
      setModalOpen(true);
      setModalState((prevState: ModalStateType) => ({
        ...prevState,
        type: 'navigateWithoutSaving',
        secondaryActions: () => {
          setSchedulerTab(newValue as SchedulerTabs);
          setModalOpen(false);
          setUnsavedChange(false);
        },
        primaryActions: undefined,
      }));
    } else {
      setSchedulerTab(newValue as SchedulerTabs);
      setNavigationFlag(false);
    }
  };

  return (
    <EventSchedulerContext.Provider value={contextValue}>
      <Box
        sx={{
          display: 'flex',
          alignItems: 'flex-start',
          flexDirection: 'column',
          gap: '0.75rem',
          width: '100%',
        }}
      >
        <CardContainer flexDirection="column">
          <Box sx={{ width: '100%' }}>
            {displayTabs && (
            <Tabs
              onChange={(_, newValue) => {
                setNavigationFlag(true);
                handleTabChange(newValue);
              }}
              value={schedulerTab}
            >
              {schedulerTabs.map((tab) => (
                <Tab
                  icon={tab.icon ? tab.icon : undefined}
                  key={tab.value}
                  label={tab.label}
                  value={tab.value}
                />
              ))}
            </Tabs>
            )}
            <PageLoadingIndicator isLoading={isLoading} type="primary" />
            <SchedulerModal
              state={modalState}
              open={modalOpen}
            />
            <TabContent
              configured={configured}
              schedulerTab={schedulerTab}
              schedulerType={schedulerType}
              setIsLoading={setIsLoading}
            />
          </Box>
        </CardContainer>
      </Box>
    </EventSchedulerContext.Provider>
  );
};

export default Scheduler;
