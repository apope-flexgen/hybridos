// TODO: fix lint
/* eslint-disable max-lines */
import {
  Box, IconButton, MuiButton, PageLoadingIndicator, Tab, Tabs, Tooltip, Typography,
} from '@flexgen/storybook';
import isEqual from 'lodash.isequal';
import {
  createContext, useCallback, useContext, useEffect, useMemo, useState,
} from 'react';
import { Dashboard } from 'shared/types/dtos/dashboards.dto';
import { CANCEL, DUPLICATE, SAVE } from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  BoxSX, ButtonsContainer, Item, ListContainer, MainBoxSX, MuiButtonSX, TabContainer, Toolbar,
} from 'src/pages/UIConfig/TabContent/styles';
import { axiosWebUIInstance } from 'src/services/axios';
import TabContent from './TabContent';
import {
  ADD_NEW_DASHBOARD_CARD, DELETE_CARD, newDashboard, tabOptions,
} from './helpers';
import AddMuiButtonSX from './styles';

const DASHBOARDS_URL = '/dashboards';
interface IDashboardsContext {
  dashboards: Dashboard[],
  selectedDashboard: Dashboard | null,
  setDashboards: React.Dispatch<React.SetStateAction<Dashboard[]>>,
  setSelectedDashboard: React.Dispatch<React.SetStateAction<Dashboard | null>>,
  handleAddDashboard: () => void,
  selectedDashboardIndex: number | null,
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>,
}

const DashboardsContext = createContext<IDashboardsContext>({
  dashboards: [],
  selectedDashboard: null,
  setDashboards: () => [],
  setSelectedDashboard: () => null,
  handleAddDashboard: () => { },
  selectedDashboardIndex: null,
  setIsLoading: () => false,
});

export function useDashboardsContext() {
  return useContext(DashboardsContext);
}

