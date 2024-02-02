import { IconList, Tab, ColorType } from '@flexgen/storybook';
import React, { useState, useEffect } from 'react';
import ConfigurablePagesHOC from 'src/pages/ConfigurablePages/configurablePages.hoc';
import AssetsPageLayout from './assetsPage.layout';
import { AssetsPageProps } from './assetsPage.types';

const AssetsPage = (props: AssetsPageProps) => {
  const [tabComponents, setTabComponents] = useState<React.ReactElement[]>([]);
  const [tabValue, setTabValue] = useState('');

  const {
    componentState,
    alertState,
    componentFunctions,
    allControlsState,
    maintenanceActionsState,
    maintModeState,
    currentUser,
  } = props;

  useEffect(() => {
    const updatedTabComponents: JSX.Element[] = [];
    Object.entries(componentFunctions).forEach(([displayGroupID, displayGroup]) => {
      const { tabKey } = displayGroup;
      const existingTabKeyComponent = updatedTabComponents
        .find((component) => component.key === tabKey);

      if (existingTabKeyComponent) {
        Object.entries(displayGroup.statusFunctions).forEach(([, statusFunction]) => {
          if (!componentFunctions[existingTabKeyComponent.props.value]
            .statusFunctions.includes(statusFunction)) {
            componentFunctions[existingTabKeyComponent.props.value]
              .statusFunctions.push(statusFunction);
          }
        });
        Object.entries(displayGroup.controlFunctions).forEach(([, controlFunction]) => {
          if (!componentFunctions[existingTabKeyComponent.props.value]
            .controlFunctions.includes(controlFunction)) {
            componentFunctions[existingTabKeyComponent.props.value]
              .controlFunctions.push(controlFunction);
          }
        });
      } else {
        let icon;
        const color = 'primary';
        const alerts = alertState[displayGroupID];
        if (alerts === undefined && maintModeState === undefined) icon = undefined;
        else if (alerts.faultInformation.length > 0) icon = 'Fault';
        else if (alerts.alarmInformation.length > 0) icon = 'Alarm';
        else if (maintModeState && maintModeState[displayGroupID].value) icon = 'Build';
        if (alerts.alarmInformation.length > 0 && alerts.faultInformation.length > 0) icon = 'Fault';

        updatedTabComponents.push(
          <Tab
            color={color as ColorType}
            alignment="left"
            label={displayGroup.displayName}
            value={displayGroupID}
            iconPosition="end"
            key={displayGroup.tabKey}
            icon={icon as IconList}
          />,
        );
      }
    });

    updatedTabComponents.sort((a, b) => {
      if (a.props.label.toLowerCase() === 'summary') return -1;
      if (b.props.label.toLowerCase() === 'summary') return 1;
      return a.props.label.localeCompare(b.props.label, undefined, { numeric: true });
    });
    setTabComponents(updatedTabComponents);
    setTabValue((prev) => {
      if (prev in componentFunctions) {
        return prev;
      }
      return updatedTabComponents[0]?.props?.value;
    });
  }, [componentFunctions, alertState, maintModeState]);

  const handleTabChange = (_: React.ChangeEvent<object>, newValue: unknown) => {
    setTabValue(newValue as string);
  };

  return (
    <AssetsPageLayout
      allControlsState={allControlsState}
      maintenanceActionsState={maintenanceActionsState}
      componentFunctions={componentFunctions[tabValue]}
      handleTabChange={handleTabChange}
      tabComponents={tabComponents}
      tabValue={tabValue}
      assetState={componentState}
      alertState={alertState[tabValue]}
      currentUser={currentUser}
      maintModeStatus={maintModeState?.[tabValue]}
    />
  );
};

export default ConfigurablePagesHOC(AssetsPage, 'assetsPage');
