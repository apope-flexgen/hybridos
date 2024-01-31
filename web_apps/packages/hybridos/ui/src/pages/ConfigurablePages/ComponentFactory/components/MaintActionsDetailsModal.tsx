import { Box, Modal, MuiButton, Stepper } from '@flexgen/storybook';
import { MaintenanceActionStep } from 'shared/types/dtos/configurablePages.dto';
import { MaintenanceActionStatuses } from './MaintActionProgress';
import { createStepComponents, modalContentSx, halfWidth, buttonBoxSx } from './maintActions.helpers';

export interface MaintActionsDetailsModalProps {
    open: boolean;
    label: string;
    subheader: string;
    status: MaintenanceActionStatuses;
    steps: MaintenanceActionStep[];
    stepIndex: number;
    onClose: () => void;
    stopAction: () => void;
    clearAction: () => void;
}

const MaintActionsDetailsModal: React.FC<MaintActionsDetailsModalProps> = ({
    open,
    label,
    steps,
    stepIndex,
    subheader,
    status,
    onClose,
    stopAction,
    clearAction,
}: MaintActionsDetailsModalProps) => {
    const stepComponents = createStepComponents(steps, stepIndex, status, 'vertical');
    const stopAndClose = () => {
        stopAction();
        onClose();
    }
    const clearAndClose = () => {
        clearAction();
        onClose();
    };

    return (
        <Modal open={open} onClose={onClose} title={label} description={subheader}>
            <Box sx={modalContentSx}>
                <Stepper orientation='vertical' >
                    {stepComponents}
                </Stepper>
                <Box sx={buttonBoxSx}>
                    <MuiButton sx={halfWidth} label="Cancel" variant='outlined' color='inherit' onClick={onClose} />
                    {
                        status === MaintenanceActionStatuses.InProgress ?
                            <MuiButton sx={halfWidth} label='Stop Action' color='error' onClick={stopAndClose} />
                            : <MuiButton sx={halfWidth} label='Clear Action' onClick={clearAndClose} />
                    }
                </Box>
            </Box>
        </Modal>
    );
};

export default MaintActionsDetailsModal;
