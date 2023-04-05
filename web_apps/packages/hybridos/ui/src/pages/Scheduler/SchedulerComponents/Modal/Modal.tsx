import { Modal } from '@flexgen/storybook';
import { ModalStateType, labels } from './Helpers';

interface SchedulerModalProps {
  open: boolean,
  state: ModalStateType,
}

const SchedulerModal: React.FC<SchedulerModalProps> = ({
  open,
  state,
}: SchedulerModalProps) => {
  const modalActions = [];
  if (state.secondaryActions) {
    modalActions.push({
      label: labels[state.type].secondaryLabel,
      onClick: state.secondaryActions,
      primary: false,
    });
  }
  if (state.primaryActions) {
    modalActions.push({
      label: labels[state.type].primaryLabel,
      onClick: state.primaryActions,
      primary: true,
    });
  }

  return (
    <Modal
      actions={modalActions}
      description={labels[state.type].description}
      onBackdropClick={state.onClose}
      onClose={state.onClose}
      open={open}
      title={labels[state.type].title}
    />
  );
};

export default SchedulerModal;