// TODO: fix lint
// eslint-disable-next-line max-statements
const Dashboards = () => {
  const [selectedTab, setSelectedTab] = useState<null | string>(null);
  const [dashboards, setDashboards] = useState<Dashboard[]>([]);
  const [selectedDashboard, setSelectedDashboard] = useState<null | any>(null);
  const [selectedDashboardIndex, setSelectedDashboardIndex] = useState<number | null>(null);
  const [isLoading, setIsLoading] = useState(false);

  const handleAddDashboard = useCallback(() => {
    setDashboards((prevDashboards) => [
      newDashboard,
      ...prevDashboards,
    ]);
    setSelectedTab('info');
    setSelectedDashboard(newDashboard);
    setSelectedDashboardIndex(0);
  }, []);

  const contextValue = useMemo(() => ({
    dashboards,
    setDashboards,
    selectedDashboard,
    setSelectedDashboard,
    handleAddDashboard,
    selectedDashboardIndex,
    setIsLoading,
  }), [
    dashboards,
    setDashboards,
    selectedDashboard,
    setSelectedDashboard,
    handleAddDashboard,
    selectedDashboardIndex,
    setIsLoading,
  ]);

  const fetchDashboards = async () => {
    try {
      setIsLoading(true);
      const res = await axiosWebUIInstance.get(DASHBOARDS_URL);
      const { data }: { data: Dashboard[] } = res.data;
      setDashboards(data);
      if (data.length) {
        setSelectedTab('info');
        setSelectedDashboard(data[0]);
        setSelectedDashboardIndex(0);
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleDashboardClick = (dashboard: Dashboard, index: number) => {
    setSelectedDashboard(dashboard);
    setSelectedDashboardIndex(index);
  };

  const handleSaveClick = async () => {
    try {
      setIsLoading(true);
      const data = [...dashboards];
      data.splice(
        selectedDashboardIndex !== null ? selectedDashboardIndex : -1,
        1,
        selectedDashboard || newDashboard,
      );
      const res = await axiosWebUIInstance.post(
        DASHBOARDS_URL,
        { data },
      );
      const updatedDashboards = res.data.data;
      setDashboards(updatedDashboards);
    } finally {
      setIsLoading(false);
    }
  };

  const handleDeleteClick = async () => {
    try {
      setIsLoading(true);
      const data = [...dashboards].filter((_, index) => index !== selectedDashboardIndex);
      const res = await axiosWebUIInstance.post(
        DASHBOARDS_URL,
        { data },
      );
      const updatedDashboards = res.data.data;
      setDashboards(updatedDashboards);
      if (updatedDashboards.length) {
        setSelectedDashboard(updatedDashboards[0]);
        setSelectedDashboardIndex(0);
      } else {
        setSelectedTab(null);
        setSelectedDashboard(null);
        setSelectedDashboardIndex(null);
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleCancelClick = () => {
    setSelectedDashboard(dashboards[selectedDashboardIndex || 0]);
  };

  const handleCopyClick = () => {
    setDashboards((prevDashboards) => [dashboards[selectedDashboardIndex || 0], ...prevDashboards]);
    setSelectedDashboard(dashboards[selectedDashboardIndex || 0]);
    setSelectedDashboardIndex(0);
    setSelectedTab('info');
  };

  const disableCancel = useMemo(
    () => isEqual(dashboards[selectedDashboardIndex || 0], selectedDashboard),
    [dashboards, selectedDashboard, selectedDashboardIndex],
  );

  useEffect(() => {
    fetchDashboards();
  }, []);

  return (
    <DashboardsContext.Provider value={contextValue}>
      <TabContainer>
        <Box sx={MainBoxSX}>
          <PageLoadingIndicator isLoading={isLoading} type="primary" />
          <Typography text="Dashboard Cards" variant="bodyLBold" />
          <ListContainer>
            {dashboards.map((dashboard, index) => (
              // TODO: fix lint
              // eslint-disable-next-line react/no-array-index-key
              <Item key={index}>
                <MuiButton
                  color="inherit"
                  label={dashboard.info.name || 'New Item'}
                  onClick={() => handleDashboardClick(dashboard, index)}
                  sx={MuiButtonSX}
                  variant={index === selectedDashboardIndex ? 'contained' : 'outlined'}
                />
                {index === selectedDashboardIndex && (
                <Tooltip title={DUPLICATE}>
                  <IconButton
                    color="action"
                    icon="ContentCopy"
                    onClick={handleCopyClick}
                  />
                </Tooltip>

                )}
              </Item>
            ))}
            <MuiButton
              fullWidth
              label={ADD_NEW_DASHBOARD_CARD}
              onClick={handleAddDashboard}
              size="small"
              startIcon="Add"
              sx={AddMuiButtonSX}
              variant="outlined"
            />
          </ListContainer>
        </Box>
        <Box sx={BoxSX}>
          {selectedDashboard && (
            <Toolbar>
              <Typography text={selectedDashboard.info.name} variant="bodyMBold" />
              <ButtonsContainer>
                <MuiButton
                  color="inherit"
                  disabled={disableCancel}
                  label={CANCEL}
                  onClick={handleCancelClick}
                  variant="text"
                />
                <MuiButton
                  color="error"
                  label={DELETE_CARD}
                  onClick={handleDeleteClick}
                  variant="outlined"
                />
                <MuiButton
                  label={SAVE}
                  onClick={handleSaveClick}
                  variant="contained"
                />
              </ButtonsContainer>
            </Toolbar>
          )}
          <Tabs onChange={(_, tab) => setSelectedTab(tab as string)} value={selectedTab}>
            {tabOptions.map(({ label, value }) => (
              <Tab
                disabled={!dashboards.length}
                key={value}
                label={label}
                value={value}
              />
            ))}
          </Tabs>
          <TabContent selectedTab={selectedTab} />
        </Box>
      </TabContainer>
    </DashboardsContext.Provider>
  );
};

export default Dashboards;
