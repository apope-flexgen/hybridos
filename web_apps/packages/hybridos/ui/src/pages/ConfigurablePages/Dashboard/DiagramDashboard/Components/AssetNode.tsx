import {
  Box, Divider, Icon, ThemeType, Typography,
} from '@flexgen/storybook';
import React, { useMemo } from 'react';
import { Handle, NodeProps, Position } from 'reactflow';
import { assetIconMapping } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { generateStatusComponent } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.helpers';
import {
  edgeSx,
  statusesBoxSx,
  iconTextBoxSx,
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

  const {
    assetType, label, hasParent, statuses, staticStatusData,
  } = data;

  const statusComponents = useMemo(() => {
    if (statuses && staticStatusData) {
      return Object.entries(staticStatusData).map(([key, status]) => generateStatusComponent(key, status, statuses));
    }
    return [];
  }, [statuses, staticStatusData]);

  const assetIcon = (assetIconMapping[assetType] as ValidAssetIcon) || 'RemoveCircleOutline';

  return (
    <>
      {hasParent && <Handle type="target" position={Position.Top} style={handleSx} />}
      <Box sx={outerBoxSx}>
        <Box sx={iconTextBoxSx(statusComponents.length)}>
          <Icon src={assetIcon} color="primary" size="large" />
          <Typography text={label} variant="bodyL" sx={{ fontWeight: '600' }} />
        </Box>
        {statusComponents.length > 0 && (
          <>
            <Divider variant="fullWidth" orientation="horizontal" />
            <Box sx={statusesBoxSx}>{statusComponents}</Box>
          </>
        )}
      </Box>
      <Handle type="source" position={Position.Bottom} style={{ visibility: 'hidden' }} />
    </>
  );
};

export default AssetNode;
