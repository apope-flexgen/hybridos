// TODO: fix lint
/* eslint-disable react/no-array-index-key, @typescript-eslint/no-shadow */
import {
  CardContainer, CardHeader, CardRow,
} from '@flexgen/storybook';
import Masonry from '@mui/lab/Masonry';
import { Box } from '@mui/material';
import ConfigurablePagesHOC, { ConfigurablePagesProps } from 'src/pages/ConfigurablePages/configurablePages.hoc';

const Dashboard = (props: ConfigurablePagesProps) => {
  const { componentState, componentFunctions } = props;

  const dashboardComponents = Object.entries(componentFunctions).map(
    ([displayGroupID, componentFunctions]) => (
      <CardContainer key={displayGroupID}>
        <CardHeader>{componentFunctions.displayName}</CardHeader>
        <Box marginTop="10px">
          {componentFunctions.statusFunctions.map((fn, i) => (
            <CardRow key={i}>{fn(componentState)}</CardRow>
          ))}
        </Box>
      </CardContainer>
    ),
  );

  return (
    <Masonry columns={3} spacing={2}>
      {dashboardComponents}
    </Masonry>
  );
};

export default ConfigurablePagesHOC(Dashboard, 'dashboard');
