/* eslint-disable max-lines */
import {
  CardContainer,
  ResizableContainer,
  Tabs,
  ThemeType,
  customMUIScrollbar,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import React from 'react';
import {
  AlertState,
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import AssetControl from './AssetControl';
import AssetStatus from './AssetStatus';
import {
  tabsAndStatusContainerSx,
  getTabsContainerSx,
  internalTabsAndStatusContainerSx,
} from './assetsPage.styles';

export type AssetsPageLayoutProps = {
  tabValue: string;
  maintModeStatus?: { value: boolean };
  handleTabChange: (event: React.ChangeEvent<object>, newValue: unknown) => void;
  tabComponents: React.ReactElement[];
  assetState: ConfigurablePageStateStructure;
  alertState?: AlertState[string];
  componentFunctions?: DisplayGroupFunctions;
  batchControlsState?: any;
  maintenanceActionsState?: boolean;
  assetKey?: string;
};

const Window = ({ children }: { children: React.ReactNode }) => {
  const theme = useTheme() as ThemeType;

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        flexGrow: 1,
        overflow: 'auto',
        backgroundColor: theme.fgc.assetsPageTemplate.statusContentBg,
        ...customMUIScrollbar(theme),
      }}
    >
      {children}
    </Box>
  );
};

type TabsColumnProps = {
  tabComponents: React.ReactElement[];
  handleTabChange: (event: React.ChangeEvent<object>, newValue: unknown) => void;
  tabValue: string;
};

const TabsColumn = ({ tabComponents, handleTabChange, tabValue }: TabsColumnProps) => {
  const theme = useTheme() as ThemeType;

  return (
    <Box sx={getTabsContainerSx(theme)}>
      <Tabs onChange={handleTabChange} orientation="vertical" value={tabValue} variant="scrollable">
        {tabComponents}
      </Tabs>
    </Box>
  );
};

const AssetsPageLayout = (props: AssetsPageLayoutProps) => {
  const {
    tabValue,
    handleTabChange,
    tabComponents,
    assetState,
    alertState,
    componentFunctions,
    batchControlsState,
    assetKey,
    maintModeStatus,
    maintenanceActionsState,
  } = props;

  const alertsToDisplay = alertState || { faultInformation: [], alarmInformation: [] };

  return (
    <ResizableContainer
      reactiveContent={(
        <Box sx={tabsAndStatusContainerSx}>
          <CardContainer>
            <Box sx={internalTabsAndStatusContainerSx}>
              <TabsColumn
                tabComponents={tabComponents}
                handleTabChange={handleTabChange}
                tabValue={tabValue}
              />
              <Window>
                <AssetStatus
                  maintModeStatus={maintModeStatus?.value}
                  assetName={componentFunctions?.displayName || 'Asset'}
                  statusChildren={componentFunctions?.statusFunctions || []}
                  assetState={assetState}
                  alertState={alertsToDisplay}
                  maintenanceActionsState={maintenanceActionsState || false}
                  maintenanceActionsChildren={componentFunctions?.maintenanceActionsFunctions || []}
                />
              </Window>
            </Box>
          </CardContainer>
        </Box>
      )}
      draggableContent={(
        <AssetControl
          isSummaryTab={tabValue?.toLowerCase().includes('summary') || false}
          componentFunctions={componentFunctions}
          assetState={assetState}
          batchControlsState={batchControlsState}
          assetKey={assetKey}
        />
      )}
      maxWidth="50%"
      minWidth="15%"
      defaultWidth="20%"
    />
  );
};

export default AssetsPageLayout;
