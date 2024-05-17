import { useEffect, useState } from 'react';
import ReactFlow, { ReactFlowInstance } from 'reactflow';
import AssetNode from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/AssetNode';
import {
  minZoom,
  resizeDiagramTimeout,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { DiagramProps } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.types';

const Diagram: React.FC<DiagramProps> = ({ site, nodes, edges }: DiagramProps) => {
  const [diagramInstance, setDiagramInstance] = useState<ReactFlowInstance>();

  const nodeTypes = {
    assetNode: AssetNode,
  };

  useEffect(() => {
    setTimeout(() => diagramInstance?.fitView(), resizeDiagramTimeout);
  }, [site]);

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
