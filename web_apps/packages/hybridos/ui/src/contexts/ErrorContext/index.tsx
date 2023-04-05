// TODO: fix lint
/* eslint-disable @typescript-eslint/no-shadow */
import {
  createContext, FC, useContext, useMemo, useState,
} from 'react';
import {
  ErrorProviderProps,
  ErrorContextType,
  ExtraPropsAndActions,
  ErrorProps,
  ModalErrorProps,
} from './types';

export const ErrorContext = createContext<ErrorContextType | null>(null);

export function useErrorContext() {
  return useContext(ErrorContext);
}

const ErrorProvider: FC<ErrorProviderProps> = ({ children }) => {
  const [modalIsOpen, setModalIsOpen] = useState<boolean>(false);
  const [modalProps, setModalProps] = useState<ModalErrorProps | undefined>();
  const [extraPropsAndActions, setExtraPropsAndActions] = useState<
  ExtraPropsAndActions | undefined
  >();

  const showErrorModal = ({
    title,
    description,
    iconType,
    errorCode,
    extraPropsAndActions,
  }: ErrorProps) => {
    setModalIsOpen(true);
    setModalProps({
      errorType: errorCode,
      errorDescription: description,
      headerTitle: title,
      iconType,
      ...extraPropsAndActions,
    });
    setExtraPropsAndActions(extraPropsAndActions);
  };

  const clearErrorModal = () => {
    setModalIsOpen(false);
    setModalProps(undefined);
    setExtraPropsAndActions({});
  };

  // eslint-disable-next-line @typescript-eslint/ban-ts-comment
  // @ts-ignore
  const context: ErrorContextType = useMemo(() => ({
    extraPropsAndActions,
    showErrorModal,
    clearErrorModal,
    modalIsOpen,
    modalProps,
  }), [extraPropsAndActions, modalIsOpen, modalProps]);

  return <ErrorContext.Provider value={context}>{children}</ErrorContext.Provider>;
};

export default ErrorProvider;
