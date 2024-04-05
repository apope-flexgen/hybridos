import {
  IconButton,
} from '@flexgen/storybook';
import React, { useState } from 'react';
import AlertNotesModal from 'src/pages/ActivityLog/ResolvedAlerts/AlertNotesModal/AlertNotesModal';
import { ResolvedAlertObject } from 'src/pages/ActivityLog/activityLog.types';

export interface AlertNotesButtonProps {
  alertInfo: ResolvedAlertObject
}

const AlertNotesButton: React.FC<AlertNotesButtonProps> = ({
  alertInfo,
}: AlertNotesButtonProps) => {
  const [modalOpen, setModalOpen] = useState<boolean>(false);

  return (
    <>
      {modalOpen && (
      <AlertNotesModal
        open={modalOpen}
        alertInfo={alertInfo}
        onClose={() => setModalOpen(false)}
      />
      )}
      <IconButton icon="Events" onClick={() => { setModalOpen(true); }} />
    </>
  );
};

export default AlertNotesButton;
