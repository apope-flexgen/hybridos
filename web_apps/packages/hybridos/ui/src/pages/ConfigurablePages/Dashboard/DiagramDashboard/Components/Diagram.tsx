import { useEffect, useState } from 'react';
import ReactFlow, { ReactFlowInstance, useNodesInitialized } from 'reactflow';
import AssetNode from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/AssetNode';
import {
  minZoom,
  resizeDiagramTimeout,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { DiagramProps } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.types';

const nodeTypes = {
  assetNode: AssetNode,
};

const Diagram: React.FC<DiagramProps> = ({
  nodes, edges, site, setIsLoading,
}: DiagramProps) => {
  const [diagramInstance, setDiagramInstance] = useState<ReactFlowInstance>();
  const nodesInitialized = useNodesInitialized();

  useEffect(() => {
    if (nodesInitialized) {
      setTimeout(() => {
        diagramInstance?.fitView();
        setIsLoading(false);
      }, resizeDiagramTimeout);
    }
  }, [site, nodesInitialized]);

  return (
    <ReactFlow
      id="diagram-component"
      nodes={nodes}
      edges={edges}
      nodeTypes={nodeTypes}
      fitView
      minZoom={minZoom}
      onInit={(instance) => setDiagramInstance(instance)}
      proOptions={{ hideAttribution: true }}
    />
  );
};

export default Diagram;
