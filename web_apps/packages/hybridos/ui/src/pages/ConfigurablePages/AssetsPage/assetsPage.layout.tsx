// TODO: figure out what's wrong with eslint
import {
  CardContainer, ResizableContainer, Tabs, ThemeType,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import {
  AlertState,
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import AssetControl from './AssetControl';
import AssetStatus from './AssetStatus';
import { tabsAndStatusContainerSx, tabsContainerSx, internalTabsAndStatusContainerSx, } from './assetsPage.styles';

export type AssetsPageLayoutProps = {
  tabValue: string;
  handleTabChange: (event: React.ChangeEvent<object>, newValue: unknown) => void;
  tabComponents: React.ReactElement[];
  assetState: ConfigurablePageStateStructure;
  alertState?: AlertState[string];
  componentFunctions?: DisplayGroupFunctions;
  allControlsState?: any;
  currentUser?: any;
};

const Window = ({ children }: { children: React.ReactNode }) => (
  <Box
    sx={{
      display: 'flex',
      flexDirection: 'column',
      flexGrow: 1,
      overflow: 'auto',
    }}
  >
    {children}
  </Box>
);

type TabsColumnProps = {
  tabComponents: React.ReactElement[];
  handleTabChange: (event: React.ChangeEvent<object>, newValue: unknown) => void;
  tabValue: string;
};

const TabsColumn = ({ tabComponents, handleTabChange, tabValue }: TabsColumnProps) => {
  const theme = useTheme() as ThemeType;

  return (
    <Box sx={tabsContainerSx(theme)}>
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
    allControlsState,
    currentUser,
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
                  assetName={componentFunctions?.displayName || 'Asset'}
                  statusChildren={componentFunctions?.statusFunctions || []}
                  assetState={assetState}
                  alertState={alertsToDisplay}
                />
              </Window>
            </Box>
          </CardContainer>
        </Box>
      )}
      draggableContent={(
        <AssetControl
          componentFunctions={componentFunctions}
          assetState={assetState}
          allControlsState={allControlsState}
          currentUser={currentUser}
        />
      )}
      maxWidth="50%"
      minWidth="15%"
      defaultWidth="20%"
    />
  );
};

export default AssetsPageLayout;
