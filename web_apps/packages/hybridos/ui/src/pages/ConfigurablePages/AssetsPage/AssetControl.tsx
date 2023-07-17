/* eslint-disable */
// TODO: fix lint
import { ThemeType, Typography } from '@flexgen/storybook';
import { Box } from '@mui/material';
import AllControlsContainer from './AllControls';
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import { getControlOuterBoxSx, getControlInnerBoxSx } from './assetsPage.styles';

type AssetControlProps = {
  componentFunctions?: DisplayGroupFunctions;
  assetState: ConfigurablePageStateStructure;
  allControlsState?: boolean;
  currentUser?: any;
};

const AssetControl = ({
  componentFunctions,
  assetState,
  allControlsState,
  currentUser,
}: AssetControlProps) => {
  const theme = useTheme() as ThemeType;

  const controlChildrenMapped =
    componentFunctions !== undefined
      ? componentFunctions.controlFunctions.map((child) => <>{child(assetState)}</>)
      : [];

  return (
    <Box sx={getControlOuterBoxSx(theme)}>
      <Box sx={{ margin: '5px' }}>
        {allControlsState && (
          <AllControlsContainer currentUser={currentUser} uris={Object.keys(assetState)} />
        )}
        <Typography
          variant='headingS'
          text={
            controlChildrenMapped.length > 0
              ? componentFunctions?.displayName
                ? `${componentFunctions.displayName.toUpperCase()} CONTROLS`
                : 'ASSET CONTROLS'
              : 'NO CONTROLS'
          }
          color='secondary'
        />
      </Box>
      {controlChildrenMapped.length > 0 && (
        <Box sx={getControlInnerBoxSx(theme)}>{controlChildrenMapped}</Box>
      )}
    </Box>
  );
};

export default AssetControl;
