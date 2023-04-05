import {
  createContext, FC, ReactNode, useMemo, useState,
} from 'react';

export type NotifSeverityType = 'info' | 'success' | 'error' | 'warning';

export type NotifContextType = {
  severity: NotifSeverityType | null;
  message: string | null;
  action?: React.ReactNode;
  notif: (
    newSeverity: NotifSeverityType,
    newMessage: string,
    newAction?: React.ReactNode
  ) => void;
  clear: () => void;
};

interface Props {
  children: ReactNode
}

export const NotifContext = createContext<NotifContextType | null>(null);

const NotifProvider: FC<Props> = ({ children }) => {
  const [message, setMessage] = useState<string | null>(null);
  const [severity, setSeverity] = useState<NotifSeverityType | null>(null);
  const [action, setAction] = useState<React.ReactNode>();

  const notif = (
    newSeverity: NotifSeverityType,
    newMessage: string,
    newAction?: React.ReactNode,
  ) => {
    window.scroll(0, 0);
    setSeverity(newSeverity);
    setMessage(newMessage);
    setAction(newAction);
  };

  const clear = () => setMessage(null);

  const context = useMemo(() => ({
    severity, message, action, notif, clear,
  }), [action, message, severity]);

  return (
    <NotifContext.Provider value={context}>
      {children}
    </NotifContext.Provider>
  );
};

export default NotifProvider;
