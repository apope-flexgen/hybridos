import {
  Box,
  CardContainer,
  CardRow,
  PageLoadingIndicator,
  Select,
  ThemeType,
  Typography,
} from '@flexgen/storybook';

import { useCallback, useEffect, useState } from 'react';
import { ReactFlowProvider, useEdgesState, useNodesState } from 'reactflow';
import { useAppContext } from 'src/App/App';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

import Diagram from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/Components/Diagram';
import { FLEET_SITE_IDS_URL } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.constants';
import { generateStaticElements } from 'src/pages/ConfigurablePages/Dashboard/DiagramDashboard/diagramDashboard.helpers';
import QueryService from 'src/services/QueryService';

import { useTheme } from 'styled-components';
import {
  boxSx, cardContainerSx, diagramBoxSx, titleSelectSx,
} from './diagramDashboard.styles';

import 'reactflow/dist/style.css';

/**
 * A Site Diagram Dashboard view, the 3rd option from the Dashboard view.
 * Fetches diagram data from DBI, then processes it and surfaces Site Diagram nodes with appropriate
 * links between them.
 *
 * @returns DiagramDashboard Component
 */
const DiagramDashboard = () => {
  const [nodes, setNodes] = useNodesState([]);
  const [edges, setEdges] = useEdgesState([]);
  const [sitesMenu, setSitesMenu] = useState<string[]>([]);
  const [selectedSite, setSelectedSite] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);

  const axiosInstance = useAxiosWebUIInstance();
  const theme = useTheme() as ThemeType;
  const { product } = useAppContext();

  useEffect(() => {
    if (product === FLEET_MANAGER) {
      axiosInstance.get(FLEET_SITE_IDS_URL).then((res) => {
        const formattedSiteNames = res.data
          .sort((a, b) => a.localeCompare(b))
          .map((name: string) => name.replace(/\b\w/g, (char: string) => char.toUpperCase()));
        setSitesMenu(formattedSiteNames);
        setSelectedSite(formattedSiteNames[0]);
      });
    }
  }, [axiosInstance, product]);

  const handleDataFromSocket = useCallback((newDataFromSocket: any) => {
    if (newDataFromSocket.hasStatic) {
      generateStaticElements(newDataFromSocket, setNodes, setEdges, theme);
    }

    Object.values(newDataFromSocket.displayGroups).forEach((dataObject: any) => {
      setNodes((prevNodes) => prevNodes.map((node) => {
        if (dataObject.treeId === node.id) {
          const newData = {
            ...node.data,
            [newDataFromSocket.hasStatic ? 'staticStatusData' : 'statuses']: {
              ...node.data?.statuses,
              ...dataObject.status,
            },
          };
          return { ...node, data: newData };
        }
        return node;
      }));
    });
  }, []);

  useEffect(() => {
    if (product !== FLEET_MANAGER) {
      setIsLoading(true);

      QueryService.getSiteDiagram(null, handleDataFromSocket);
    }
  }, []);

  useEffect(() => {
    if (product === FLEET_MANAGER && selectedSite) {
      setIsLoading(true);
      QueryService.getSiteDiagram(selectedSite, handleDataFromSocket);
    }
  }, [selectedSite]);

  useEffect(
    () => () => {
      QueryService.cleanupSocket();
    },
    [],
  );

  return (
    <CardContainer
      id="diagram-container"
      boxShadow={false}
      variant="rounded"
      direction="row"
      styleOverrides={cardContainerSx}
    >
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
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
      <Box sx={product === FLEET_MANAGER ? diagramBoxSx(theme, isLoading) : boxSx(isLoading)}>
        <ReactFlowProvider>
          <Diagram site={selectedSite} nodes={nodes} edges={edges} setIsLoading={setIsLoading} />
        </ReactFlowProvider>
      </Box>
    </CardContainer>
  );
};

export default DiagramDashboard;
