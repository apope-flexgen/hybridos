import { Divider, Typography, EmptyContainer } from '@flexgen/storybook';
import { Box } from '@mui/material';
import Grid from '@mui/material/Grid';
import React, { ReactElement } from 'react';
import {
  AlertState,
  ConfigurableComponentFunction,
  ConfigurablePageStateStructure,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import AlertContainer from './AlertContainer';
import {
  statusOuterBoxSx,
  statusPointsDisplayOuterBoxSx,
  statusPointsDisplaySubheaderBoxSx,
} from './assetsPage.styles';

// TODO: if we can find a way to calculate this automatically
// rather than setting a constant for everyone, that would be
// an upgrade. But this works for now

export interface SingleAssetProps {
  assetName: string;
  statusChildren: ConfigurableComponentFunction[];
  assetState: ConfigurablePageStateStructure;
  alertState: AlertState[string];
}

const Header = ({ headerText }: { headerText: string }) => (
  <Typography variant="headingL" color="primary" text={headerText} />
);

const noStatusesToDisplayMessage = 'No statuses have been configured for this asset';

const StatusPointsDisplay = ({ statusComponents }: { statusComponents: ReactElement[] }) => {
  const numColumns = statusComponents.length < 12 ? 1 : 2;
  return (
    <Box sx={statusPointsDisplayOuterBoxSx}>
      <Box sx={statusPointsDisplaySubheaderBoxSx}>
        <Typography variant="headingS" color="secondary" text="STATUS" />
        <Divider orientation="horizontal" variant="fullWidth" />
      </Box>
      {statusComponents.length > 0 ? (
        <Grid container columns={numColumns} spacing={1} columnSpacing={3} alignItems="flex-end">
          {statusComponents}
        </Grid>
      ) : (
        <Box sx={{ height: '665px' }}>
          <EmptyContainer text={noStatusesToDisplayMessage} />
        </Box>
      )}
    </Box>
  );
};

const AssetStatus: React.FC<SingleAssetProps> = (props: SingleAssetProps): ReactElement => {
  const {
    assetName, statusChildren, assetState, alertState,
  } = props;

  const statusChildrenMapped = statusChildren.map((child) => (
    <Grid item xs={1}>
      {child(assetState)}
    </Grid>
  ));

  return (
    <Box sx={statusOuterBoxSx}>
      <Header headerText={assetName} />
      <AlertContainer alerts={alertState} />
      <StatusPointsDisplay statusComponents={statusChildrenMapped} />
    </Box>
  );
};

export default AssetStatus;
