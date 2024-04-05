import {
  MuiButton,
} from '@flexgen/storybook';
import React, { useState } from 'react';
import ResolveAlertModal from 'src/pages/ActivityLog/Alerts/ResolveAlertModal/ResolveAlertModal';
import { ActiveAlertObject } from 'src/pages/ActivityLog/activityLog.types';

export interface ResolveAlertButtonProps {
  alertInfo: ActiveAlertObject
}

const ResolveAlertButton: React.FC<ResolveAlertButtonProps> = ({
  alertInfo,
}: ResolveAlertButtonProps) => {
  const [modalOpen, setModalOpen] = useState<boolean>(false);

  return (
    <>
      {modalOpen && (
      <ResolveAlertModal
        open={modalOpen}
        alertInfo={alertInfo}
        onClose={() => setModalOpen(false)}
      />
      )}
      <MuiButton label="Resolve" variant="text" onClick={() => { setModalOpen(true); }} />
    </>
  );
};

export default ResolveAlertButton;
