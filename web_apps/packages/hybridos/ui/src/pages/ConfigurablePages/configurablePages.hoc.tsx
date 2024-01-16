/* eslint-disable react-hooks/rules-of-hooks */
/* eslint-disable max-len */
// TODO: fix lint
import { PageLoadingIndicator } from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { PageProps } from 'src/pages/PageTypes';
import QueryService from 'src/services/QueryService';
import { getUpdatedComponentFunctions, getUpdatedStates } from './configurablePages.helpers';
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
  AlertState,
  MaintModeState,
  MaintActionsControlState,
} from './configurablePages.types';

export interface ConfigurablePagesProps extends PageProps {
  componentState: ConfigurablePageStateStructure;
  alertState: AlertState;
  componentFunctions: {
    [displayGroupID: string]: DisplayGroupFunctions;
  };
  allControlsState?: any;
  maintModeState?: MaintModeState;
  maintenanceActionsState?: any;
  maintActionsControlState?: MaintActionsControlState;
}

const ConfigurablePagesHOC = <T extends ConfigurablePagesProps>(
  WrappedComponent: (props: T) => JSX.Element,
  pageName: string,
) => (props: Omit<T, keyof ConfigurablePagesProps>) => {
    const [componentState, setComponentState] = useState<ConfigurablePageStateStructure>({});
    const [allControlsState, setAllControlsState] = useState<boolean>(false);
    const [maintenanceActionsState, setMaintenanceActionsState] = useState<boolean>(false);
    const [alertState, setAlertState] = useState<AlertState>({});
    const [maintModeState, setMaintModeState] = useState<MaintModeState>({});
    const [maintActionsControlState, setMaintActionsControlState] = useState<MaintActionsControlState>({});
    const [isLoading, setIsLoading] = useState<any>(false);

    const [componentFunctions, setComponentFunctions] = useState<{
      [displayGroupID: string]: DisplayGroupFunctions;
    }>({});

    const updateStateFromNewData = (data: ConfigurablePageDTO['displayGroups']) => {
      const [
        updatedComponentState,
        updatedAlertState,
        updatedMaintModeState,
        updatedMaintActionsControlState,
      ] = getUpdatedStates(data);

      setMaintModeState((prevMaintModeState) => ({
        ...prevMaintModeState,
        ...updatedMaintModeState,
      }));

      setMaintActionsControlState((prevMaintActionsControlState) => ({
        ...prevMaintActionsControlState,
        ...updatedMaintActionsControlState,
      }));

      setComponentState((prevComponentState) => ({
        ...prevComponentState,
        ...updatedComponentState,
      }));

      setAlertState((prevAlertState) => ({
        ...prevAlertState,
        ...updatedAlertState,
      }));
    };

    const updateComponentFunctionsFromNewData = (data: ConfigurablePageDTO['displayGroups']) => {
      const updatedComponentFunctions = getUpdatedComponentFunctions(data);

      setComponentFunctions(updatedComponentFunctions);
      setIsLoading(false);
    };

    const handleNewMessage = useCallback((newInformationFromSocket: any) => {
      const data = newInformationFromSocket as ConfigurablePageDTO;

      updateStateFromNewData(data.displayGroups);

      setAllControlsState(data.hasAllControls || false);
      setMaintenanceActionsState(data.hasMaintenanceActions || false);

      if (!data.hasStatic) return;

      updateComponentFunctionsFromNewData(data.displayGroups);
    }, []);

    useEffect(() => {
      setIsLoading(true);
      if ('assetKey' in props) {
        const { assetKey } = props;
        QueryService.getConfigurablePage(handleNewMessage, pageName, assetKey as string);
      } else {
        QueryService.getConfigurablePage(handleNewMessage, pageName);
      }

      return () => {
        QueryService.cleanupSocket();
      };
    }, [handleNewMessage, props]);

    return (
      <>
        <PageLoadingIndicator isLoading={isLoading} type="primary" />
        <WrappedComponent
          {...(props as T)}
          allControlsState={allControlsState}
          maintModeState={maintModeState}
          maintenanceActionsState={maintenanceActionsState}
          maintActionsControlState={maintActionsControlState}
          componentState={componentState}
          alertState={alertState}
          componentFunctions={componentFunctions}
        />
      </>
    );
  };

export default ConfigurablePagesHOC;
