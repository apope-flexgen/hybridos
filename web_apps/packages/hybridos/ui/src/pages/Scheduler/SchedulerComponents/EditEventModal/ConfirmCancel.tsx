import { MuiButton } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React from 'react';
import { editEventLabels } from 'src/pages/Scheduler/SchedulerLabels';

export interface ConfirmCancelProps {
  // when the user hits confirm
  onConfirmClick?: () => void
  // when the user hits cancel
  onCancelClick?: () => void
  // label to display next to cofirm
  confirmLabel?: string
  // label to display next to cancel
  cancelLabel?: string
  // whether to display iconbuttons next to or on top of eachother
  displayDirection?: 'row' | 'column'
}

const ConfirmCancel: React.FunctionComponent<ConfirmCancelProps> = ({
  onConfirmClick,
  onCancelClick,
  cancelLabel,
  confirmLabel,
  displayDirection = 'row',
}) => (
  <Box sx={{ display: 'flex', flexDirection: displayDirection, gap: '8px' }}>
    <MuiButton
      color={editEventLabels.confirmCancelButton.cancelColor}
      label={cancelLabel}
      onClick={onCancelClick}
      startIcon="Close"
      variant="text"
    />
    <MuiButton
      color={editEventLabels.confirmCancelButton.confirmColor}
      label={confirmLabel}
      onClick={onConfirmClick}
      startIcon="Check"
      variant="text"
    />
  </Box>
);

export default ConfirmCancel;
