// TODO: fix lint
/* eslint-disable react/no-children-prop */
import { Modal, TextField, Typography } from '@flexgen/storybook';
import { useState } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

interface PassExpLoginProps {
  user: any // TODO: Use Better Type
  onLogin: any // TODO: Use Better Type
}

const PassExpLogin = (props: PassExpLoginProps) => {
  const { user, onLogin } = props;
  const axiosInstance = useAxiosWebUIInstance();
  const [open, setOpen] = useState<boolean>(true);
  const [error, setError] = useState<string>('');
  const [newPassword, setNewPassword] = useState<string>('');
  const [confirmPassword, setConfirmPassword] = useState<string>('');

  const PASS_EXP_URL = '/login/passExp';
  const NewPasswordTextfield = TextField;
  const ConfirmPasswordTextfield = TextField;

  const INVALID_ENTRY_ERROR = 'Invalid input. Ensure passwords are not empty and do not match each other.';

  const validatePasswords = () => {
    const newPasswordEmpty = newPassword === '';
    const confirmPasswordEmpty = confirmPassword === '';
    const passwordsMatch = newPassword === confirmPassword;
    return !newPasswordEmpty && !confirmPasswordEmpty && passwordsMatch;
  };

  const postPassExp = () => {
    const validPasswords = validatePasswords();
    if (validPasswords) {
      const body = {
        username: user.username,
        updatedPassword: newPassword,
      };
      axiosInstance.post(PASS_EXP_URL, body).then((res) => {
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
    label: 'Create new password',
    onClick: () => postPassExp(),
    primary: true,
  };

  return (
    <Modal
      actions={[SUBMIT_ACTION]}
      children={[
        <div style={{ marginLeft: '8%', marginBottom: '10%' }}>
          <Typography text="Please create a new password to continue." />
        </div>,
        <div style={{ display: 'flex', flexDirection: 'column', alignContent: 'center', width: '60%', paddingBottom: '5%', margin: 'auto' }}>
          <NewPasswordTextfield
            color="primary"
            key="NewPasswordTextfield"
            label="New Password"
            onChange={(event) => setNewPassword(event.target.value.trim())}
            type="password"
          />
          <br />
          <ConfirmPasswordTextfield
            color="primary"
            helperText={error}
            key="ConfirmPasswordTextfield"
            label="Confirm Password"
            onChange={(event) => setConfirmPassword(event.target.value.trim())}
            type="password"
          />
          <br />
        </div>,
      ]}
      onClose={() => setOpen(false)}
      open={open}
      title="Your password has expired"
    />
  );
};

export default PassExpLogin;
