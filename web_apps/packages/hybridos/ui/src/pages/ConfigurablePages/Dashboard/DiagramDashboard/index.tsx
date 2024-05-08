import { CardContainer, ThemeType } from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import ReactFlow, {
  Edge, Node, useEdgesState, useNodesState,
} from 'reactflow';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import AssetNode from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/AssetNode';
import {
  generateDiagramNodes,
  getLayoutElements,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.helpers';
import { SITE_DIAGRAM_URL } from 'src/pages/ConfigurablePages/Dashboard/dashboard.constants';
import { useTheme } from 'styled-components';
import { cardContainerSx } from './diagramDashboard.styles';
import { DiagramItem } from './diagramDashboard.types';
import 'reactflow/dist/style.css';

/**
 * A Site Diagram Dashboard view, the 3rd option from the Dashboard view.
 *
 * Fetches diagram data from DBI, then processes it and surfaces Site Diagram nodes with appropriate
 * links between them.
 *
 * @returns DiagramDashboard Component
 */
const DiagramDashboard = () => {
  const [diagramData, setDiagramData] = useState<DiagramItem>();
  const [nodes, setNodes] = useNodesState<Node[]>([]);
  const [edges, setEdges] = useEdgesState<Edge[]>([]);

  const axiosInstance = useAxiosWebUIInstance();
  const theme = useTheme() as ThemeType;

  const nodeTypes = {
    assetNode: AssetNode,
  };

  useEffect(() => {
    axiosInstance.get(SITE_DIAGRAM_URL).then((res) => {
      setDiagramData(res.data);
    });
  }, [axiosInstance]);

  useEffect(() => {
    const { nodes: diagramNodes, edges: diagramEdges } = generateDiagramNodes(theme, diagramData);
    const layoutElements = getLayoutElements(diagramNodes, diagramEdges);
    setNodes(layoutElements.nodes);
    setEdges(layoutElements.edges);
  }, [diagramData]);

  return (
    <CardContainer
      id="diagram-container"
      boxShadow={false}
      variant="rounded"
      direction="column"
      styleOverrides={cardContainerSx}
    >
      <ReactFlow
        id="diagram-component"
        nodes={nodes}
        edges={edges}
        nodeTypes={nodeTypes}
        fitView
        proOptions={{ hideAttribution: true }}
      />
    </CardContainer>
  );
};

export default DiagramDashboard;
