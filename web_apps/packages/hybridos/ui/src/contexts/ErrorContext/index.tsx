// TODO: fix lint
/* eslint-disable @typescript-eslint/no-shadow */
import {
  createContext, FC, useContext, useEffect, useMemo, useState,
} from 'react';
import QueryService from 'src/services/QueryService';
import {
  ErrorProviderProps,
  ExtraPropsAndActions,
  ErrorProps,
  ModalErrorProps,
  ErrorContextFunctionsType,
  ErrorContextStatesType,
} from './types';

export const ErrorContextFunctions = createContext<ErrorContextFunctionsType | null>(null);
export const ErrorContextStates = createContext<ErrorContextStatesType | null>(null);

export function useErrorContextFunctions() {
  return useContext(ErrorContextFunctions);
}

export function useErrorContextStates() {
  return useContext(ErrorContextStates);
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
  const functionsContext: ErrorContextType = useMemo(
    () => ({
      showErrorModal,
      clearErrorModal,
    }),
    [],
  );

  const statesContext = useMemo(
    () => ({ modalProps, modalIsOpen, extraPropsAndActions }),
    [extraPropsAndActions, modalIsOpen, modalProps],
  );

  useEffect(() => {
    QueryService.getWsException((data) => {
      showErrorModal({
        title: 'Error',
        description: data.message,
      });
    });

    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  return (
    <ErrorContextFunctions.Provider value={functionsContext}>
      <ErrorContextStates.Provider value={statesContext}>{children}</ErrorContextStates.Provider>
    </ErrorContextFunctions.Provider>
  );
};

export default ErrorProvider;
