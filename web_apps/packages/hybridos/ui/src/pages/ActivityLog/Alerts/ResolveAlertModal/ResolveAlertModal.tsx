import {
  Box, Divider, Modal, MuiButton, TextField, Typography,
} from '@flexgen/storybook';
import { useContext, useEffect, useState } from 'react';
import { useAppContext } from 'src/App/App';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import ActiveAlertBanner from 'src/pages/ActivityLog/Alerts/ActiveAlertBanner/ActiveAlertBanner';
import {
  alertModalSx,
  cancelButtonSx,
  modalButtonsSx,
  saveButtonSx,
  textFieldSx,
} from 'src/pages/ActivityLog/activityLog.styles';
import { ActiveAlertObject } from 'src/pages/ActivityLog/activityLog.types';

export interface ResolveAlertModalProps {
  open: boolean;
  alertInfo: ActiveAlertObject;
  onClose: () => void;
}

const ResolveAlertModal: React.FC<ResolveAlertModalProps> = ({
  open,
  alertInfo,
  onClose,
}: ResolveAlertModalProps) => {
  const [messageError, setMessageError] = useState<boolean>(false);
  const [message, setMessage] = useState<string>('');

  const { product } = useAppContext();
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  useEffect(() => {
    if (message !== '') setMessageError(false);
  }, [message]);

  const handleSubmit = () => {
    if (message === '') {
      setMessageError(true);
      return;
    }

    axiosInstance.post(`/alerts/${alertInfo.id}`, { message }).then((res) => {
      if (res.data.success) {
        onClose();
        notifCtx?.notif('success', 'Alert successfully resolved');
      } else notifCtx?.notif('error', `Error resolving alert ${res.data.message || ''}`);
    });
  };

  return (
    <Modal title="Resolve Alert" open={open} onClose={onClose}>
      <Box sx={alertModalSx}>
        {alertInfo.status.toLowerCase() === 'active' && <ActiveAlertBanner />}
        {product === FLEET_MANAGER && (
          <Typography text={`Site: ${alertInfo.site}`} variant="labelM" />
        )}
        <Typography text={`Alert: ${alertInfo.details[0].message}`} variant="labelM" />
        <Typography text={`Triggered: ${alertInfo.trigger_time}`} variant="bodyS" />
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={textFieldSx}>
          <TextField
            placeholder="Enter a remediation description..."
            value={message}
            fullWidth
            multiline
            minRows={5}
            maxRows={5}
            color={messageError ? 'error' : 'primary'}
            helperText={messageError ? 'Field cannot be empty' : ''}
            onChange={(event: any) => setMessage(event.target.value)}
          />
        </Box>
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={modalButtonsSx}>
          <MuiButton label="Cancel" variant="outlined" onClick={onClose} sx={cancelButtonSx} />
          <MuiButton label="Resolve" onClick={handleSubmit} sx={saveButtonSx} />
        </Box>
      </Box>
    </Modal>
  );
};

export default ResolveAlertModal;
