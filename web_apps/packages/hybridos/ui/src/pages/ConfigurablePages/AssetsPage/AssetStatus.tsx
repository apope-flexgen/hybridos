/* eslint-disable max-lines */
import {
  Divider,
  Typography,
  EmptyContainer,
  Chip,
  CardContainer,
  ThemeType,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import Grid from '@mui/material/Grid';
import React, { ReactElement } from 'react';
import { useTheme } from 'styled-components';
import AlertContainer from './AlertContainer';
import {
  maintenanceActionsBoxSx,
  statusAndMaintenanceActionsBoxSx,
  getStatusOuterBoxSx,
  statusPadding,
  statusPointsDisplaySubheaderBoxSx,
} from './assetsPage.styles';
import { SingleAssetProps } from './assetsPage.types';

const Header = ({
  headerText,
  maintModeStatus,
}: {
  headerText: string;
  maintModeStatus: boolean;
}) => (
  <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
    <Typography variant="bodyXLBold" color="primary" text={headerText} />
    {maintModeStatus && (
      <Chip
        variant="outlined"
        color="primary"
        size="small"
        borderStyle="squared"
        label="Maintenance Mode"
        icon="Build"
      />
    )}
  </Box>
);

const noStatusesToDisplayMessage = 'No statuses have been configured for this asset';

const StatusPointsDisplay = ({ statusComponents }: { statusComponents: ReactElement[] }) => {
  const numColumns = statusComponents.length < 12 ? 1 : 2;
  return (
    <CardContainer>
      <Box sx={statusPointsDisplaySubheaderBoxSx}>
        <Typography variant="labelM" color="primary" text="Status" />
      </Box>
      <Box sx={{ width: '100%' }}>
        <Divider orientation="horizontal" variant="fullWidth" />
      </Box>
      {statusComponents.length > 0 ? (
        <Grid
          sx={statusPadding}
          container
          columns={numColumns}
          spacing={1}
          columnSpacing={3}
          alignItems="flex-end"
        >
          {statusComponents}
        </Grid>
      ) : (
        <Box sx={{ height: '665px', width: '100%' }}>
          <EmptyContainer text={noStatusesToDisplayMessage} />
        </Box>
      )}
    </CardContainer>
  );
};

const MaintenanceActionsDisplay = ({
  maintenanceActionsComponents,
}: {
  maintenanceActionsComponents: ReactElement[];
}) => {
  // don't display inactive maintenance actions
  const activeMaintenaceActionsComponents = maintenanceActionsComponents.filter(
    (maintenanceActionComponent) => !maintenanceActionComponent.props.inactive,
  );

  return activeMaintenaceActionsComponents.length !== 0 ? (
    <CardContainer>
      <Box sx={statusPointsDisplaySubheaderBoxSx}>
        <Typography variant="labelM" color="primary" text="Maintenance Actions" />
      </Box>
      <Box sx={{ width: '100%' }}>
        <Divider orientation="horizontal" variant="fullWidth" />
      </Box>
      <Box sx={maintenanceActionsBoxSx}>{activeMaintenaceActionsComponents}</Box>
    </CardContainer>
  ) : (
    <></>
  );
};

const AssetStatus: React.FC<SingleAssetProps> = (props: SingleAssetProps): ReactElement => {
  const {
    assetName,
    statusChildren,
    assetState,
    alertState,
    maintModeStatus,
    maintenanceActionsChildren,
    maintenanceActionsState,
  } = props;

  const theme = useTheme() as ThemeType;

  const statusChildrenMapped = statusChildren.map((child) => (
    <Grid item xs={1}>
      {child(assetState)}
    </Grid>
  ));

  const maintenanceActionsMapped = maintenanceActionsChildren
    ? maintenanceActionsChildren.map((child) => child(assetState))
    : [];

  return (
    <Box sx={getStatusOuterBoxSx(theme)}>
      <Box sx={statusAndMaintenanceActionsBoxSx}>
        <Header headerText={assetName} maintModeStatus={maintModeStatus || false} />
        {(alertState.alarmInformation.length > 0 || alertState.faultInformation.length > 0) && (
          <AlertContainer alerts={alertState} />
        )}
        {maintenanceActionsState && (
          <MaintenanceActionsDisplay maintenanceActionsComponents={maintenanceActionsMapped} />
        )}
        <StatusPointsDisplay statusComponents={statusChildrenMapped} />
      </Box>
    </Box>
  );
};

export default AssetStatus;
