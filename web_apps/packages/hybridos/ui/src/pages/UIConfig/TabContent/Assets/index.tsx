/* eslint-disable */
// TODO: fix lint
import {
  Box,
  IconButton,
  MuiButton,
  PageLoadingIndicator,
  Tab,
  Tabs,
  ThemeType,
  Tooltip,
  Typography,
} from '@flexgen/storybook';
import isEqual from 'lodash.isequal';
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';
import { Asset } from 'shared/types/dtos/assets.dto';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { CANCEL, DUPLICATE, SAVE } from 'src/pages/UIConfig/TabContent/helpers/constants';
import {
  BoxSX,
  ButtonsContainer,
  Item,
  ListContainer,
  MainBoxSX,
  MuiButtonSX,
  TabContainer,
  Toolbar,
} from 'src/pages/UIConfig/TabContent/styles';
import TabContent from './TabContent';
import { tabOptions, ADD_NEW_ASSET, DELETE_ASSET, newAsset, ASSETS_URL } from './helpers';
import { Layout } from 'shared/types/dtos/layouts.dto';
import { NotifContext, NotifContextType } from 'src/contexts/NotifContext';
import { useAppContext } from 'src/App/App';
import { useTheme } from 'styled-components';

interface IAssetsContext {
  assets: Asset[];
  setAssets: React.Dispatch<React.SetStateAction<Asset[] | []>>;
  selectedAsset: Asset | null;
  setSelectedAsset: React.Dispatch<React.SetStateAction<Asset | null>>;
  handleAddAsset: () => void;
  selectedAssetIndex: number | null;
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>;
}

