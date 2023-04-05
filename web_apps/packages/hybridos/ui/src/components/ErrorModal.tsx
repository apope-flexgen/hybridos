import { ModalError } from '@flexgen/storybook';
import { useContext } from 'react';
import { ErrorContext } from 'src/contexts/ErrorContext';
import { ErrorContextType } from 'src/contexts/ErrorContext/types';

const ErrorModal = (): JSX.Element => {
  const { modalProps, modalIsOpen, clearErrorModal } = useContext(
    ErrorContext,
  ) as ErrorContextType;

  const render = modalIsOpen && modalProps ? (
    <ModalError isOpen={modalIsOpen} onCloseAction={clearErrorModal} {...modalProps} />
  ) : (
    // TODO: fix lint
    // eslint-disable-next-line react/jsx-no-useless-fragment
    <></>
  );

  return render;
};

export default ErrorModal;
