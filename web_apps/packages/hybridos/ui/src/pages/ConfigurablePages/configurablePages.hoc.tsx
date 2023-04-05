/* eslint-disable react-hooks/rules-of-hooks */
import { useCallback, useEffect, useState } from 'react';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import QueryService from 'src/services/QueryService';
import { getUpdatedComponentFunctions, getUpdatedStates } from './configurablePages.helpers';
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
  AlertState,
} from './configurablePages.types';

export type ConfigurablePagesProps = {
  componentState: ConfigurablePageStateStructure
  alertState: AlertState
  componentFunctions: {
    [displayGroupID: string]: DisplayGroupFunctions
  }
};

const ConfigurablePagesHOC = <T extends ConfigurablePagesProps>(
  WrappedComponent: (props: T) => JSX.Element,
  pageName: string,
) => (props: Omit<T, keyof ConfigurablePagesProps>) => {
    const [componentState, setComponentState] = useState<ConfigurablePageStateStructure>({});
    const [alertState, setAlertState] = useState<AlertState>({});
    const [componentFunctions, setComponentFunctions] = useState<{
      [displayGroupID: string]: DisplayGroupFunctions
    }>({});

    const updateStateFromNewData = (data: ConfigurablePageDTO['displayGroups']) => {
      const [updatedComponentState, updatedAlertState] = getUpdatedStates(data);

      setComponentState((prevComponentState) => ({
        ...prevComponentState,
        ...updatedComponentState,
      }));

      setAlertState(updatedAlertState);
    };

    const updateComponentFunctionsFromNewData = (
      data: ConfigurablePageDTO['displayGroups'],
    ) => {
      const updatedComponentFunctions = getUpdatedComponentFunctions(data);

      setComponentFunctions(updatedComponentFunctions);
    };

    const handleNewMessage = useCallback((newInformationFromSocket: MessageEvent) => {
      const data = JSON.parse(newInformationFromSocket.data) as ConfigurablePageDTO;

      updateStateFromNewData(data.displayGroups);

      if (!data.hasStatic) return;

      updateComponentFunctionsFromNewData(data.displayGroups);
    }, []);

    useEffect(() => {
      if ('category' in props) {
        const { category } = props;
        QueryService.getConfigurablePage(
          handleNewMessage,
          pageName,
          category as string,
        );
      } else {
        QueryService.getConfigurablePage(handleNewMessage, pageName);
      }

      return () => {
        QueryService.cleanupSocket();
      };
    }, [handleNewMessage, props]);

    return (
      <WrappedComponent
        {...(props as T)}
        componentState={componentState}
        alertState={alertState}
        componentFunctions={componentFunctions}
      />
    );
  };

export default ConfigurablePagesHOC;