const AssetsContext = createContext<IAssetsContext>({
  assets: [],
  setAssets: () => [],
  selectedAsset: null,
  setSelectedAsset: () => null,
  handleAddAsset: () => {},
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
  const notifCtx = useContext<NotifContextType | null>(NotifContext);
  const theme = useTheme() as ThemeType;
  const { layouts } = useAppContext();
  const axiosInstance = useAxiosWebUIInstance();

  const handleAddAsset = useCallback(
    async (asset = newAsset) => {
      try {
        setIsLoading(true);
        const newAssets = [asset, ...assets];
        setAssets(newAssets);
        setSelectedTab('info');
        setSelectedAsset(asset);
        setSelectedAssetIndex(0);
      } finally {
        setIsLoading(false);
      }
    },
    [assets, axiosInstance],
  );

  const contextValue = useMemo(
    () => ({
      assets,
      setAssets,
      selectedAsset,
      setSelectedAsset,
      handleAddAsset,
      selectedAssetIndex,
      setIsLoading,
    }),
    [
      assets,
      setAssets,
      selectedAsset,
      setSelectedAsset,
      handleAddAsset,
      selectedAssetIndex,
      setIsLoading,
    ],
  );

  const parseData = useCallback(
    (assets: Asset[], layouts: Layout[]) =>
      assets.map((asset) => {
        const matchLayout = layouts.find((layout) => layout.info.key === asset.info.assetKey);
        return {
          ...asset,
          info: {
            ...asset.info,
            name: matchLayout?.info.name || asset.info.name,
          },
        };
      }),
    [layouts],
  );

  const fetchData = useCallback(async () => {
    try {
      setIsLoading(true);
      const res = await axiosInstance.get(ASSETS_URL);
      const assetsData: Asset[] = parseData(res.data.data || [], layouts);
      setAssets(assetsData);
      if (assetsData.length) {
        setSelectedTab('info');
        setSelectedAsset(assetsData[0]);
        setSelectedAssetIndex(0);
      }
    } finally {
      setIsLoading(false);
    }
  }, [axiosInstance, parseData]);

  const handleAssetClick = (asset: Asset, index: number) => {
    setSelectedAsset(asset);
    setSelectedAssetIndex(index);
  };

  const handleSaveClick = async () => {
    try {
      setIsLoading(true);
      let data = [...assets];
      data.splice(
        selectedAssetIndex !== null ? selectedAssetIndex : -1,
        1,
        selectedAsset || newAsset,
      );
      const res = await axiosInstance.post(ASSETS_URL, { data });
      const updatedAssets = parseData(res.data.data, layouts);
      setAssets(updatedAssets);
      notifCtx?.notif('success', 'Data successfully saved');
    } finally {
      setIsLoading(false);
    }
  };

  const handleDeleteClick = async () => {
    try {
      setIsLoading(true);
      const data = [...assets].filter((_, index) => index !== selectedAssetIndex);
      const res = await axiosInstance.post(ASSETS_URL, { data });
      const updatedAssets = parseData(res.data.data, layouts);
      setAssets(updatedAssets);
      if (updatedAssets.length) {
        setSelectedAsset(updatedAssets[0]);
        setSelectedAssetIndex(0);
      } else {
        setSelectedTab(null);
        setSelectedAsset(null);
        setSelectedAssetIndex(null);
      }
      notifCtx?.notif('success', 'Data successfully saved');
    } finally {
      setIsLoading(false);
    }
  };

  const handleCancelClick = () => {
    setSelectedAsset(assets[selectedAssetIndex || 0]);
  };

  const handleCopyClick = () => {
    handleAddAsset(assets[selectedAssetIndex || 0]);
  };

  const disabledCancel = useMemo(
    () => isEqual(assets[selectedAssetIndex || 0], selectedAsset),
    [assets, selectedAsset, selectedAssetIndex],
  );

  const disabledSave = useMemo(
    () =>
      !!assets.length &&
      (!(selectedAsset?.info.name || '').trim() || !(selectedAsset?.info.sourceURI || '').trim()),
    [assets, selectedAsset],
  );

  const disabledAdd = useMemo(
    () =>
      !!assets.length &&
      assets.some(
        (asset) => !(asset?.info.name || '').trim() || !(asset?.info.sourceURI || '').trim(),
      ),
    [assets],
  );

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  return (
    <AssetsContext.Provider value={contextValue}>
      <TabContainer>
        <Box sx={MainBoxSX(theme)}>
          <PageLoadingIndicator isLoading={isLoading} type='primary' />
          <Typography text='Assets' variant='bodyLBold' />
          <ListContainer>
            {assets.map((asset, index) => (
              // TODO: fix lint
              // eslint-disable-next-line react/no-array-index-key
              <Item key={index}>
                <MuiButton
                  label={asset.info.name}
                  onClick={() => handleAssetClick(asset, index)}
                  sx={MuiButtonSX}
                  variant={index === selectedAssetIndex ? 'contained' : 'outlined'}
                  disabled={disabledAdd}
                />
                {index === selectedAssetIndex && (
                  <Tooltip title={DUPLICATE}>
                    <IconButton
                      color='action'
                      icon='ContentCopy'
                      onClick={handleCopyClick}
                      disabled={disabledAdd}
                    />
                  </Tooltip>
                )}
              </Item>
            ))}
            <MuiButton
              fullWidth
              label={ADD_NEW_ASSET}
              onClick={() => handleAddAsset()}
              size='medium'
              startIcon='Add'
              variant='outlined'
              disabled={disabledAdd}
            />
          </ListContainer>
        </Box>
        <Box sx={BoxSX(theme)}>
          {selectedAsset && (
            <Toolbar>
              <Typography text={selectedAsset.info?.name || ''} variant='bodyMBold' />
              <ButtonsContainer>
                <MuiButton
                  color='inherit'
                  disabled={disabledCancel}
                  label={CANCEL}
                  onClick={handleCancelClick}
                  variant='text'
                />
                <MuiButton
                  color='error'
                  label={DELETE_ASSET}
                  onClick={handleDeleteClick}
                  variant='outlined'
                />
                <MuiButton
                  disabled={disabledSave}
                  label={SAVE}
                  onClick={handleSaveClick}
                  variant='contained'
                />
              </ButtonsContainer>
            </Toolbar>
          )}
          <Tabs onChange={(_, tab) => setSelectedTab(tab as string)} value={selectedTab}>
            {tabOptions.map(({ label, value }) => (
              <Tab disabled={!assets.length} key={value} label={label} value={value} />
            ))}
          </Tabs>
          <TabContent selectedTab={selectedTab} />
        </Box>
      </TabContainer>
    </AssetsContext.Provider>
  );
};

export default Assets;
