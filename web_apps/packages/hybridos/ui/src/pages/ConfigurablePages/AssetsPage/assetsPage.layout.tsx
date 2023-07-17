// TODO: figure out what's wrong with eslint
import { CardContainer, Tabs, ThemeType } from '@flexgen/storybook';
import { Box } from '@mui/material';
import {
  AlertState,
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import AssetControl from './AssetControl';
import AssetStatus from './AssetStatus';

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
      padding: '2rem',
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
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        minWidth: '15%',
        backgroundColor: theme.fgd.primary.main_12p,
        height: '100%',
      }}
    >
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
    <>
      <Box sx={{ display: 'flex', flexDirection: 'row', width: '80%' }}>
        <CardContainer>
          <Box
            sx={{
              display: 'flex',
              flexDirection: 'row',
              height: '1px',
              minHeight: '100%',
              width: '100%',
              overflow: 'auto',
            }}
          >
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
      <Box sx={{ width: '20%', flexShrink: 0 }}>
        <AssetControl
          componentFunctions={componentFunctions}
          assetState={assetState}
          allControlsState={allControlsState}
          currentUser={currentUser}
        />
      </Box>
    </>
  );
};

export default AssetsPageLayout;
