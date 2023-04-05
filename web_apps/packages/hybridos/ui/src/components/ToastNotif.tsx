import { IconButton, NotificationColor, Snackbar, Box } from '@flexgen/storybook';
import { useContext } from 'react';
import { NotifContext, NotifContextType } from 'src/contexts/NotifContext';

const ToastNotif = (): JSX.Element => {
  const {
    severity, message, action, clear,
  } = useContext(NotifContext) as NotifContextType;

  const closeButtonAndAction = (
    <>
      {action}
      <IconButton
        color={severity === 'info' ? 'primary' : (severity as NotificationColor)}
        icon="Close"
        onClick={() => clear()}
      />
    </>
  );

  return (
    <Snackbar
      action={closeButtonAndAction}
      anchorOrigin={{ horizontal: 'right', vertical: 'bottom' }}
      autoHideDuration={7000}
      color={severity === 'info' ? 'primary' : (severity as NotificationColor)}
      id="test-notif"
      message={message || ''}
      onClose={() => clear()}
      open={message !== null}
    />
  );
};

export default ToastNotif;
