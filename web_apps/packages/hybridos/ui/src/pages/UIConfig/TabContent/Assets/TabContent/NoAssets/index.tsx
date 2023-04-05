import {
  Box, CardRow, Label, MuiButton, Typography,
} from '@flexgen/storybook';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import {
  CREATE_NEW_ASSET, NO_ASSETS_TO_DISPLAY, SETTING, VALUE,
} from 'src/pages/UIConfig/TabContent/Assets/helpers';
import BoxSX from './styles';

const NoAssets = () => {
  const { handleAddAsset } = useAssetsContext();

  return (
    <>
      <CardRow>
        <Label className="setting" color="primary" size="medium" value={SETTING} />
        <Label color="primary" size="medium" value={VALUE} />
      </CardRow>
      <Box sx={BoxSX}>
        <Typography color="secondary" text={NO_ASSETS_TO_DISPLAY} variant="helperText" />
        <MuiButton label={CREATE_NEW_ASSET} onClick={handleAddAsset} />
      </Box>
    </>
  );
};

export default NoAssets;
