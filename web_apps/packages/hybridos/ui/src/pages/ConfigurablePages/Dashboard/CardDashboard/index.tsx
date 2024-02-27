// TODO: fix lint
/* eslint-disable react/no-array-index-key, @typescript-eslint/no-shadow */
import { CardContainer, CardHeader, CardRow } from '@flexgen/storybook';

import { Box } from '@mui/material';
import Grid from '@mui/system/Unstable_Grid';
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
          <CardContainer key={displayGroupID} styleOverrides={{ height: '100%', display: 'block' }}>
            <CardHeader
              heading={componentFunctions.displayName}
              icon={getDashboardIcon(componentFunctions.displayName)}
            />
            <Box marginTop="10px" sx={{ alignItems: 'flex-end' }}>
              <Grid container columnSpacing={0} rowSpacing={0} gridAutoRows="1fr">
                {componentFunctions.statusFunctions.map((fn, i) => (
                  <Grid xs={getDataPointColumns(numDataPoints)}>
                    <CardRow
                      key={i}
                      alignItems="flex-end"
                      paddingBottom="0.25rem" // TODO: remove this once styleOverrides are fixed by upcoming PR
                      styleOverrides={{ height: '100%', paddingBottom: '0.25rem' }}
                    >
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
