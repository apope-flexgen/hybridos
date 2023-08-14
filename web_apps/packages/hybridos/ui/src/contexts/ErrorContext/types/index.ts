import { ReactNode } from 'react';

// TODO: Expose this in storybook and import here.
type IconType = 'warning' | 'error' | 'info';

export type ErrorContextFunctionsType = {
  showErrorModal: ({
    title,
    description,
    iconType,
    errorCode,
    extraPropsAndActions,
  }: ErrorProps) => void;
  clearErrorModal: () => void;
};
export type ErrorContextStatesType = {
  modalProps: ModalErrorProps | undefined;
  extraPropsAndActions: ExtraPropsAndActions | undefined;
  modalIsOpen: boolean;
};

export interface ErrorProviderProps {
  children: ReactNode;
}

export interface ExtraPropsAndActions {
  /** Flag to show/hide the Confirm Button */
  hasConfirmButton?: boolean;
  /** Assign label to the Confirm Button */
  confirmButtonLabel?: string;
  /** Receive the action for the Confirm Button */
  confirmButtonAction?: () => void;
  /** Flag to show/hide the Cancel Button */
  hasCancelButton?: boolean;
  /** Assign label to the Cancel Button */
  cancelButtonLabel?: string;
  /** Receive the action for the Cancel Button */
  cancelButtonAction?: () => void;
}

export interface ModalErrorProps extends ExtraPropsAndActions {
  /** Give a title to the header */
  headerTitle?: string;
  /**
   * The type of error returned by some API
   * usually the code (502, 504, 404, etc)
   * */
  errorType?: string | number;
  /** The error/warning description */
  errorDescription: string;
  /** Icon type, could be warning or error */
  iconType?: IconType;
}

export interface ErrorProps {
  title: string;
  description: string;
  iconType?: IconType;
  errorCode?: string | number;
  extraPropsAndActions?: ExtraPropsAndActions | undefined;
}
