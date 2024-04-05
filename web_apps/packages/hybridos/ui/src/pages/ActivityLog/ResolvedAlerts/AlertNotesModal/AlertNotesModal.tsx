import {
  Box,
  Divider,
  IconButton,
  Modal,
  MuiButton,
  TextField,
  Typography,
} from '@flexgen/storybook';
import { useContext, useEffect, useState } from 'react';
import { useAppContext } from 'src/App/App';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { notesHeaderSx, notesDetailsSx } from 'src/pages/ActivityLog/ResolvedAlerts/resolvedAlerts.styles';
import {
  alertModalSx, cancelButtonSx, modalButtonsSx, saveButtonSx, textFieldSx,
} from 'src/pages/ActivityLog/activityLog.styles';
import { ResolvedAlertObject } from 'src/pages/ActivityLog/activityLog.types';

export interface AlertNotesModalProps {
  open: boolean;
  alertInfo: ResolvedAlertObject
  onClose: () => void;
}

const AlertNotesModal: React.FC<AlertNotesModalProps> = ({
  open,
  alertInfo,
  onClose,
}: AlertNotesModalProps) => {
  const [formView, setFormView] = useState<boolean>(false);
  const [messageError, setMessageError] = useState<boolean>(false);
  const [message, setMessage] = useState<string>(alertInfo.resolution_message);

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
        notifCtx?.notif('success', 'Resolution successfully updated');
      } else notifCtx?.notif('error', `Error updating resolution: ${res.data.body.message}`);
    });
  };

  return (
    <Modal title="Alert Resolution" open={open} onClose={onClose}>
      <Box sx={alertModalSx}>
        <Typography
          text={product === FLEET_MANAGER ? `${alertInfo.site} - ${alertInfo.details[0].message}` : `${alertInfo.details[0].message}`}
          variant="labelM"
        />
        <Typography
          text={`Triggered: ${alertInfo.trigger_time}`}
          variant="bodyS"
        />
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={notesHeaderSx}>
          <Typography
            text="Manual Resolution Entry"
            variant="labelM"
          />
          {!formView && (<IconButton icon="Edit" onClick={() => setFormView(!formView)} />)}
        </Box>
        {formView ? (
          <Box sx={textFieldSx}>
            <TextField
              value={alertInfo.resolution_message}
              fullWidth
              multiline
              minRows={5}
              maxRows={5}
              color={messageError ? 'error' : 'primary'}
              helperText={messageError ? 'Field cannot be empty' : ''}
              onChange={(event: any) => setMessage(event.target.value)}
            />
          </Box>
        )
          : (
            <Box sx={notesDetailsSx}>
              <Typography text="Resolution" variant="labelS" />
              <Typography text={alertInfo.resolution_message} />
            </Box>
          )}
        <Divider orientation="horizontal" variant="fullWidth" />
        <Box sx={modalButtonsSx}>
          <MuiButton
            label={formView ? 'Close without saving' : 'Close'}
            variant="outlined"
            onClick={formView ? () => setFormView(!formView) : onClose}
            sx={!formView ? cancelButtonSx : undefined}
          />
          <MuiButton
            label="Save"
            onClick={handleSubmit}
            disabled={!formView}
            sx={saveButtonSx}
          />
        </Box>
      </Box>
    </Modal>
  );
};

export default AlertNotesModal;
