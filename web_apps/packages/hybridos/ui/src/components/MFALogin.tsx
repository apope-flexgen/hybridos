// TODO: fix lint
/* eslint-disable react/no-children-prop */
import { Modal, TextField } from '@flexgen/storybook';
import { useState } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

interface MfaLoginProps {
  user: any // TODO: Use Better Type
  onLogin: any // TODO: Use Better Type
}

const MFALogin = (props: MfaLoginProps) => {
  const { user, onLogin } = props;
  const axiosInstance = useAxiosWebUIInstance();
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
      axiosInstance.post(MFA_URL, body).then((res) => {
        onLogin(res.data);
      }).catch((e) => {
        setError(e.response?.data?.message);
      });
    } else {
      setError(INVALID_ENTRY_ERROR);
    }
  };

  // TODO: Handle key down ("enter")
  const SUBMIT_ACTION = {
    label: 'submit',
    onClick: () => postMfa(),
    primary: true,
  };

  return (
    <Modal
      actions={[SUBMIT_ACTION]}
      children={[
        <>
          {' '}
          {user.qrCode && <img alt="QRCode" key="QRCode" src={user.qrCode} />}
          {' '}
        </>,
        <TotpTextField
          color="primary"
          fullWidth
          helperText={error}
          key="TotpTextField"
          label="Code"
          onChange={(event) => setTotp(event.target.value.trim())}
        />, // TODO: Add prop to TextField to change helper text color.
      ]}
      onClose={() => setOpen(false)}
      open={open}
      title="Login with MFA"
    />
  );
};

export default MFALogin;
