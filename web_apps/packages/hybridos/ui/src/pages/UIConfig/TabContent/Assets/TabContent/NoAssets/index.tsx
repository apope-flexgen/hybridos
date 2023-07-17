import {
  CardRow, Label, MuiButton, EmptyContainer,
} from '@flexgen/storybook';
import { useAssetsContext } from 'src/pages/UIConfig/TabContent/Assets';
import {
  CREATE_NEW_ASSET,
  NO_ASSETS_TO_DISPLAY,
  SETTING,
  VALUE,
} from 'src/pages/UIConfig/TabContent/Assets/helpers';

const NoAssets = () => {
  const { handleAddAsset } = useAssetsContext();

  return (
    <>
      <CardRow>
        <Label className="setting" color="primary" size="medium" value={SETTING} />
        <Label color="primary" size="medium" value={VALUE} />
      </CardRow>
      <EmptyContainer text={NO_ASSETS_TO_DISPLAY}>
        <MuiButton label={CREATE_NEW_ASSET} onClick={() => handleAddAsset()} />
      </EmptyContainer>
    </>
  );
};

export default NoAssets;
