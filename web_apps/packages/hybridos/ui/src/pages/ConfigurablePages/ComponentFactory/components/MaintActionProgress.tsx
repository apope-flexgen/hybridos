import {
  MuiButton, CardContainer, Typography, Stepper, Box, Icon, IconList, ColorType, IconButton,
} from '@flexgen/storybook';
import { useState } from 'react';
import { MaintenanceActionPath, MaintenanceActionStep } from 'shared/types/dtos/configurablePages.dto';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import MaintActionsDetailsModal from './MaintActionsDetailsModal';
import {
  MaintenanceActionStatuses,
  cardContainerSx,
  contentBoxSx,
  createStepComponents,
  decideStatusIcon,
  decideStatusIconColor,
  decideStepCountText,
  stepperBoxSx,
  textBoxSx,
} from './maintActions.helpers';

export interface MaintActionProgressProps {
  label: string;
  paths: MaintenanceActionPath[];
  status: MaintenanceActionStatuses,
  steps: MaintenanceActionStep[];
  stepIndex: number;
  pathIndex: number;
  stopActionURI: string;
  clearActionURI: string;
}

const MaintActionProgress: React.FC<MaintActionProgressProps> = ({
  label,
  paths,
  status,
  steps,
  stepIndex,
  pathIndex,

  stopActionURI,
  clearActionURI,
}: MaintActionProgressProps) => {
  const [modalOpen, setModalOpen] = useState<boolean>(false);

  let subheader: string = paths[pathIndex].steps[stepIndex]?.step_name || 'Complete';
  if (status === MaintenanceActionStatuses.Aborted) subheader = 'Action Aborted';
  else if (status === MaintenanceActionStatuses.Exiting) subheader = 'Exiting';

  const statusIcon: IconList = decideStatusIcon(status);
  const statusIconColor: ColorType = decideStatusIconColor(status);
  const stepComponents = createStepComponents(steps, stepIndex, status, 'horizontal');

  const stopAction = () => {
    const realTimeService = RealTimeService.Instance;
    realTimeService.send('fimsNoReply', {
      method: 'set',
      uri: stopActionURI,
      replyto: 'web_ui',
      body: true,
      username: 'web_ui',
    });
  };

  const clearAction = () => {
    const realTimeService = RealTimeService.Instance;
    realTimeService.send('fimsNoReply', {
      method: 'set',
      uri: clearActionURI,
      replyto: 'web_ui',
      body: true,
      username: 'web_ui',
    });
  };

  return (
    <CardContainer styleOverrides={cardContainerSx}>
      <MaintActionsDetailsModal
        open={modalOpen}
        onClose={() => setModalOpen(false)}
        label={label}
        subheader={subheader}
        status={status}
        steps={steps}
        stepIndex={stepIndex}
        stopAction={stopAction}
        clearAction={clearAction}
      />
      <Icon src={statusIcon} color={statusIconColor} />
      <Box sx={contentBoxSx}>
        <Box sx={textBoxSx}>
          <Typography variant="bodyMBold" text={label} />
          <Typography variant="bodyM" text={subheader} />
          <Box sx={stepperBoxSx}>
            <Typography variant="bodyM" text={decideStepCountText(stepIndex, steps)} />
            <Stepper orientation="horizontal">
              {stepComponents}
            </Stepper>
          </Box>
          <MuiButton sx={{ padding: 0 }} variant="text" color="inherit" label="View Details" endIcon="Share" onClick={() => setModalOpen(true)} />
        </Box>
        {
                    status === MaintenanceActionStatuses.InProgress
                      ? <MuiButton label="Stop Action" startIcon="TrashOutline" variant="text" color="error" onClick={stopAction} />
                      : <IconButton icon="Close" onClick={clearAction} />
        }
      </Box>
    </CardContainer>
  );
};

export default MaintActionProgress;
