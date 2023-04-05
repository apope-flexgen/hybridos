// TODO: fix lint
/* eslint-disable max-lines */
import {
  Box, IconButton, MuiButton, PageLoadingIndicator, Tab, Tabs, Tooltip, Typography,
} from '@flexgen/storybook';
import isEqual from 'lodash.isequal';
import {
  createContext, useCallback, useContext, useEffect, useMemo, useState,
} from 'react';
import { Asset } from 'shared/types/dtos/assets.dto';
import { CANCEL, DUPLICATE, SAVE } from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  BoxSX, ButtonsContainer, Item, ListContainer, MainBoxSX, MuiButtonSX, TabContainer, Toolbar,
} from 'src/pages/UIConfig/TabContent/styles';
import { axiosWebUIInstance } from 'src/services/axios';
import TabContent from './TabContent';
import {
  tabOptions, ADD_NEW_ASSET, DELETE_ASSET, newAsset, ASSETS_URL,
} from './helpers';

interface IAssetsContext {
  assets: Asset[],
  setAssets: React.Dispatch<React.SetStateAction<Asset[] | []>>,
  selectedAsset: Asset | null,
  setSelectedAsset: React.Dispatch<React.SetStateAction<Asset | null>>,
  handleAddAsset: () => void,
  selectedAssetIndex: number | null,
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>
}

const AssetsContext = createContext<IAssetsContext>({
  assets: [],
  setAssets: () => [],
  selectedAsset: null,
  setSelectedAsset: () => null,
  handleAddAsset: () => { },
  selectedAssetIndex: null,
  setIsLoading: () => false,
});

export function useAssetsContext() {
  return useContext(AssetsContext);
}

// TODO: fix this rule
// eslint-disable-next-line max-statements
const Assets = () => {
  const [selectedTab, setSelectedTab] = useState<null | string>(null);
  const [assets, setAssets] = useState<Asset[]>([]);
  const [selectedAssetIndex, setSelectedAssetIndex] = useState<number | null>(null);
  const [selectedAsset, setSelectedAsset] = useState<null | Asset>(null);
  const [isLoading, setIsLoading] = useState(false);

  const handleAddAsset = useCallback(() => {
    setAssets((prevAssets) => [
      newAsset,
      ...prevAssets,
    ]);
    setSelectedTab('info');
    setSelectedAsset(newAsset);
    setSelectedAssetIndex(0);
  }, []);

  const contextValue = useMemo(() => ({
    assets,
    setAssets,
    selectedAsset,
    setSelectedAsset,
    handleAddAsset,
    selectedAssetIndex,
    setIsLoading,
  }), [assets,
    setAssets,
    selectedAsset,
    setSelectedAsset,
    handleAddAsset,
    selectedAssetIndex,
    setIsLoading]);

  const fetchData = async () => {
    try {
      setIsLoading(true);
      const res = await axiosWebUIInstance.get(ASSETS_URL);
      const assetsData: Asset[] = res.data.data;
      setAssets(assetsData);
      if (assetsData.length) {
        setSelectedTab('info');
        setSelectedAsset(assetsData[0]);
        setSelectedAssetIndex(0);
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleAssetClick = (asset: Asset, index: number) => {
    setSelectedAsset(asset);
    setSelectedAssetIndex(index);
  };

  const handleSaveClick = async () => {
    try {
      setIsLoading(true);
      const data = [...assets];
      data.splice(
        selectedAssetIndex !== null ? selectedAssetIndex : -1,
        1,
        selectedAsset || newAsset,
      );
      const res = await axiosWebUIInstance.post(
        ASSETS_URL,
        { data },
      );
      const updatedAssets = res.data.data;
      setAssets(updatedAssets);
    } finally {
      setIsLoading(false);
    }
  };

  const handleDeleteClick = async () => {
    try {
      setIsLoading(true);
      const data = [...assets].filter((_, index) => index !== selectedAssetIndex);
      const res = await axiosWebUIInstance.post(
        ASSETS_URL,
        { data },
      );
      const updatedAssets = res.data.data;
      setAssets(updatedAssets);
      if (updatedAssets.length) {
        setSelectedAsset(updatedAssets[0]);
        setSelectedAssetIndex(0);
      } else {
        setSelectedTab(null);
        setSelectedAsset(null);
        setSelectedAssetIndex(null);
      }
    } finally {
      setIsLoading(false);
    }
  };

  const handleCancelClick = () => {
    setSelectedAsset(assets[selectedAssetIndex || 0]);
  };

  const handleCopyClick = () => {
    setAssets((prevAssets) => [assets[selectedAssetIndex || 0], ...prevAssets]);
    setSelectedAsset(assets[selectedAssetIndex || 0]);
    setSelectedAssetIndex(0);
    setSelectedTab('info');
  };

  const disabledCancel = useMemo(
    () => isEqual(assets[selectedAssetIndex || 0], selectedAsset),
    [assets, selectedAsset, selectedAssetIndex],
  );

  const disabledSave = useMemo(() => selectedAsset?.info.assetKey?.trim() === '' || selectedAsset?.info.name?.trim() === '', [selectedAsset]);

  useEffect(() => {
    fetchData();
  }, []);

  return (
    <AssetsContext.Provider value={contextValue}>
      <TabContainer>
        <Box sx={MainBoxSX}>
          <PageLoadingIndicator isLoading={isLoading} type="primary" />
          <Typography text="Assets" variant="bodyLBold" />
          <ListContainer>
            {assets.map((asset, index) => (
              // TODO: fix lint
              // eslint-disable-next-line react/no-array-index-key
              <Item key={index}>
                <MuiButton
                  color="inherit"
                  label={asset.info.name || 'New Item'}
                  onClick={() => handleAssetClick(asset, index)}
                  sx={MuiButtonSX}
                  variant={index === selectedAssetIndex ? 'contained' : 'outlined'}
                />
                {index === selectedAssetIndex && (
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
              label={ADD_NEW_ASSET}
              onClick={handleAddAsset}
              size="medium"
              startIcon="Add"
              variant="outlined"
            />
          </ListContainer>
        </Box>
        <Box sx={BoxSX}>
          {selectedAsset && (
            <Toolbar>
              <Typography text={selectedAsset.info?.name || ''} variant="bodyMBold" />
              <ButtonsContainer>
                <MuiButton
                  color="inherit"
                  disabled={disabledCancel}
                  label={CANCEL}
                  onClick={handleCancelClick}
                  variant="text"
                />
                <MuiButton
                  color="error"
                  label={DELETE_ASSET}
                  onClick={handleDeleteClick}
                  variant="outlined"
                />
                <MuiButton
                  disabled={disabledSave}
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
                disabled={!assets.length}
                key={value}
                label={label}
                value={value}
              />
            ))}
          </Tabs>
          <TabContent selectedTab={selectedTab} />
        </Box>
      </TabContainer>
    </AssetsContext.Provider>
  );
};

export default Assets;
