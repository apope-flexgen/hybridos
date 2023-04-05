import ModeManager from 'src/pages/Scheduler/ModeManager/ModeManager';
import EventScheduler from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import UnconfiguredContainer from 'src/pages/Scheduler/SchedulerComponents/UnconfiguredContainer/UnconfiguredContainer';
import SiteFleetConfig from 'src/pages/Scheduler/SchedulerConfiguration/SchedulerConfig';
import { schedulerLabels } from 'src/pages/Scheduler/SchedulerHelpers';
import { SchedulerTabs } from 'src/pages/Scheduler/SchedulerTypes';

interface ITabContent {
  schedulerTab: SchedulerTabs;
  configured: boolean;
  schedulerType: 'SC' | 'FM' | null;
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>;
}

const TabContent = ({
  schedulerTab, configured, schedulerType, setIsLoading,
}: ITabContent) => (
  <>
    {schedulerTab === schedulerLabels.schedulerTab.value && configured && (
      <EventScheduler setIsLoading={setIsLoading} schedulerType={schedulerType} />
    )}
    {schedulerTab === schedulerLabels.schedulerTab.value && !configured && (
      <UnconfiguredContainer />
    )}
    {schedulerTab === schedulerLabels.modeManagerTab.value && configured && (
      <ModeManager schedulerType={schedulerType} />
    )}
    {schedulerTab === schedulerLabels.modeManagerTab.value && !configured && (
      <UnconfiguredContainer />
    )}
    {schedulerTab === schedulerLabels.schedulerConfigTab.value && (
      <SiteFleetConfig
        configured={configured}
        schedulerType={schedulerType}
      />
    )}
  </>
);

export default TabContent;
