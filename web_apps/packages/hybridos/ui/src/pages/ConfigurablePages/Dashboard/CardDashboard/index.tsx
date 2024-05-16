// TODO: fix lint
/* eslint-disable react/no-array-index-key, @typescript-eslint/no-shadow */
import { CardContainer, CardHeader, CardRow } from '@flexgen/storybook';

import { Box } from '@mui/material';
import Grid from '@mui/system/Unstable_Grid';
import {
  dashCardContainerSx,
  dashCardHeaderSx,
  dashCardRowSx,
  gridContainerSx,
} from 'src/pages/ConfigurablePages/Dashboard/dashboard.styles';
import ConfigurablePagesHOC, {
  ConfigurablePagesProps,
} from 'src/pages/ConfigurablePages/configurablePages.hoc';

import {
  getCardColumns,
  getCardColumnSX,
  getDashboardIcon,
  getDataPointColumns,
} from './CardDashboard.helpers';

export type CardDashboardProps = Pick<
ConfigurablePagesProps,
'componentState' | 'componentFunctions'
>;

const CardDashboard = (props: CardDashboardProps) => {
  const { componentState, componentFunctions } = props;

  const dashboardComponents = Object.entries(componentFunctions).map(
    ([displayGroupID, componentFunctions]) => {
      const numDataPoints = componentFunctions.statusFunctions.length;

      return (
        <Grid xs={getCardColumns(numDataPoints)} sx={getCardColumnSX(numDataPoints)}>
          <CardContainer key={displayGroupID} styleOverrides={dashCardContainerSx}>
            <CardHeader
              heading={componentFunctions.displayName}
              icon={getDashboardIcon(componentFunctions.displayName)}
              styleOverrides={dashCardHeaderSx}
            />
            <Box sx={gridContainerSx}>
              <Grid container columnSpacing={0} rowSpacing={0} gridAutoRows="1fr">
                <Grid xs={12}>
                  {componentFunctions.alarmFaultStatusFunction(componentState.dashboard)}
                </Grid>
                {componentFunctions.statusFunctions.map((fn, i) => (
                  <Grid xs={getDataPointColumns(numDataPoints)}>
                    <CardRow key={i} alignItems="flex-end" styleOverrides={dashCardRowSx}>
                      {fn(componentState.dashboard)}
                    </CardRow>
                  </Grid>
                ))}
              </Grid>
            </Box>
          </CardContainer>
        </Grid>
      );
    },
  );

  return (
    <Grid container spacing={2} gridAutoRows="1fr">
      {dashboardComponents}
    </Grid>
  );
};

export default ConfigurablePagesHOC(CardDashboard, 'dashboard');
