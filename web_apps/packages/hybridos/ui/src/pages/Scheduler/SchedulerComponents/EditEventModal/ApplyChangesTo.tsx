import { Switch, RadioButton, Typography } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React from 'react';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { ApplyChangesTo } from 'src/pages/Scheduler/SchedulerTypes';
import { applyChangesRadios } from './EditEventModal-helpers';

interface ApplyChangesToProps {
  applyChangesTo: ApplyChangesTo;
  setApplyChangesTo: React.Dispatch<React.SetStateAction<ApplyChangesTo>>;
  editRecurring: boolean;
  setEditRecurring: React.Dispatch<React.SetStateAction<boolean>>;
  pastEvent: boolean;
  activeEvent: boolean;
}

const ApplyChanges: React.FunctionComponent<ApplyChangesToProps> = ({
  applyChangesTo,
  setApplyChangesTo,
  editRecurring,
  setEditRecurring,
  pastEvent,
  activeEvent,
}: ApplyChangesToProps) => {
  const { disableAllFields } = useSchedulerContext();

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
      }}
    >
      {applyChangesRadios.map((radio) => (
        <RadioButton
          disabled={disableAllFields || pastEvent || activeEvent}
          key={radio.label}
          label={radio.label}
          onChange={() => {
            setApplyChangesTo({
              thisEventOnly: !applyChangesTo.thisEventOnly,
              allInSeries: !applyChangesTo.allInSeries,
            });
            if (applyChangesTo.allInSeries) setEditRecurring(false);
          }}
          value={applyChangesTo[radio.value as keyof ApplyChangesTo]}
        />
      ))}
      <Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'center' }}>
        <Switch
          disabled={applyChangesTo.thisEventOnly || disableAllFields || pastEvent}
          onChange={() => {
            setEditRecurring(!editRecurring);
          }}
          value={editRecurring}
        />
        <Typography
          text="Edit Recurring Settings"
          color={disableAllFields ? 'disabled' : 'primary'}
          variant="bodyM"
        />
      </Box>
    </Box>
  );
};

export default ApplyChanges;
