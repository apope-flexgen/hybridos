/* eslint-disable react/jsx-no-useless-fragment */
import { ModalError } from '@flexgen/storybook';
import { useContext } from 'react';

import { ErrorContextFunctions, ErrorContextStates } from 'src/contexts/ErrorContext';
import { ErrorContextFunctionsType, ErrorContextStatesType } from 'src/contexts/ErrorContext/types';

const ErrorModal = (): JSX.Element => {
  const { modalProps, modalIsOpen } = useContext(ErrorContextStates) as ErrorContextStatesType;

  const { clearErrorModal } = useContext(ErrorContextFunctions) as ErrorContextFunctionsType;

  const render = modalIsOpen && modalProps ? (
    <ModalError isOpen={modalIsOpen} onCloseAction={clearErrorModal} {...modalProps} />
  ) : (
    <></>
  );

  return render;
};

export default ErrorModal;
