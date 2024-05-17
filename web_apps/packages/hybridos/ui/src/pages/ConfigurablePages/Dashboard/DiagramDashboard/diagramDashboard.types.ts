import { Edge, Node } from 'reactflow';

// A diagram item and its child relationships, matching the format from psm_tree.json
export type DiagramItem = {
  id: string;
  asset_type: string | undefined;
  children: DiagramItem[] | undefined;
};

// Props passed to Diagram component
export interface DiagramProps {
  nodes: Node[];
  edges: Edge[];
  site: string;
}

// Props passed to the custom AssetNode component
export interface AssetNodeProps {
  assetType: string;
  label: string;
  hasParent: boolean;
}

// Valid icon options to be displayed in the diagram
export type ValidAssetIcon =
  | 'BatteryHorizontal'
  | 'SolarRounded'
  | 'Generator'
  | 'FeederAlt'
  | 'Microgrid'
  | 'RemoveCircleOutline';

export type FleetConfigs = {
  [key: string]: {
    tree: {
      root: DiagramItem;
    };
  };
};
