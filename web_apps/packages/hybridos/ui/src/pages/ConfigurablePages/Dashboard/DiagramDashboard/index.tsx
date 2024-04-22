import { CardContainer, ThemeType } from '@flexgen/storybook';
import { dia, shapes, layout } from '@joint/plus';
import {
  useEffect, useState, useRef,
} from 'react';

import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { SITE_DIAGRAM_URL } from 'src/pages/ConfigurablePages/Dashboard/dashboard.constants';
import { useTheme } from 'styled-components';
import { addItemToGraph } from './diagramDashboard.helpers';
import { canvasSx, cardContainerSx } from './diagramDashboard.styles';
import { DiagramItem } from './diagramDashboard.types';

/**
 * A Site Diagram Dashboard view, the 3rd option from the Dashboard view.
 *
 * Fetches diagram data from DBI, then processes it and surfaces Site Diagram nodes with appropriate
 * links between them.
 *
 * @returns DiagramDashboard Component
 */
const DiagramDashboard = () => {
  const theme = useTheme() as ThemeType;
  const canvasRef = useRef(null);
  const [diagramData, setDiagramData] = useState<DiagramItem>();
  const axiosInstance = useAxiosWebUIInstance();

  // Fetch site diagram data
  useEffect(() => {
    axiosInstance.get(SITE_DIAGRAM_URL).then((res) => {
      setDiagramData(res.data);
    });
  }, [axiosInstance]);

  /**
   * Process diagram data to create diagram nodes
   *
   * 1. Instantiate graph, paper, layout
   * 2. Run layout of tree to determine dimensions
   * 3. Manually position of first graph element
   * 4. Rerun final layout & set paper dimensions
   * 5. Create event listener for paper to resize on window resize
   */
  // process diagram data to create diagram nodes
  useEffect(() => {
    if (!diagramData || !canvasRef.current) {
      return () => {};
    }
    const canvas = canvasRef.current as HTMLElement;
    // recursively add all diagram items to graph
    const graph = new dia.Graph();
    addItemToGraph(diagramData, graph, theme);

    // instantiate paper
    const paper = new dia.Paper({
      model: graph,
      sorting: dia.Paper.sorting.APPROX,
      cellViewNamespace: shapes,
      interactive: false,
    });

    // Add paper to canvas
    canvas.appendChild(paper.el);
    const treeLayout = new layout.TreeLayout({
      graph,
      parentGap: 50,
      siblingGap: 20,
      direction: 'B',
    });

    // initially run layout
    treeLayout.layout();

    // Get the bounding box of all elements in the graph
    const graphBoundingBox = graph.getBBox();
    const graphDims = {
      width: (graphBoundingBox?.width || 0),
      height: (graphBoundingBox?.height || 0),
    };

    // force first graph element location
    const root = graph.getElements()[0];
    root.position(graphDims.width * 0.5 - (root.getBBox().width / 2), 0);
    // rerun layout
    treeLayout.layout();

    const handleResize = () => {
      if (!paper || !canvas) return;
      const ratio = (Math.min(canvas.clientWidth / (graphDims.width), canvas.clientHeight / (graphDims.height)));
      paper.setDimensions(graphDims.width * ratio, graphDims.height * ratio);
      paper.scale(ratio, ratio);
    };

    handleResize();
    window.addEventListener('resize', handleResize);

    return () => {
      paper.remove();
      window.removeEventListener('resize', handleResize);
    };
  }, [diagramData, theme]);

  return (
    <CardContainer
      boxShadow={false}
      variant="rounded"
      direction="column"
      styleOverrides={cardContainerSx}
    >
      <div ref={canvasRef} style={canvasSx} />
    </CardContainer>
  );
};

export default DiagramDashboard;
