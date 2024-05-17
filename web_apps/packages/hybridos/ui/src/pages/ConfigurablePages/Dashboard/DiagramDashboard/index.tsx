import {
  Box, CardContainer, CardRow, Select, ThemeType, Typography,
} from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import {
  Edge, Node, ReactFlowProvider, useEdgesState, useNodesState,
} from 'reactflow';
import { useAppContext } from 'src/App/App';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import Diagram from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/Diagram';
import {
  FLEET_SITE_DIAGRAMS_URL,
  SITE_DIAGRAM_URL,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import {
  generateDiagramNodes,
  getLayoutElements,
} from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.helpers';
import { useTheme } from 'styled-components';
import {
  boxSx, cardContainerSx, diagramBoxSx, titleSelectSx,
} from './diagramDashboard.styles';
import { DiagramItem, FleetConfigs } from './diagramDashboard.types';
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
  const [siteConfigs, setSiteConfigs] = useState<FleetConfigs>({});
  const [sitesMenu, setSitesMenu] = useState<string[]>([]);
  const [selectedSite, setSelectedSite] = useState<string>('');

  const axiosInstance = useAxiosWebUIInstance();
  const theme = useTheme() as ThemeType;
  const { product } = useAppContext();

  useEffect(() => {
    if (product === FLEET_MANAGER) {
      axiosInstance.get(FLEET_SITE_DIAGRAMS_URL).then((res) => {
        const formattedSiteNames = Object.keys(res.data)
          .sort((a, b) => a.localeCompare(b))
          .map((name: string) => name.replace(/\b\w/g, (char: string) => char.toUpperCase()));
        setSiteConfigs(res.data);
        setSitesMenu(formattedSiteNames);
        setSelectedSite(formattedSiteNames[0]);
      });
    } else {
      axiosInstance.get(SITE_DIAGRAM_URL).then((res) => {
        setDiagramData(res.data.tree.root);
      });
    }
  }, [axiosInstance]);

  useEffect(() => {
    setDiagramData(siteConfigs[selectedSite.toLocaleLowerCase()]?.tree.root);
  }, [selectedSite]);

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
      direction="row"
      styleOverrides={cardContainerSx}
    >
      {product === FLEET_MANAGER && (
        <CardRow styleOverrides={titleSelectSx}>
          <Typography text={selectedSite} variant="bodyLBold" />
          <Select
            label="Site"
            menuItems={sitesMenu}
            onChange={(e) => setSelectedSite(e.target.value)}
            value={selectedSite}
          />
        </CardRow>
      )}
      <Box sx={product === FLEET_MANAGER ? diagramBoxSx(theme) : boxSx}>
        <ReactFlowProvider>
          <Diagram site={selectedSite} nodes={nodes} edges={edges} />
        </ReactFlowProvider>
      </Box>
    </CardContainer>
  );
};

export default DiagramDashboard;
