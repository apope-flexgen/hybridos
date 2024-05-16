/* eslint-disable */
// TODO: fix lint
import { Tab, Tabs, ThemeType } from '@flexgen/storybook';
import { Box } from '@mui/material';
import {
  ConfigurablePageStateStructure,
  DisplayGroupFunctions,
} from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import { getControlOuterBoxSx } from './assetsPage.styles';
import { AssetTab, AssetTabs } from 'src/pages/ConfigurablePages/AssetsPage/assetsPage.types';
import { useContext, useState } from 'react';
import IndiviudalAssetControl from './IndividualAssetControls';
import BatchAssetControl from './BatchAssetControls';
import {
  BatchSelectContext,
  BatchSelectContextType,
} from 'src/pages/ConfigurablePages/contexts/BatchSelectContext';

type AssetControlProps = {
  componentFunctions?: DisplayGroupFunctions;
  assetState: ConfigurablePageStateStructure;
  batchControlsState?: boolean;
  assetKey?: string;
  isSummaryTab: boolean;
};

const AssetControl = ({
  componentFunctions,
  assetState,
  batchControlsState,
  assetKey,
  isSummaryTab,
}: AssetControlProps) => {
  const theme = useTheme() as ThemeType;
  const [selectedTab, setSelectedTab] = useState<AssetTab>('asset');

  const { selectedAssets } = useContext(BatchSelectContext) as BatchSelectContextType;

  const handleTabChange = (newValue: any) => {
    setSelectedTab(newValue);
  };

  const controlChildrenMapped =
    componentFunctions !== undefined
      ? componentFunctions.controlFunctions.map((child) => <>{child(assetState)}</>)
      : [];

  const batchControlChildrenMapped =
    componentFunctions !== undefined && componentFunctions.batchControlFunctions !== undefined
      ? componentFunctions.batchControlFunctions.map((child) => (
          <>{child(assetState, selectedAssets)}</>
        ))
      : [];

  return (
    <>
      {batchControlsState && !isSummaryTab && (
        <Box sx={{ paddingTop: '8px', backgroundColor: theme.fgd.background.paper }}>
          <Tabs value={selectedTab} onChange={(_, newValue) => handleTabChange(newValue)}>
            <Tab label='Asset' value={AssetTabs.asset} />
            <Tab label='Batch' value={AssetTabs.batch} />
          </Tabs>
        </Box>
      )}
      <Box sx={getControlOuterBoxSx(theme)}>
        {!batchControlsState || selectedTab === AssetTabs.asset || isSummaryTab ? (
          <IndiviudalAssetControl
            controlChildrenMapped={controlChildrenMapped}
            componentFunctions={componentFunctions}
          />
        ) : (
          <BatchAssetControl
            assetKey={assetKey}
            uris={assetState ? Object.keys(assetState) : []}
            batchControlChildrenMapped={batchControlChildrenMapped}
          />
        )}
      </Box>
    </>
  );
};

export default AssetControl;
