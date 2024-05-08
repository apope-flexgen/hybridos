/* eslint-disable no-nested-ternary */
import { Box, ThemeType, Typography } from '@flexgen/storybook';
import { DisplayGroupFunctions } from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import { assetControlOuterBoxSx, getControlInnerBoxSx } from './assetsPage.styles';

interface IndiviudalAssetControlProps {
  controlChildrenMapped: JSX.Element[];
  componentFunctions?: DisplayGroupFunctions;
}

const IndiviudalAssetControl = ({
  componentFunctions,
  controlChildrenMapped,
}: IndiviudalAssetControlProps) => {
  const theme = useTheme() as ThemeType;

  return (
    <Box sx={assetControlOuterBoxSx}>
      <Typography
        variant="bodyLBold"
        text={
          controlChildrenMapped.length > 0
            ? componentFunctions?.displayName
              ? `${componentFunctions.displayName} Controls`
              : 'Asset Controls'
            : 'No Controls Configured'
        }
      />
      {controlChildrenMapped.length > 0 && (
        <Box sx={getControlInnerBoxSx(theme)}>{controlChildrenMapped}</Box>
      )}
    </Box>
  );
};

export default IndiviudalAssetControl;
