/* eslint-disable max-lines */
import Dagre from '@dagrejs/dagre';
import { Progress, ThemeType, DataPoint } from '@flexgen/storybook';
import { Edge, Node } from 'reactflow';
import Contactor from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/Contactor';
import {
  ASSET,
  assetTypeMapping,
  layoutDirection,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { edgeSx } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.styles';

import { DiagramItem, StaticStatusObject, Status } from './diagramDashboard.types';

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
      pathOptions: {
        borderRadius: 0,
      },
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
    graph.setNode(node.id, { width: node.width || 225, height: node.height || 100 });
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

/**
 * Generates a component based on the props passed in
 *
 * @param key key from staticStatusData map, used to map between status static data and status value
 * @param status object containing static data about the status component
 * @param stateStatusData an object containing state data about this status component (used for value)
 * @returns A formatted component that correctly represents this status
 */
export const generateStatusComponent = (
  key: string,
  status: { static?: StaticStatusObject },
  statuses: Status,
) => {
  if (!status.static?.type) {
    return (
      <DataPoint
        label={status.static?.label || ''}
        value={statuses[key]?.state?.value || '-'}
        unit={status.static?.unit || ''}
        allowWrap
        variant="horizontal"
      />
    );
  }
  switch (status.static?.type.toLowerCase()) {
    case 'progress':
      return (
        <Progress
          variant="secondary"
          value={statuses[key]?.state?.value || 0}
          showPercentage
          orientation="horizontal"
          height={8}
          label={status.static?.label || ''}
          fullWidth
        />
      );
    // TODO: change icon that contactor uses to actual contactor icons, once in storybook
    case 'contactor':
      return (
        <Contactor
          icon="Generator"
          label={`${status.static?.label || 'Breaker'} ${statuses[key]?.state?.value ?? 'Present'}`}
        />
      );
    default:
      return <></>;
  }
};

/**
 * Generates the nodes and edges for a static tree diagram
 *
 * @param newDataFromSocket static data from socket regarding statuses, labels, etc.
 * @param setNodes setter for nodes to display on diagram
 * @param setEdges setter for edges to display on diagram
 * @param theme  Current theme being used
 */
export const generateStaticElements = (
  newDataFromSocket: any,
  setNodes: React.Dispatch<React.SetStateAction<Node<Node[], string | undefined>[]>>,
  setEdges: React.Dispatch<React.SetStateAction<Edge<Edge[]>[]>>,
  theme: ThemeType,
) => {
  const { nodes: diagramNodes, edges: diagramEdges } = generateDiagramNodes(
    theme,
    newDataFromSocket.tree,
  );

  const layoutElements = getLayoutElements(diagramNodes, diagramEdges);

  setNodes(layoutElements.nodes);
  setEdges(layoutElements.edges);
};
