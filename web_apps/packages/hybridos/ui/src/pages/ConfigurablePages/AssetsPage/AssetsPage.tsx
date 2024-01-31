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
    maintModeState,
    currentUser,
  } = props;

  useEffect(() => {
    const updatedTabComponents = Object.entries(componentFunctions).map(
      ([displayGroupID, displayGroup]) => {
        let icon;
        let color = 'primary';
        const alerts = alertState[displayGroupID];
        if (maintModeState && maintModeState[displayGroupID].value) icon = 'Build';
        if (alerts.faultInformation.length > 0) icon = 'Fault';
        if (alerts.alarmInformation.length > 0) icon = 'Alarm';
        if (alerts.alarmInformation.length > 0 && alerts.faultInformation.length > 0) icon = 'Fault';


        return (
          <Tab
            color={color as ColorType}
            alignment="left"
            label={displayGroup.displayName}
            value={displayGroupID}
            iconPosition="end"
            key={displayGroupID}
            icon={icon as IconList}
          />
        );
      },
    );

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
