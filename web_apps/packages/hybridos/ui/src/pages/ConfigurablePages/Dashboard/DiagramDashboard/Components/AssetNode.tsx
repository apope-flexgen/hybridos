import {
  Box, Icon, ThemeType, Typography,
} from '@flexgen/storybook';
import React from 'react';
import { Handle, NodeProps, Position } from 'reactflow';
import { assetIconMapping } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import {
  edgeSx,
  nodeBoxSx,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.styles';
import {
  AssetNodeProps,
  ValidAssetIcon,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.types';
import { useTheme } from 'styled-components';

const AssetNode: React.FC<NodeProps<AssetNodeProps>> = ({ data }: NodeProps<AssetNodeProps>) => {
  const theme = useTheme() as ThemeType;
  const outerBoxSx = nodeBoxSx(theme);
  const handleSx = edgeSx(theme);

  const { assetType, label, hasParent } = data;

  const assetIcon = (assetIconMapping[assetType] as ValidAssetIcon) || 'RemoveCircleOutline';

  return (
    <>
      {hasParent && <Handle type="target" position={Position.Top} style={handleSx} />}
      <Box sx={outerBoxSx}>
        <Icon src={assetIcon} color="primary" />
        <Typography text={label} variant="bodyM" />
      </Box>
      <Handle type="source" position={Position.Bottom} style={{ visibility: 'hidden' }} />
    </>
  );
};

export default AssetNode;
