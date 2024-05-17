import Dagre from '@dagrejs/dagre';
import { ThemeType } from '@flexgen/storybook';
import { Edge, Node } from 'reactflow';
import {
  ASSET,
  assetTypeMapping,
  layoutDirection,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { edgeSx } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.styles';
import { DiagramItem } from './diagramDashboard.types';

/**
 * Format a label for node in site diagram
 *
 * @param _label Label string to format, can be undefined
 * @returns Properly formatted label string
 */
export const formatNodelabel = (_label?: string): string => {
  if (!_label) {
    return 'No label found';
  }
  let label = _label;
  label = label.replaceAll('psm_', '');
  label = label.replaceAll('_', ' ');
  label = label.replaceAll(/(?<!\w)ess/g, 'ESS');
  label = label.replaceAll(/(?<!\w)pcs/g, 'PCS');
  label = label.replaceAll(/(?<!\w)bms/g, 'BMS');
  label = label.replaceAll(/(?<!\w)poi/g, 'POI');
  label = label.replaceAll(/(?<!\w)pv/g, 'PV');
  label = label.replace(/\b\w/g, (char: string) => char.toUpperCase());
  return label;
};

/**
 * Determines asset type based on the string read from config file
 *
 * @param assetString String read from config file
 * @returns Asset type
 */
export const determineAssetType = (assetString?: string): ASSET => {
  let assetType = ASSET.Default;

  if (!assetString) return assetType;

  Object.entries(assetTypeMapping).forEach(([type, values]) => {
    if (values.includes(assetString)) {
      assetType = type as ASSET;
    }
  });

  return assetType;
};

/**
 * Generates diagram-compatible arrays of node and edge objects based on the raw data passed in;
 * Recursively called on each child of each node to append the resulting node/edge objects to
 * their respective arrays
 *
 * @param theme Current theme being used
 * @param item The DiagramItem to parse into a diagram-compatible object
 * @param parentId The item's parent's id, used for generating edge
 * @param xOffset Used for intial x positioning of node
 * @param yOffset Used for initial y positioning of node
 * @returns Array of generated nodes and array of generated edges
 */
export const generateDiagramNodes = (
  theme: ThemeType,
  item?: DiagramItem,
  parentId = '',
  xOffset = 0,
  yOffset = 0,
): { nodes: Node[]; edges: Edge[] } => {
  const nodes: Node[] = [];
  const edges: Edge[] = [];

  if (!item) return { nodes, edges };

  const hasParent = parentId !== '';

  nodes.push({
    type: 'assetNode',
    id: item.id,
    position: { x: xOffset, y: yOffset },
    data: {
      assetType: determineAssetType(item.asset_type),
      label: formatNodelabel(item.id),
      hasParent,
    },
  });

  if (hasParent) {
    edges.push({
      type: 'smoothstep',
      id: `${parentId}-${item.id}`,
      source: parentId,
      target: item.id,
      style: { ...edgeSx(theme) },
    });
  }

  const childXOffset = xOffset + 200;
  let childYOffset = yOffset;

  if (item.children && item.children.length > 0) {
    item.children.forEach((child) => {
      const { nodes: childNodes, edges: childEdges } = generateDiagramNodes(
        theme,
        child,
        item.id,
        childXOffset,
        childYOffset,
      );
      nodes.push(...childNodes);
      edges.push(...childEdges);
      childYOffset += 100;
    });
  }

  return { nodes, edges };
};

/**
 * Applies layouting to nodes and edges in diagram
 *
 * @param nodes Array of all nodes in diagram
 * @param edges Array of all edges in diagram
 * @returns Arrays of nodes and edges in diagram with their positions updated after applying layout
 */
export const getLayoutElements = (
  nodes: Node[],
  edges: Edge[],
): { nodes: Node[]; edges: Edge[] } => {
  const graph = new Dagre.graphlib.Graph().setDefaultEdgeLabel(() => ({}));

  // set layout options, such as direction
  graph.setGraph({ rankdir: layoutDirection });

  // add nodes and edges to layouting graph
  nodes.forEach((node) => {
    // TODO: use dynamic node width and height in layouting; will be completed as part of DC-505
    graph.setNode(node.id, { width: 200, height: 50 });
  });
  edges.forEach((edge) => graph.setEdge(edge.source, edge.target));

  Dagre.layout(graph);

  return {
    nodes: nodes.map((node) => {
      const { x, y } = graph.node(node.id);
      return { ...node, position: { x, y } };
    }),
    edges,
  };
};
