/* eslint-disable no-nested-ternary */
import {
  Box, Select, ThemeType, Typography,
} from '@flexgen/storybook';
import { useEffect, useContext } from 'react';
import { BatchSelectContext, BatchSelectContextType } from 'src/pages/ConfigurablePages/contexts/BatchSelectContext';
import { useTheme } from 'styled-components';
import { batchActionsOuterBoxSx, batchActionsTextBoxSx, getControlInnerBoxSx } from './assetsPage.styles';

interface BatchAssetControlProps {
  batchControlChildrenMapped: JSX.Element[]
  uris: string[],
  assetKey?: string,
}

const BatchAssetControl = ({
  batchControlChildrenMapped,
  uris,
  assetKey,
}: BatchAssetControlProps) => {
  const theme = useTheme() as ThemeType;
  const {
    selectedAssets,
    setSelectedAssets,
  } = useContext(BatchSelectContext) as BatchSelectContextType;

  useEffect(() => {
    setSelectedAssets([]);
  }, [assetKey]);

  const handleSelectControlRecipient = (e: any) => {
    const { value } = e.target;
    if (value.includes('Select All')) {
      setSelectedAssets(uris);
    } else {
      setSelectedAssets(value);
    }
  };

  const handleRemoveControlRecipient = (asset: string) => {
    setSelectedAssets((prevSelected) => {
      const newSelectedAssets = prevSelected.filter((item) => item !== asset);
      return newSelectedAssets;
    });
  };

  return (
    <Box sx={batchActionsOuterBoxSx}>
      <Box sx={batchActionsTextBoxSx}>
        <Typography
          variant="bodyLBold"
          text="Batch Controls"
        />
        <Typography
          variant="bodyS"
          text="The controls below will be completed on all selected assets"
          color="secondary"
        />
      </Box>
      <Select
        multiSelect
        value={selectedAssets}
        menuItems={['Select All', ...uris]}
        fullWidth
        onChange={handleSelectControlRecipient}
        label="Control Recipients"
        onDelete={handleRemoveControlRecipient}
      />
      {
    batchControlChildrenMapped?.length > 0 && (
    <Box sx={getControlInnerBoxSx(theme)}>{batchControlChildrenMapped}</Box>
    )
    }
    </Box>
  );
};

export default BatchAssetControl;
