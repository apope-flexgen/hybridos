import { MuiButton, EmptyContainer } from '@flexgen/storybook';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { schedulerLabels } from 'src/pages/Scheduler/SchedulerHelpers';

const UnconfiguredContainer = () => {
  const { admin, setSchedulerTab } = useSchedulerContext();

  return (
    <div style={{ height: '500px' }}>
      <EmptyContainer
        text={
          admin
            ? schedulerLabels.unconfiguredContainer.adminLabel
            : schedulerLabels.unconfiguredContainer.userLabel
        }
      >
        {admin && (
          <MuiButton
            color={schedulerLabels.unconfiguredContainer.adminButton.color}
            label={schedulerLabels.unconfiguredContainer.adminButton.label}
            onClick={() => {
              setSchedulerTab(schedulerLabels.schedulerConfigTab.value);
            }}
            size={schedulerLabels.unconfiguredContainer.adminButton.size}
          />
        )}
      </EmptyContainer>
    </div>
  );
};

export default UnconfiguredContainer;
