/* eslint-disable */
// TODO: fix lint
import { Modal, TextField, Typography } from '@flexgen/storybook';
import { MFAModalSX, MFAQRCodeSX, MFATextFieldSX } from './styles';
import { useCallback, useState } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

interface MfaLoginProps {
  user: any; // TODO: Use Better Type
  onLogin: any; // TODO: Use Better Type
}

const MFALogin = (props: MfaLoginProps) => {
  const { user, onLogin } = props;
  const axiosInstance = useAxiosWebUIInstance(true);
  const [open, setOpen] = useState<boolean>(true);
  const [error, setError] = useState<string>('');
  const [totp, setTotp] = useState<string>('');

  const MFA_URL = '/login/mfa';
  const TotpTextField = TextField;

  const INVALID_ENTRY_ERROR = 'Invalid input. Ensure code field is not empty.';
  const validateTotp = () => {
    const empty = totp === '';
    return !empty;
  };

  const postMfa = () => {
    const validEntry = validateTotp();
    if (validEntry) {
      const body = {
        username: user.username,
        totp,
      };
      axiosInstance
        .post(MFA_URL, body)
        .then((res) => {
          onLogin(res.data);
        })
        .catch((e) => {
          setError(e.response?.data?.message);
        });
    } else {
      setError(INVALID_ENTRY_ERROR);
    }
  };

  const SUBMIT_ACTION = {
    label: 'authenticate',
    onClick: () => postMfa(),
    primary: true,
  };

  const CLOSE_ACTION = {
    label: 'close',
    onClick: () => setOpen(false),
    primary: false,
  };

  return (
    <div onKeyDown={(e) => e.code === 'Enter' && postMfa()}>
      <Modal
        actions={[CLOSE_ACTION, SUBMIT_ACTION]}
        children={[
          <div style={MFAModalSX}>
            <Typography
              variant='bodyS'
              text={
                user.qrCode
                  ? 'Scan the QR code with your Multifactor Authentication app to begin setup'
                  : 'Enter your time-based token from your Multifactor Authentication app'
              }
            />
          </div>,
          <div style={MFAQRCodeSX}>
            {' '}
            {user.qrCode && <img alt='QRCode' key='QRCode' src={user.qrCode} />}{' '}
          </div>,
          <div style={MFATextFieldSX}>
            <TotpTextField
              color={error !== '' ? 'error' : 'primary'}
              fullWidth
              helperText={error}
              key='TotpTextField'
              label='Time-Based Token'
              onChange={(event) => setTotp(event.target.value.trim())}
            />
          </div>, // TODO: Add prop to TextField to change helper text color.
        ]}
        onClose={() => setOpen(false)}
        open={open}
        title={user.qrCode ? 'Set up Multifactor Authentication' : 'Login with MFA'}
      />
    </div>
  );
};

export default MFALogin;
