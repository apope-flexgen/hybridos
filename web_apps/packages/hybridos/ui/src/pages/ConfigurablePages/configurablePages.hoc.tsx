/* eslint-disable react-hooks/rules-of-hooks */
/* eslint-disable max-len */
// TODO: fix lint
import { PageLoadingIndicator } from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import BatchSelectProvider from 'src/pages/ConfigurablePages/contexts/BatchSelectContext';
import { PageProps } from 'src/pages/PageTypes';
import QueryService from 'src/services/QueryService';
import { getUpdatedComponentFunctions, getUpdatedStates } from './configurablePages.helpers';
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
  AlertState,
  MaintModeState,
} from './configurablePages.types';

export interface ConfigurablePagesProps extends PageProps {
  componentState: { [key: string]: ConfigurablePageStateStructure };
  alertState: AlertState;
  componentFunctions: {
    [displayGroupID: string]: DisplayGroupFunctions;
  };
  batchControlsState?: any;
  maintModeState?: MaintModeState;
  maintenanceActionsState?: any;
}

const ConfigurablePagesHOC = <T extends ConfigurablePagesProps>(
  WrappedComponent: (props: T) => JSX.Element,
  pageName: string,
) => (props: Omit<T, keyof ConfigurablePagesProps>) => {
    const [componentState, setComponentState] = useState<{
      [key: string]: ConfigurablePageStateStructure;
    }>({});
    const [batchControlsState, setBatchControlsState] = useState<boolean>(false);
    const [maintenanceActionsState, setMaintenanceActionsState] = useState<boolean>(false);
    const [alertState, setAlertState] = useState<AlertState>({});
    const [maintModeState, setMaintModeState] = useState<MaintModeState>({});
    const [isLoading, setIsLoading] = useState<any>(false);

    const [componentFunctions, setComponentFunctions] = useState<{
      [displayGroupID: string]: DisplayGroupFunctions;
    }>({});

    const updateStateFromNewData = (
      data: ConfigurablePageDTO['displayGroups'],
      assetKey: string,
    ) => {
      const [, updatedAlertState, updatedMaintModeState] = getUpdatedStates(data);

      setMaintModeState((prevMaintModeState) => ({
        ...prevMaintModeState,
        ...updatedMaintModeState,
      }));

      setComponentState((prevComponentState) => {
        const [updatedComponentState] = getUpdatedStates(data, prevComponentState[assetKey]);
        return {
          ...prevComponentState,
          [assetKey]: { ...prevComponentState[assetKey], ...updatedComponentState },
        };
      });

      setAlertState((prevAlertState) => ({
        ...prevAlertState,
        ...updatedAlertState,
      }));
    };

    const updateComponentFunctionsFromNewData = (data: ConfigurablePageDTO['displayGroups']) => {
      setComponentFunctions(getUpdatedComponentFunctions(data));
      setIsLoading(false);
    };

    const handleNewMessage = useCallback((newInformationFromSocket: any) => {
      const data = newInformationFromSocket as ConfigurablePageDTO;
      updateStateFromNewData(data.displayGroups, data.assetKey || '');

      setBatchControlsState(data.hasBatchControls || false);
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
      <BatchSelectProvider>
        <PageLoadingIndicator isLoading={isLoading} type="primary" />
        <WrappedComponent
          {...(props as T)}
          batchControlsState={batchControlsState}
          maintModeState={maintModeState}
          maintenanceActionsState={maintenanceActionsState}
          componentState={componentState}
          alertState={alertState}
          componentFunctions={componentFunctions}
        />
      </BatchSelectProvider>
    );
  };

export default ConfigurablePagesHOC;
